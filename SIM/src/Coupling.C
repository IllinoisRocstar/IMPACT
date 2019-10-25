//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <cmath>
#include <cstdlib>
#include <iostream>

#include "Coupling.h"

// Delete all agents
Coupling::~Coupling() {
  for (int i = 0, n = agents.size(); i < n; ++i) {
    delete agents[i];
  }
  agents.clear();
}

Agent *Coupling::add_agent(Agent *agent) {
  agents.push_back(agent);
  return agent;
}

// Schedule the actions for the scheduler and the agents
void Coupling::schedule() {
  get_init_scheduler().schedule();
  get_scheduler().schedule();

  for (int i = 0, n = agents.size(); i < n; ++i) {
    agents[i]->schedule();
  }
}

// Invoke initialization of the actions in the scheduler and the agents
void Coupling::init(double t, double dt, int reinit) {
  for (int i = 0, n = agents.size(); i < n; ++i) {
    agents[i]->init_module(t, dt);
    //    agents[i]->init_buffers(t);
  }

  // in warm restart, reset scheduler so that it can be re-inited
  if (reinit) callMethod(&Scheduler::restarting, t);

  callMethod(&Scheduler::init_actions, t);
}

// Invoke finalize of the actions in the scheduler and the agents
void Coupling::finalize() {
  get_init_scheduler().finalize_actions();
  get_scheduler().finalize_actions();

  for (int i = 0, n = agents.size(); i < n; ++i) agents[i]->finalize();
}

int Coupling::new_start(double t) const { return t == 0.0; }

double Coupling::run(double t, double dt, int pred, double zoom) {
  iPredCorr = pred;

  // Change dt to the maximum time step allowed.
  for (int i = 0, n = agents.size(); i < n; ++i) {
    dt = std::min(dt, agents[i]->max_timestep(t, dt));
  }

  get_scheduler().run_actions(t, dt);

  // Return the new time as t + dt
  if (zoom > 0.0)
    return t + dt * zoom;
  else
    return t + dt;
}

// invoke method fn to all schedulers, and all schedulers of agents
void Coupling::callMethod(Scheduler_voidfn1_t fn, double t) {
  int i, n;

  (get_init_scheduler().*fn)(t);

  for (i = 0, n = agents.size(); i < n; ++i) agents[i]->callMethod(fn, t);

  (get_scheduler().*fn)(t);
}

void Coupling::run_initactions(double t, double dt) {
  get_init_scheduler().set_alpha(0.0);
  get_init_scheduler().run_actions(t, dt);

  init_started = 0;
  init_remeshed = 0;
}

// Invoke input functions of the agents
void Coupling::input(double t) {
  for (int i = 0, n = agents.size(); i < n; ++i) {
    agents[i]->input(t);
  }
}

int Coupling::check_convergence() {
  int InterfaceConverged;
  if (maxPredCorr > 1) {
    InterfaceConverged = 0;
    for (int i = 0, n = agents.size(); i < n; ++i)
      if (!agents[i]->check_convergence()) return InterfaceConverged;
    InterfaceConverged = 1;
  } else
    InterfaceConverged = 1;
  return InterfaceConverged;
}

// Write out restart files (including visualization data)
void Coupling::output_restart_files(double t) {
  for (int i = 0, n = agents.size(); i < n; ++i) {
    agents[i]->output_restart_files(t);
  }
}

// Write out visualization files
void Coupling::output_visualization_files(double t) {
  for (int i = 0, n = agents.size(); i < n; ++i) {
    agents[i]->output_visualization_files(t);
  }
}

// print in GDL
void Coupling::print(const char *fname) {
  FILE *f = fopen(fname, "w");

  fprintf(f,
          "graph: { title: \"Coupling\" \n\
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

  //  get_scheduler().print(f, name());
  for (unsigned int i = 0; i < agents.size(); i++) {
    agents[i]->print(f);
    if (i > 0)
      fprintf(f, "edge: { sourcename: \"%s\" targetname: \"%s\" label: \"\"}\n",
              agents[i - 1]->get_agent_name().c_str(),
              agents[i]->get_agent_name().c_str());
  }

  fprintf(f, "} \n");
  fclose(f);
}
