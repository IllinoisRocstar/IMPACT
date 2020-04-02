//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef _COUPLING_H_
#define _COUPLING_H_

#include "com.h"

#include "Agent.h"
#include "UserScheduler.h"

/**
 * A Coupling represents the coupling scheme used to combine physics modules in
 * a simulation.
 *
 * A Coupling is composed of a number of Agent and a main Scheduler. Its
 * constructor will construct the Agent and Scheduler and add Action to define
 * the coupling algorithm. Then the order of initialization, execution, and
 * finalization is determined with schedule() method. The init() and finalize()
 * methods will initialize and finalize the Agent and Action in the Scheduler,
 * respectively. The run() procedure will take the current time and
 * pre-determined time step, run the Scheduler with the main physics Action from
 * each Agent, and return the new time.
 */
class Coupling : public COM_Object {
public:
  /**
   * Constructor. Derived class will add actions for the coupling scheme
   * @param coupl_name name of coupling
   * @param com MPI communicator
   */
  explicit Coupling(std::string coupl_name, MPI_Comm com);
  ~Coupling() override;

public:
  /**
   * @name Metadata getters
   */
  ///@{
  /**
   * Get Coupling name
   * @return coupling name
   */
  const std::string &get_coupling_name() const { return coupling_name; }
  ///@}

  /**
   * @name Agent interaction
   */
  ///@{
  /**
   * Add new Agent
   * @return returns pointer to added Agent
   */
  Agent *add_agent(Agent *);
  ///@}

  /**
   * @name Driver interaction
   */
  ///@{
  /**
   * Schedule the top-level actions of the coupling scheme and the actions of
   * the agents.
   */
  void schedule();
  /**
   * Invoke initialization of the actions in the scheduler and the agents
   * @param t current time
   * @param dt current time step
   * @param reinit re-initialization (Default: false)
   */
  void init(double t, double dt, bool reinit = false);
  /**
   * Invoke the scheduler
   * @param t current time
   * @param dt current time step
   * @param iPredCorr predictor-corrector iteration
   * @param zoom time zoom level
   * @return new time
   */
  double run(double t, double dt, int iPredCorr, double zoom);
  /**
   * Invoke finalization of the actions in the scheduler and the agents
   */
  void finalize();
  ///@}

  void run_initactions(double t, double dt);

  /**
   * @name Predictor-corrector methods
   */
  ///@{
  /**
   * Get current PC step
   * @return
   */
  int get_ipc() const { return iPredCorr; }
  /**
   * Get maximum PC steps
   * @return
   */
  int get_max_ipc() const { return maxPredCorr; }
  /**
   * Set maximum PC steps
   * @param max_ipc
   */
  void set_max_ipc(int max_ipc) { maxPredCorr = max_ipc; }
  /**
   * Initialize the PC steps
   * @param iPredCorr_ Current PC step
   */
  void init_convergence(int iPredCorr_);
  /**
   * Check for PC convergence
   * @return
   */
  bool check_convergence();
  ///@}

  bool is_initial_start() const { return init_started; }
  bool is_restart() const { return restarting; }
  /// true if in initialization step and remeshed is true
  bool is_initial_remeshed() const { return init_remeshed; }
  virtual int new_start(double t) const;

  /**
   * @name I/O Methods
   */
  ///@{
  /**
   * Invoke input functions of the agents
   * @param t current time
   */
  void input(double t);
  /**
   * Write out restart files (including visualization data)
   * @param t current time
   */
  void output_restart_files(double t);
  /**
   * Write out visualization files
   * @param t current time
   */
  void output_visualization_files(double t);
  /**
   * Read the restart info file
   * @param CurrentTime current time
   * @param iStep current time step
   */
  void read_restart_info(double CurrentTime, int iStep);
  /**
   * Write to the restart info file
   * @param CurrentTime current time
   * @param iStep current time step
   */
  void write_restart_info(double CurrentTime, int iStep);
  ///@}

  /**
   * Print to file in GDL.
   * @param fname file name
   */
  void print(const char *fname) const;

protected:
  /**
   * Call the same method on all Scheduler owned by Coupling and its Agent.
   * @param fn member function pointer to Scheduler method
   * @param t time to pass to Scheduler method
   */
  void callMethod(Scheduler_voidfn1_t fn, double t);

protected:
  typedef std::vector<Agent *> AgentList;

protected:
  Scheduler *init_scheduler; ///< Initialization scheduler called through init()
  Scheduler *scheduler;      ///< Runtime Scheduler called through run()

  /**
   * @name Metadata getters
   */
  ///@{
  const std::string coupling_name; ///< Coupling name
  AgentList agents;                ///< List of all agents registered
  MPI_Comm communicator;           ///< MPI Communicator
  int comm_rank;                   ///< MPI rank
  std::vector<std::string> modules;
  ///@}

  bool init_started{true};   ///< initial start
  bool init_remeshed{false}; ///< initialization after remeshing
  bool restarting{false};    ///< in restarting
  std::string restartInfo;   ///< Name of Restart info file

  /**
   * @name Predictor-corrector iteration data
   */
  ///@{
  int iPredCorr{0};   ///< current iteration number
  int maxPredCorr{1}; ///< maximum iterations allowed
  ///@}
};

#endif
