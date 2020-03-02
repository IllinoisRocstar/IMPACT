#include "DDGScheduler.h"

#include "com.h"

void DDGScheduler::schedule() {
  if (scheduled)
    COM_abort_msg(EXIT_FAILURE, "ERROR: Scheduler has already been scheduled.");

  // TODO: Remove hard-coded prints to stdout. Use a logger class.
  FILE *f = stdout;

  // schedule all sub-schedulers
  for (auto &&action : actions)
    action->action()->schedule();

  // debugging
  if (verbose) {
    fprintf(f, "-------------- DDGScheduler %s -------------\n",
            scheduler_name.c_str());
    printActions(f);
  }

  buildDDG();

  sanityCheck();

  topological_sort();

  // debugging
  if (verbose) {
    fprintf(f, "Topological sort:\n");
    print_toposort(f);
    fprintf(f, "\n");
  }

  scheduled = true;
}

/**
 * To guarantee the order that actions are added will not affect the result,
 * the Data Dependency Graph (DDG) must respect three conditions:
 *   1. All reads/inputs must be fulfilled
 *   2. No duplicate writes/outputs
 *   3. No loops.
 *
 * The algorithm will search through the pool of actions by fulfilling reads.
 */
void DDGScheduler::buildDDG() {
  ActionList pool = actions;
  // discoverSets(pool);

  auto act = pool.begin();
  while (!pool.empty()) {
    if ((*act)->n_read() > 0) { // the action reads from other actions
      for (unsigned int i = 0; i < (*act)->n_read(); ++i) {
        if ((*act)->input[i] == nullptr) {
          // find matching output actions for act from pool
          for (const auto &a : pool) {
            int outIdx =
                a->hasOutput((*act)->read_attr[i], (*act)->read_idx[i]);
            if (outIdx != -1) {
              // set up double link
              if ((*act)->input[i] != nullptr)
                COM_abort_msg(
                    EXIT_FAILURE,
                    "Error: Duplicate input. Action " + (*act)->name() +
                        " can take input (attr: " + (*act)->write_attr[i] +
                        " index: " + std::to_string((*act)->write_idx[i]) +
                        ") from either Action " + a->name() + " or Action " +
                        (*act)->input[i]->name());

              (*act)->input[i] = a;

              if (a->output[outIdx] != nullptr)
                COM_abort_msg(EXIT_FAILURE,
                              "Error: Duplicate output. Action " + a->name() +
                                  " can provide output to either Action " +
                                  (*act)->name() + " or Action " +
                                  a->output[outIdx]->name());

              a->output[outIdx] = (*act);
            }
          }

          // a read without any matching write
          if ((*act)->input[i] == nullptr)
            COM_abort_msg(EXIT_FAILURE,
                          "Error: Cannot find matching output for Action " +
                              (*act)->name() +
                              " of attr: " + (*act)->read_attr[i] +
                              " index: " + std::to_string((*act)->read_idx[i]));
        }
      }
    } else if ((*act)->n_write() > 0) { // the action only writes
      for (auto i = 0; i < (*act)->n_write(); ++i) {
        if ((*act)->output[i] == nullptr) {
          // find matching input actions for act from pool
          for (const auto &a : pool) {
            int inIdx =
                a->hasInput((*act)->write_attr[i], (*act)->write_idx[i]);
            if (inIdx != -1) {
              // set up double link
              if ((*act)->output[i] != nullptr)
                COM_abort_msg(
                    EXIT_FAILURE,
                    "Error: Duplicate output. Action " + (*act)->name() +
                        " can provide output to either " + a->name() +
                        " index: " + std::to_string(a->read_idx[i]) +
                        " or from " + (*act)->output[i]->name() + " index: " +
                        std::to_string((*act)->output[i]->read_idx[i]));

              (*act)->output[i] = a;

              if (a->input[inIdx] != nullptr)
                COM_abort_msg(
                    EXIT_FAILURE,
                    "Error: Duplicate input. Action " + a->name() +
                        " can take input (attr: " + (*act)->write_attr[i] +
                        " index: " + std::to_string((*act)->write_idx[i]) +
                        ") from either Action " + (*act)->name() +
                        " or Action " + a->input[inIdx]->name());

              a->input[inIdx] = (*act);
            }
          }

          // a write without any matching read
          if ((*act)->output[i] == nullptr)
            COM_abort_msg(
                EXIT_FAILURE,
                "Error: Cannot find matching input to Action " +
                    (*act)->name() + " of attr: " + (*act)->write_attr[i] +
                    " index: " + std::to_string((*act)->write_idx[i]));
        }
      }
    }

    if ((*act)->fulfilled() == 1) {
      if (pool.size() == 1)
        return;
      else {
        if (act == --(pool.end())) {
          auto temp = act;
          act = pool.begin();
          pool.erase(temp);
        } else {
          act = pool.erase(act);
        }
      }
    } else {
      act = (act == --(pool.end())) ? pool.begin() : act + 1;
    }

    if (act < pool.begin())
      COM_abort_msg(EXIT_FAILURE, "ERROR: Unexpected loop detected!");
  }

  // if the action is not dependent on anything, then add it to the roots list
  for (const auto &action : actions)
    if (action->n_read() == 0)
      roots.push_back(action);
}

/**
 * Make sure all input and output are not null
 */
void DDGScheduler::sanityCheck() const {
  for (const auto &item : actions) {
    if (item->n_read() != item->input.size())
      COM_abort_msg(EXIT_FAILURE, "Error: Scheduler::sanityCheck failed.");
    for (auto i = 0; i < item->n_read(); ++i)
      if (item->input[i] == nullptr)
        COM_abort_msg(EXIT_FAILURE,
                      "Error: Scheduler::sanityCheck failed at 2.");
    if (item->n_write() != item->output.size())
      COM_abort_msg(EXIT_FAILURE, "Error: Scheduler::sanityCheck failed at 3.");
    for (auto i = 0; i < item->n_write(); ++i)
      if (item->output[i] == nullptr)
        COM_abort_msg(EXIT_FAILURE,
                      "Error: Scheduler::sanityCheck failed at 4.");
  }
}

/**
 * Sort the actions into a flattened tree. The sorting will fail iff there is
 * a cycle in the graph. If such a cycle is found, an error is outputted.
 */
void DDGScheduler::topological_sort() {
  ActionList pool = actions;

  while (!pool.empty()) {
    ActionList::iterator aitem;

    // Find next action to add to `sort`
    for (aitem = pool.begin(); aitem != pool.end(); ++aitem) {
      if ((*aitem)->n_read() == 0)
        break; // this action is a root

      // check every input
      bool fulfilled = true;
      for (unsigned int i = 0; i < (*aitem)->n_read(); ++i) {
        ActionItem *in = (*aitem)->input[i];

        // search in sort
        ActionList::iterator s;
        for (s = sort.begin(); s != sort.end(); ++s)
          if ((*s)->action() == in->action())
            break; // upstream action has been found in the sort

        if (s == sort.end()) { // no upstream action has been found
          fulfilled = false;
          break;
        }
      }
      if (fulfilled)
        break;
    }

    // Add found actions
    if (aitem != pool.end()) {
      sort.push_back(*aitem);
      pool.erase(aitem);
    } else {
      break; // Dependency has not been resolved.
    }
  }

  // Not all actions resolved. There must be a cycle in the graph.
  if (!pool.empty())
    COM_abort_msg(EXIT_FAILURE, "Error: Cycle detected in the DDG graph "
                                "during the scheduling of actions!");
}

void DDGScheduler::discoverSets(const ActionList &pool) {
  std::map<std::string, int> Sets;
  // Add all action names to the Map, each in their own set
  for (int y = 0; y < pool.size(); ++y)
    Sets[std::string(pool[y]->name())] = y;

  for (unsigned int idx = 1; idx < pool.size(); ++idx) {
    ActionItem *item = pool[idx];
    // search through the actionlist, create links and add to the same set
    for (unsigned int i = 0; i < item->n_read(); ++i) {
      for (int j = idx - 1; j >= 0; --j) {
        ActionItem *aitem = pool[j];
        int inIdx = aitem->hasOutput(item->read_attr[i], item->read_idx[i]);
        if (inIdx != -1) { // if aitem writes to item
          // if aitem writes to item, add them to the same set
          if (Sets[item->name()] < Sets[aitem->name()])
            Sets[aitem->name()] = Sets[item->name()];
          else
            Sets[item->name()] = Sets[aitem->name()];
        }
      }
    }

    for (unsigned int i = 0; i < item->n_write(); ++i) {
      for (int j = idx - 1; j >= 0; --j) {
        ActionItem *aitem = pool[j];
        int outIdx = aitem->hasInput(item->write_attr[i], item->write_idx[i]);
        if (outIdx != -1) {
          // if item writes to aitem, union the two sets they belong to
          int otherSet = Sets[aitem->name()];
          int thisSet = Sets[item->name()];
          if (thisSet < otherSet) {
            for (auto &&Set : Sets)
              if (Set.second == otherSet)
                Set.second = thisSet;
            // Sets[aitem->name()] = Sets[item->name()];
          } else if (otherSet < thisSet) {
            for (auto &&Set : Sets)
              if (Set.second == thisSet)
                Set.second = otherSet;
            // Sets[item->name()] = Sets[aitem->name()];
          }
        }
      }
    }
  }
  /* std::cout << "I am within Scheduler: " << this->scheduler_name <<
  std::endl; for (std::map<std::string, int>::iterator q = Sets.begin(); q !=
  Sets.end();
       ++q) {
    std::cout << "Action: " << q->first << " is in set number: " << q->second
              << std::endl;
  } */
}
