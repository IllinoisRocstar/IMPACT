//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <cstring>

#include "Scheduler.h"
#include "com.h"

Scheduler::Scheduler(bool verbose_)
    : scheduler_name("Scheduler"), scheduled(false), inited(false),
      verbose(verbose_) {}

Scheduler::~Scheduler() {
  for (auto &&aitem : actions)
    delete aitem;
}

void Scheduler::add_action(Action *action) {
  auto *aitem = new ActionItem(action);
  actions.push_back(aitem);

  // call back action
  action->declare(*this);
}

void Scheduler::reads(const Action *a, const std::string &attr, int idx) {
  // FIXME: locate this action, doing linear search now
  ActionList::iterator aitem;
  for (aitem = actions.begin(); aitem != actions.end(); ++aitem)
    if ((*aitem)->action() == a)
      break;

  if (aitem == actions.end())
    COM_abort_msg(EXIT_FAILURE, "IMPACT ERROR: Action '" + a->name() +
                                    "' not registered to Scheduler '" +
                                    scheduler_name + "'.");

  (*aitem)->read_attr.push_back(attr);
  (*aitem)->read_idx.push_back(idx);
  (*aitem)->input.push_back(nullptr);
}

void Scheduler::writes(const Action *a, const std::string &attr, int idx) {
  // locate this action
  ActionList::iterator aitem;
  for (aitem = actions.begin(); aitem != actions.end(); ++aitem)
    if ((*aitem)->action() == a)
      break;

  if (aitem == actions.end())
    COM_abort_msg(EXIT_FAILURE, "IMPACT ERROR: Action '" + a->name() +
                                    "' not registered to Scheduler '" +
                                    scheduler_name + "'.");

  (*aitem)->write_attr.push_back(attr);
  (*aitem)->write_idx.push_back(idx);
  (*aitem)->output.push_back(nullptr);
}

void Scheduler::init_actions(double t) {
  if (!scheduled)
    COM_abort_msg(EXIT_FAILURE, "IMPACT ERROR: Scheduler '" + scheduler_name +
                                    "' has not been scheduled.");

  if (inited)
    return;

  // do in sorted order
  for (auto &&item : sort)
    item->action()->init(t);

  inited = true;
}

void Scheduler::run_actions(double t, double dt, double alpha) {
  if (!inited)
    COM_abort_msg(EXIT_FAILURE,
                  "IMPACT ERROR: Scheduler '" + scheduler_name +
                      "'has not been initialized when calling run_actions().");

  // do in sorted order
  for (auto &&item : sort)
    item->action()->run(t, dt, alpha);
}

void Scheduler::finalize_actions() {
  if (!inited)
    COM_abort_msg(EXIT_FAILURE, "IMPACT ERROR: Scheduler '" + scheduler_name +
                                    "' has not been initialized when calling "
                                    "finalize_actions().");

  if (sort.empty())
    return;

  // do in reversed order
  for (auto aitem = sort.rbegin(); aitem != sort.rend(); ++aitem)
    (*aitem)->action()->finalize();
}

void Scheduler::print(const char *fname) const {
  FILE *f = fopen(fname, "w");

  fprintf(f, "graph: { title: \"DependenceTree\" \n\
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

char *Scheduler::print(FILE *f, const char *container_name) const {
  if (actions.empty())
    return nullptr;

  std::string sched_name = container_name;
  sched_name = sched_name + "-" + name();
  fprintf(f, "graph: { title: \"%s\" label: \"%s\" \n\
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

  for (auto &&action : actions)
    action->print_flag = 0;

  for (auto &&root : roots)
    print_helper(f, root);

  fprintf(f, "}\n");

  return strdup(sched_name.c_str());
}

void Scheduler::print_toposort(FILE *f) const {
  for (const auto &actionItem : sort)
    actionItem->action()->print_toposort(f);
}

void Scheduler::print_helper(FILE *f, const ActionItem *aitem) {
  if (aitem->print_flag == 1)
    return;

  aitem->print_flag = 1;
  aitem->action()->print(f);

  for (unsigned int i = 0; i < aitem->n_read(); ++i)
    if (aitem->input[i])
      fprintf(
          f,
          "edge: { sourcename: \"%s\" targetname: \"%s\" label: \"%s,%d\"}\n",
          aitem->input[i]->name().c_str(), aitem->name().c_str(),
          aitem->read_attr[i].c_str(), aitem->read_idx[i]);

  for (unsigned int i = 0; i < aitem->n_write(); ++i)
    if (aitem->output[i])
      print_helper(f, aitem->output[i]);
}

void Scheduler::printActions(FILE *f) const {
  for (const auto &action : actions)
    action->print(f);
  fprintf(f, "\n");
}

bool Scheduler::ActionItem::fulfilledInput() const {
  for (auto j = 0; j < n_read(); ++j)
    if (input[j] == nullptr)
      return false;
  return true;
}

bool Scheduler::ActionItem::fulfilledOutput() const {
  for (auto i = 0; i < n_write(); ++i)
    if (output[i] == nullptr)
      return false;
  return true;
}

bool Scheduler::ActionItem::fulfilled() const {
  return fulfilledInput() && fulfilledOutput();
}

int Scheduler::ActionItem::hasInput(const std::string &attr, int idx) const {
  for (auto i = 0; i < n_read(); ++i)
    if (attr == read_attr[i] && idx == read_idx[i])
      return i;
  return -1;
}

int Scheduler::ActionItem::hasOutput(const std::string &attr, int idx) const {
  for (auto i = 0; i < n_write(); ++i)
    if (attr == write_attr[i] && idx == write_idx[i])
      return i;
  return -1;
}

void Scheduler::ActionItem::print(FILE *f) const {
  fprintf(f, "=========== Action %s =============\n", name().c_str());
  for (auto i = 0; i < n_read(); ++i)
    fprintf(f, "reads: %s %d \n", read_attr[i].c_str(), read_idx[i]);
  for (auto j = 0; j < n_write(); ++j)
    fprintf(f, "writes: %s %d \n", write_attr[j].c_str(), write_idx[j]);
}
