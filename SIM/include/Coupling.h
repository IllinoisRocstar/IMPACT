//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef _COUPLING_H_
#define _COUPLING_H_

#include "Agent.h"
#include "com.h"
//#include "Scheduler.h"

typedef std::vector<Agent *> AgentList;

// coupling
class Coupling : public COM_Object {
 private:
  Scheduler init_scheduler;
  Scheduler scheduler;

 protected:
  std::string coupling_name;
  AgentList agents;

  std::vector<std::string> modules;
  int comm_rank;
  int init_started;   // initial_start
  int restarting;     // in restarting
  int init_remeshed;  // initialization after remeshing
  // PC iteration
  int iPredCorr;    // class
  int maxPredCorr;  // modified

  std::string restartInfo;

  // compute integrals
  int overwrite_integ;
  std::string integFname;

  int overwrite_dist;
  std::string distFname;

 public:
  /// Constructor. Derived class will add actions for the coupling scheme
  Coupling(const char *coupl_name) : coupling_name(coupl_name) {}

  /// Destructor
  virtual ~Coupling();

  const char *name() { return coupling_name.c_str(); }

  /// Add new agent.
  Agent *add_agent(Agent *);

  /// Schedule the top-level actions of the coupling scheme and
  /// the actions of the agents.
  void schedule();

  /// Invoke initialization of the actions in the scheduler and the agents
  void init(double t, double dt, int reinit = 0);

  /// Invoke finalization of the actions in the scheduler and the agents
  void finalize();

  /// Invoke the scheduler
  double run(double t, double dt, int iPredCorr, double zoom);
  void run_initactions(double t, double dt);

  /// Invoke input functions of the agents
  void input(double t);

  ///
  int get_ipc() const { return iPredCorr; }
  int get_max_ipc() const { return maxPredCorr; }
  void set_max_ipc(int max_ipc) { maxPredCorr = max_ipc; }

  int initial_start() const { return init_started; }
  int in_restart() const { return restarting; }

  /// true if in initialization step and remeshed is true
  int initial_remeshed() const { return init_remeshed; }

  virtual int new_start(double t) const;

  virtual void init_convergence(int iPredCorr) {}
  virtual int check_convergence();

  virtual void update_integrals(double currentTime) {}
  virtual void update_distances(double currentTime) {}

  // Write out restart files (including visualization data)
  void output_restart_files(double t);

  // Write out visualization files
  void output_visualization_files(double t);

  // print for visualization
  void print(const char *fname);

  virtual Scheduler &get_init_scheduler() { return init_scheduler; }
  virtual Scheduler &get_scheduler() { return scheduler; }

 protected:
  void callMethod(Scheduler_voidfn1_t fn, double t);
};

#endif
