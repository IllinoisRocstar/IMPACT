//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <string.h>
#include <cstdlib>
#include <iostream>
#include <map>

#include "Scheduler.h"
#include "com.h"

/**
   Scheduler
*/

Scheduler::Scheduler() : alphaT(-1), scheduled(0), inited(0) {
  scheduler_name = "Scheduler";
}

void Scheduler::add_action(Action *action) {
  ActionItem *aitem = new ActionItem(action);
  actions.push_back(aitem);
  // call back action
  action->declare(*this);
}

void Scheduler::reads(Action *a, const char *attr, int idx) {
  // FIXME: locate this action, doing linear search now
  ActionList::iterator aitem;
  for (aitem = actions.begin(); aitem != actions.end(); ++aitem)
    if ((*aitem)->action() == a) break;
  if (aitem == actions.end()) {
    printf("ERROR: action '%s' not registered to scheduler. \n", a->name());
    COM_assertion(false);
  }
  (*aitem)->read_attr.push_back(strdup(attr));  // copy the string
  (*aitem)->read_idx.push_back(idx);
  (*aitem)->input.push_back(NULL);
}

void Scheduler::writes(Action *a, const char *attr, int idx) {
  // locate this action
  ActionList::iterator aitem;
  for (aitem = actions.begin(); aitem != actions.end(); ++aitem)
    if ((*aitem)->action() == a) break;
  if (aitem == actions.end()) {
    printf("ERROR: action '%s' not registered to scheduler. \n", a->name());
    COM_assertion(false);
  }
  (*aitem)->write_attr.push_back(strdup(attr));  // copy the string
  (*aitem)->write_idx.push_back(idx);
  (*aitem)->output.push_back(NULL);
}

void Scheduler::schedule() {
  COM_assertion_msg(!scheduled,
                    "ERROR: Scheduler has already been scheduled.\n");

  // schedule all sub-schedulers
  for (unsigned int i = 0; i < actions.size(); ++i)
    actions[i]->action()->schedule();

  // debugging
  printActions();

  buildDDG();

  sanityCheck();

  topological_sort();

  printf("Topological sort:\n");
  print_toposort(stdout);
  printf("\n");

  scheduled = 1;
}

void Scheduler::init_actions(double t) {
  COM_assertion_msg(scheduled, "ERROR: Scheduler has not been scheduled.\n");
  if (inited) return;
  inited = 1;

  // do at sorted order
  for (ActionList::iterator aitem = sort.begin(); aitem != sort.end();
       ++aitem) {
    ActionItem *item = *aitem;
    item->action()->init(t);
  }
}

void Scheduler::run_actions(double t, double dt) {
  COM_assertion_msg(
      scheduled,
      "ERROR: Scheduler has not been scheduled when calling run_actions.\n");
  // do at sorted order
  for (ActionList::iterator aitem = sort.begin(); aitem != sort.end();
       ++aitem) {
    ActionItem *item = *aitem;
    item->action()->run(t, dt, alphaT);
  }
}

void Scheduler::finalize_actions() {
  COM_assertion_msg(scheduled,
                    "ERROR: Scheduler has not been scheduled when calling "
                    "finalize_actions.\n");
  if (sort.empty()) return;
  // do at reversed order
  for (ActionList::iterator aitem = sort.end() - 1; aitem >= sort.begin();
       --aitem) {
    ActionItem *item = *aitem;
    item->action()->finalize();
  }
}

// print in GDL
void Scheduler::print(const char *fname) {
  FILE *f = fopen(fname, "w");

  fprintf(f,
          "graph: { title: \"DependenceTree\" \n\
        display_edge_labels: yes \n\
        layoutalgorithm: tree   \n\
        scaling: maxspect   \n\
        node.color     : green   \n\
        node.textcolor : black   \n\
        node.bordercolor: black \n\
        node.borderwidth: 1    \n\
        edge.color     : blue   \n\
        edge.arrowsize : 7   \n\
        edge.thickness : 2   \n\
        edge.fontname:\"helvO08\"  \n\
        node.label: \"no type\" \n");

  print(f, "scheduler");

  fprintf(f, "} \n");
  fclose(f);
}

char *Scheduler::print(FILE *f, const char *container_name) {
  if (actions.empty()) return NULL;

  std::string sched_name = container_name;
  sched_name = sched_name + "-" + name();
  fprintf(f,
          "graph: { title: \"%s\" label: \"%s\" \n\
        display_edge_labels: yes \n\
        layoutalgorithm: tree   \n\
        scaling: maxspect   \n\
        color :  white           \n\
        node.color     : lightblue   \n\
        node.textcolor : black   \n\
        node.bordercolor: black \n\
        node.borderwidth: 1    \n\
        edge.color     : lightblue   \n\
        edge.arrowsize : 7   \n\
        edge.thickness : 2   \n\
        edge.fontname:\"helvO08\"  \n\
        node.label: \"no type\" \n",
          sched_name.c_str(), sched_name.c_str());

  unsigned int i;
  for (i = 0; i < actions.size(); ++i) actions[i]->print_flag = 0;

  for (i = 0; i < roots.size(); ++i) {
    print_helper(f, roots[i]);
  }

  fprintf(f, "}\n");

  return strdup(sched_name.c_str());
}

void Scheduler::discoverSets(ActionList &pool) {
  std::map<std::string, int> Sets;
  // Add all action names to the Map, each in their own set
  for (int y = 0; y < pool.size(); ++y) {
    Sets[std::string(pool[y]->name())] = y;
  }

  for (unsigned int idx = 1; idx < pool.size(); ++idx) {
    ActionItem *item = pool[idx];
    // search through the actionlist, create links and add to the same set
    for (unsigned int i = 0; i < item->n_read(); ++i) {
      for (int j = idx - 1; j >= 0; --j) {
        ActionItem *aitem = pool[j];
        int inIdx = aitem->hasOutput(item->read_attr[i], item->read_idx[i]);
        if (inIdx != -1) {  // if aitem writes to item
          // if aitem writes to item, add them to the same set
          if (Sets[item->name()] < Sets[aitem->name()]) {
            Sets[aitem->name()] = Sets[item->name()];
          } else {
            Sets[item->name()] = Sets[aitem->name()];
          }
        }
      }
    }

    for (unsigned int i = 0; i < item->n_write(); ++i) {
      for (int j = idx - 1; j >= 0; --j) {
        ActionItem *aitem = pool[j];
        int outIdx = aitem->hasInput(item->write_attr[i], item->write_idx[i]);
        if (outIdx !=
            -1) {  // if item writes to aitem, union the two sets they belong to
          int otherSet = Sets[aitem->name()];
          int thisSet = Sets[item->name()];
          if (thisSet < otherSet) {
            for (std::map<std::string, int>::iterator it = Sets.begin();
                 it != Sets.end(); ++it) {
              if (it->second == otherSet) {
                it->second = thisSet;
              }
            }
            // Sets[aitem->name()] = Sets[item->name()];
          } else if (otherSet < thisSet) {
            for (std::map<std::string, int>::iterator it = Sets.begin();
                 it != Sets.end(); ++it) {
              if (it->second == thisSet) {
                it->second = otherSet;
              }
            }
            // Sets[item->name()] = Sets[aitem->name()];
          }
        }
      }
    }
  }
  std::cout << "I am within Scheduler: " << this->scheduler_name << std::endl;
  for (std::map<std::string, int>::iterator q = Sets.begin(); q != Sets.end();
       ++q) {
    std::cout << "Action: " << q->first << " is in set number: " << q->second
              << std::endl;
  }
}

// Data Dependency Graph
void Scheduler::buildDDG() {
#if 1
  ActionList pool = actions;
  discoverSets(pool);

  // search through the actionlist, create links and add to the same set
  for (unsigned int idx = 1; idx < pool.size(); ++idx) {
    ActionItem *item = pool[idx];
    // search all input
    for (unsigned int i = 0; i < item->n_read(); ++i) {
      for (int j = idx - 1; j >= 0; --j) {
        ActionItem *aitem = pool[j];
        int inIdx = aitem->hasOutput(item->read_attr[i], item->read_idx[i]);
        if (inIdx != -1) {  // if aitem writes to item
          item->input[i] = aitem;
          aitem->output[inIdx] = item;
        }
      }
    }
  }
  /* while (!pool.empty()) {
    // find one leaf with no output or output already fulfilled
    ActionList::iterator act = pool.end() - 1;
    for (; act >= pool.begin(); act--) {
      if ((*act)->fulfilled() == 1) {
        printf("FOUND LEAF: %s\n", (*act)->name());
        pool.erase(act);
      } else {
        break;
      }
    }
    if (act < pool.begin()) {  // error with loop
      printf("Loop detected!\n");
      COM_assertion(false);
    }

    ActionItem *item = *act;
    //pool.erase(act); // don't use iterator act afterwards

    if (item->n_read() == 0) {  // this is a tree root
      roots.push_back(item);
    } else {
      for (unsigned int i = 0; i < item->n_read(); i++) {
        if (item->input[i] == NULL) {
          // find matching input actions for item from pool
          ActionList::iterator a = pool.end() - 1;
          for (; a >= pool.begin(); a--) {
            int inIdx = (*a)->hasOutput(item->read_attr[i], item->read_idx[i]);
            if (inIdx != -1) {  // set up double link
              printf("updated output of %s for %s %d\n",
                    (*a)->name(), item->read_attr[i], item->read_idx[i]);
              item->input[i] = *a;
              (*a)->output[inIdx] = item;
              break;           // only do once
            }
          }
          if (a < pool.begin()) {  // a read without any matching write
            printf("Error: can not find matching input for action %s of attr:%s
  index:%d. \n",
                   (*a)->name(),
                   item->read_attr[i],
                   item->read_idx[i]
            );
          }
        }  // end of for i

      }
    }
  }  // end of while */
  /* ActionList::iterator act = pool.begin();
  while(!pool.empty()){
    (*act)->print(stdout);
    if((*act)->n_write() > 0){ // the action writes to other actions
      for (unsigned int i = 0; i < (*act)->n_write(); ++i) {
        if ((*act)->output[i] == NULL) {
          // find matching input actions for act from pool
          ActionList::iterator a = pool.begin();
          for (; a != pool.end(); ++a) {
            int inIdx = (*a)->hasInput((*act)->write_attr[i],
  (*act)->write_idx[i]); if (inIdx != -1) {  // set up double link
              if((*act)->output[i] == NULL){
                (*act)->output[i] = *a;
              }
              if((*a)->input[inIdx] == NULL){
                (*a)->input[inIdx] = (*act);
                std::cout << "Action " << (*act)->name() << " outputs to " <<
  (*a)->name() << " index: " << (*a)->read_idx[i] << std::endl;
              }
            }
          }
          if ((*act)->output[i] == NULL) {  // a read without any matching write
            std::cout << "Error: can not find matching input for action " <<
  (*act)->name() << "of attr:" << (*a)->read_attr[i] << "index:" <<
  (*a)->read_idx[i] << std::endl; COM_assertion(false);
          }
        }
      } // end of for i
    }

    if ((*act)->fulfilled() == 1) {
      std::cout << "FOUND LEAF:" << (*act)->name() << std::endl;
      if(pool.size() == 1) return;
      else {
        if(act == --(pool.end()) ){
          ActionList::iterator temp = act;
          act = pool.begin();
          pool.erase(temp);
        }
        else{
          act = pool.erase(act);
        }
      }
    } else {
      act = (act == --(pool.end()) ) ?
        pool.begin()
        : act + 1;
    } */

  /* if (act < pool.begin()) {  // error with loop
    printf("Loop detected!\n");
    COM_assertion(false);
  }
}*/

  // if the action is not dependent on anything, then add it to the roots list
  for (int i = 0; i < actions.size(); ++i) {
    for (int j = 0; j < actions[i]->n_read(); ++j) {
      if (actions[i]->input[j] != NULL) {  // this is a tree root
        roots.push_back(actions[i]);
        // pool[j]->print(stdout);
      }
    }
  }
  std::cout << "Made it to the end of the scheduler" << this->scheduler_name << std::endl;
#else
  // make links
  ActionList pool = actions;

  for (unsigned int idx = 1; idx < pool.size(); ++idx) {
    ActionItem *item = pool[idx];
    // search all input
    for (unsigned int i = 0; i < item->n_read(); ++i) {
      for (int j = idx - 1; j >= 0; --j) {
        ActionItem *aitem = pool[j];
        int inIdx = aitem->hasOutput(item->read_attr[i], item->read_idx[i]);
        if (inIdx != -1) {
          item->input[i] = aitem;
          aitem->output[inIdx] = item;
        }
      }
    }
  }

  //  identify root nodes
  for (unsigned int idx = 0; idx < pool.size(); ++idx) {
    ActionItem *item = pool[idx];
    int isroot = 1;
    for (unsigned int i = 0; i < item->n_read(); ++i)
      if (item->input[i] != NULL) {
        isroot = 0;
        break;
      }
    if (isroot) roots.push_back(item);
  }
#endif
}

void Scheduler::topological_sort() {
  ActionList pool;
  pool = actions;
  while (!pool.empty()) {
    ActionList::iterator aitem;
    for (aitem = pool.begin(); aitem != pool.end(); ++aitem) {
      if ((*aitem)->n_read() == 0) break;  // this action is a root
      // check every input
      int flag = 1;
      for (unsigned int i = 0; i < (*aitem)->n_read(); ++i) {
        ActionItem *in = (*aitem)->input[i];
        // search in sort
        ActionList::iterator s;
        for (s = sort.begin(); s != sort.end(); ++s)
          if ((*s)->action() == in->action())
            break;              // upstream action has been found in the sort
        if (s == sort.end()) {  // no upstream action has been found
          flag = 0;
          break;
        }
      }
      if (flag) break;
    }
    if (aitem != pool.end()) {
      sort.push_back(*aitem);
      pool.erase(aitem);
    } else
      break;  // the pool is now empty
  }

  if (!pool.empty()) {
    printf("ERROR in sorting!\n");
    exit(1);
  }
}

void Scheduler::sanityCheck() {
  // make sure all input and output are not null
  for (ActionList::iterator aitem = actions.begin(); aitem != actions.end();
       ++aitem) {
    ActionItem *item = *aitem;
    unsigned int i;
    COM_assertion_msg(item->n_read() == item->input.size(),
                      "ERROR: Scheduler::sanityCheck failed.\n");
    for (i = 0; i < item->n_read(); ++i)
      COM_assertion_msg(item->input[i] != NULL,
                        "ERROR: Scheduler::sanityCheck failed at 2.\n");
    COM_assertion_msg(item->n_write() == item->output.size(),
                      "ERROR: Scheduler::sanityCheck failed at 3.\n");
    for (i = 0; i < item->n_write(); ++i)
      COM_assertion_msg(item->output[i] != NULL,
                        "ERROR: Scheduler::sanityCheck failed at 4.\n");
  }
}

void Scheduler::print_helper(FILE *f, ActionItem *aitem) {
  if (aitem->print_flag == 1) return;

  aitem->print_flag = 1;
  aitem->action()->print(f);

  unsigned int i;
  for (i = 0; i < aitem->n_read(); ++i) {
    if (aitem->input[i])
      fprintf(
          f,
          "edge: { sourcename: \"%s\" targetname: \"%s\" label: \"%s,%d\"}\n",
          aitem->input[i]->name(), aitem->name(), aitem->read_attr[i],
          aitem->read_idx[i]);
  }

  for (i = 0; i < aitem->n_write(); ++i)
    if (aitem->output[i]) print_helper(f, aitem->output[i]);
}

void Scheduler::print_toposort(FILE *f) {
  for (unsigned int i = 0; i < sort.size(); ++i)
    sort[i]->action()->print_toposort(f);
}

void Scheduler::printActions() {
  for (unsigned int i = 0; i < actions.size(); ++i) actions[i]->print(stdout);
  printf("\n");
}

// return true if all output actions satisfied
int Scheduler::ActionItem::fulfilled() {
  for (unsigned int i = 0; i < n_write(); ++i)
    if (output[i] == NULL) return 0;
  for (unsigned int j = 0; j < n_read(); ++j)
    if (input[j] == NULL) return 0;
  return 1;
}

int Scheduler::ActionItem::hasInput(const char *attr, int idx) {
  for (unsigned int i = 0; i < n_read(); ++i)
    if (strcasecmp(attr, read_attr[i]) == 0 && idx == read_idx[i]) return i;
  return -1;
}

int Scheduler::ActionItem::hasOutput(const char *attr, int idx) {
  for (unsigned int i = 0; i < n_write(); ++i)
    if (strcasecmp(attr, write_attr[i]) == 0 && idx == write_idx[i]) return i;
  return -1;
}

void Scheduler::ActionItem::print(FILE *f) {
  unsigned int i;
  fprintf(f, "=========== Action %s =============\n", name());
  for (i = 0; i < n_read(); ++i)
    printf("reads: %s %d \n", read_attr[i], read_idx[i]);
  for (i = 0; i < n_write(); ++i)
    printf("writes: %s %d \n", write_attr[i], write_idx[i]);
}
