#include "SchedulerAction.h"

SchedulerAction::~SchedulerAction() { delete sub_schedule; }

void SchedulerAction::init(double t) { sub_schedule->init_actions(t); }

void SchedulerAction::run(double t, double dt, double alpha) {
  sub_schedule->run_actions(t, dt, alpha);
}

void SchedulerAction::finalize() { sub_schedule->finalize_actions(); }

void SchedulerAction::print(FILE *f) const {
  fprintf(f, "graph: { title: \"%s\" label: \"%s\" \n\
        status: folded \n\
        display_edge_labels: yes \n\
        layoutalgorithm: tree   \n\
        scaling: maxspect   \n\
        color :  red           \n\
        node.color     : black   \n\
        node.textcolor : red   \n\
        node.bordercolor: black \n\
        node.borderwidth: 1    \n\
        edge.color     : lightblue   \n\
        edge.arrowsize : 7   \n\
        edge.thickness : 2   \n\
        edge.fontname:\"helvO08\"  \n\
        node.label: \"no type\" \n",
          name().c_str(), name().c_str());
  sub_schedule->print(f, this->name().c_str());
  fprintf(f, "}\n");
}

void SchedulerAction::print_toposort(FILE *f) const {
  fprintf(f, "( ");
  sub_schedule->print_toposort(f);
  fprintf(f, ") ");
}

void SchedulerAction::schedule() { sub_schedule->schedule(); }
