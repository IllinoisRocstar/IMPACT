//
// Created by agondolo on 9/5/18.
//

#include "SchedulerAction.h"

// SchedulerAction

SchedulerAction::~SchedulerAction() { delete sched; }

void SchedulerAction::init(double t) { sched->init_actions(t); }

void SchedulerAction::run(double t, double dt, double alpha) {
  // sched->set_alpha(alpha);
  sched->run_actions(t, dt);
}

void SchedulerAction::finalize() { sched->finalize_actions(); }

void SchedulerAction::print(FILE *f, char *container_name) {
  fprintf(f,
          "graph: { title: \"%s\" label: \"%s\" \n\
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
          name(), name());
  sched->print(f, container_name);
  fprintf(f, "}\n");
}

void SchedulerAction::print_toposort(FILE *f) {
  fprintf(f, "( ");
  sched->print_toposort(f);
  fprintf(f, ") ");
}

void SchedulerAction::schedule() { sched->schedule(); }
