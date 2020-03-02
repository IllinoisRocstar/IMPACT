//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <cstdio>
#include <iostream>
#include <utility>

#include "Coupling.h"

Coupling::Coupling(std::string coupl_name, MPI_Comm com)
    : coupling_name(std::move(coupl_name)), communicator(com),
      comm_rank(COMMPI_Comm_rank(com)) {
  scheduler = new UserScheduler();
  scheduler->set_name(coupling_name);
  init_scheduler = new UserScheduler();
  init_scheduler->set_name(coupling_name + "-init");
}

Coupling::~Coupling() {
  delete scheduler;
  delete init_scheduler;

  // Delete all agents
  for (auto &&agent : agents)
    delete agent;
}

Agent *Coupling::add_agent(Agent *agent) {
  agents.push_back(agent);
  return agent;
}

void Coupling::schedule() {
  init_scheduler->schedule();
  scheduler->schedule();

  for (auto &&agent : agents)
    agent->schedule();
}

void Coupling::init(double t, double dt, bool reinit) {
  // schedule all the agent and coupling schedulers
  schedule();

  for (auto &&agent : agents) {
    agent->init(t, dt);
    // agent->init_buffers(t);
  }

  // in warm restart, reset all schedulers so that they can be re-inited
  if (reinit)
    callMethod(&Scheduler::restarting, t);

  // initialize all schedulers
  callMethod(&Scheduler::init_actions, t);

  // initialize each modules by executing init schedulers
  run_initactions(t, dt);
}

void Coupling::finalize() {
  init_scheduler->finalize_actions();
  scheduler->finalize_actions();

  for (auto &&agent : agents)
    agent->finalize();
}

int Coupling::new_start(double t) const { return t == 0.0; }

double Coupling::run(double t, double dt, int pred, double zoom) {
  iPredCorr = pred;

  // Change dt to the maximum time step allowed.
  for (auto &&agent : agents)
    dt = std::min(dt, agent->max_timestep(t, dt));

  scheduler->run_actions(t, dt, -1.0);

  // Return the new time as t + dt
  if (zoom > 0.0)
    return t + dt * zoom;
  else
    return t + dt;
}

// invoke method fn to all schedulers, and all schedulers of agents
void Coupling::callMethod(Scheduler_voidfn1_t fn, double t) {
  (init_scheduler->*fn)(t);

  for (auto &&agent : agents)
    agent->callMethod(fn, t);

  (scheduler->*fn)(t);
}

void Coupling::run_initactions(double t, double dt) {
  init_scheduler->run_actions(t, dt, 0.0);

  init_started = false;
  init_remeshed = false;
}

void Coupling::input(double t) {
  for (auto &&agent : agents)
    agent->input(t);
}

void Coupling::init_convergence(int iPredCorr_) {
  if (maxPredCorr > 1 && iPredCorr_ > 0) {
    for (auto &agent : agents) {
      agent->init_convergence(iPredCorr_);
    }
  }
}

bool Coupling::check_convergence() {
  if (maxPredCorr > 1)
    for (auto &&agent : agents)
      if (!agent->check_convergence())
        return false;

  return true;
}

void Coupling::output_restart_files(double t) {
  for (auto &&agent : agents)
    agent->output_restart_files(t);
}

void Coupling::output_visualization_files(double t) {
  for (auto &&agent : agents)
    agent->output_visualization_files(t);
}

void Coupling::print(const char *fname) const {
  FILE *f = fopen(fname, "w");

  fprintf(f, "graph: { title: \"Coupling\" \n\
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

  scheduler->print(f, coupling_name.c_str());
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

void Coupling::read_restart_info(double CurrentTime, int iStep) {
  if (CurrentTime != 0.0) {
    FILE *fp = fopen(restartInfo.c_str(), "r");
    if (fp == nullptr) {
      COM_abort_msg(EXIT_FAILURE,
                    "IMPACT ERROR: Failed to read file " + restartInfo);
    }
    int curStep = iStep;
    double initialTime = CurrentTime;
    while (!feof(fp)) {
      fscanf(fp, "%d %le", &curStep, &initialTime);
    }
    fclose(fp);
    if (comm_rank == 0)
      std::cout << "IMPACT: Restart info file found with last iteration "
                << curStep << " at time " << initialTime << std::endl;
    /*
    // subtle - it starts from 0
    param->update_start_time(curStep - 1, initialTime);
    */
  } else {
    if (comm_rank == 0)
      std::cout << "IMPACT: This run is not a restart" << std::endl;
  }
}

void Coupling::write_restart_info(double CurrentTime, int iStep) {
  FILE *fp;
  if (comm_rank == 0) {
    if (CurrentTime == 0.0)
      fp = fopen(restartInfo.c_str(), "w");
    else
      fp = fopen(restartInfo.c_str(), "a");
    if (fp == nullptr) {
      COM_abort_msg(EXIT_FAILURE,
                    "IMPACT: Failed to open restart info file, " + restartInfo);
    }
    fprintf(fp, "%d %.5le \n", iStep, CurrentTime);
    fclose(fp);
  }
}
