//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** \file Agent.h
 *  * Contains declaration of the base class for Agent implementations.
 *   * @see Agent.C
 *    */

#ifndef _AGENT_H_
#define _AGENT_H_

#include <string>
#include <utility>
#include <vector>

#include "com.h"

#include "Action.h"
#include "UserScheduler.h"

class Agent;
class Coupling;
class InterpolateBase;

/**
 * Base class for registering a new DataItem to the Agent. Used by Actions and
 * Couplings to add new DataItems without needing to check prior existance or
 * memory management.
 */
class DataItemBase {
public:
  /**
   * Construct the base class
   * @param ag pointer to owning Agent
   * @param target_window_ COM Window name
   * @param attr_ COM DataItem attribute name
   */
  DataItemBase(Agent *ag, std::string target_window_, std::string attr_)
      : agent(ag), target_window(std::move(target_window_)),
        attr(std::move(attr_)) {}

  virtual ~DataItemBase() = default;

  /**
   * Create the desired window and/or data item.
   * @param bufname Name of window where to create the data item.
   */
  virtual void create(const std::string &bufname) = 0;
  void assign(const std::string &bufname);

public:
  Agent *agent;              ///< owning Agent
  std::string target_window; ///< Target Window name
  std::string attr;          ///< DataItem attribute name
};

/**
 * Add new DataItem using COM_new_dataitem()
 */
class NewDataItem : public DataItemBase {
public:
  /**
   * Add new DataItem
   * @param target_window Target window
   * @param target_attr Target attribute name
   * @param loc COM location ('w', 'p', 'n', or 'e')
   * @param type COM type (e.g., COM_INTEGER, COM_DOUBLE, etc.)
   * @param ncomp number of components
   * @param unit name of units
   */
  NewDataItem(Agent *ag, std::string target_window_, std::string attr_,
              char loc_, int type_, int ncomp_, std::string unit_);
  void create(const std::string &bufname) override;

protected:
  char loc;         ///< COM location ('w', 'p', 'n', or 'e')
  int type;         ///< COM type (e.g., COM_INTEGER, COM_DOUBLE, etc.)
  int ncomp;        ///< number of components
  std::string unit; ///< name of units
};

/**
 * Clone a DataItem using COM_clone_dataitem()
 */
class CloneDataItem : public DataItemBase {
public:
  /**
   * Add the clone of a DataItem
   * @param cond check existence of parent attribute before cloning
   * @param target_window Target window
   * @param target_attr Target attribute name
   * @param parent_window Parent window
   * @param parent_attr Parent attribute name
   * @param wg with ghosts (Default: 1)
   * @param ptnname Name of attribute to use as filter (Default: "")
   * @param val Value to pass to filter (Default: 0)
   */
  CloneDataItem(Agent *ag, bool cond, std::string target_window_,
                std::string attr_, std::string parent_window_,
                std::string parent_attr_, int wg_ = 1,
                std::string ptnname_ = "", int val_ = 0);
  void create(const std::string &bufname) override;

protected:
  bool condition; ///< check existence of parent attribute before cloning
  std::string parent_window; ///< Parent window
  std::string parent_attr;   ///< Parent attribute name
  int wg;                    ///< with ghosts (Default: 1)
  std::string ptnname; ///< Name of attribute to use as filter (Default: "")
  int val;             ///< Value to pass to filter (Default: 0)
};

/**
 * Use-inherit a DataItem using COM_use_dataitem()
 */
class UseDataItem : public DataItemBase {
public:
  /**
   * Add the use-inherit of a DataItem
   * @param target_window Target window
   * @param target_attr Target attribute name
   * @param parent_window Parent window
   * @param parent_attr Parent attribute name
   * @param wg with ghosts (Default: 1)
   * @param ptnname Name of attribute to use as filter (Default: "")
   * @param val Value to pass to filter (Default: 0)
   */
  UseDataItem(Agent *ag, std::string target_window_, std::string attr_,
              std::string parent_window_, std::string parent_attr_, int wg_ = 1,
              std::string ptnname_ = "", int val_ = 0);
  void create(const std::string &bufname) override;

protected:
  std::string parent_window; ///< Parent window
  std::string parent_attr;   ///< Parent attribute name
  int wg;                    ///< with ghosts (Default: 1)
  std::string ptnname; ///< Name of attribute to use as filter (Default: "")
  int val;             ///< Value to pass to filter (Default: 0)
};

/**
 * Main physics module action provided by .update_solution
 */
class PhysicsAction : public Action {
public:
  explicit PhysicsAction(Agent *ag);

  void init(double t) override{};
  void run(double t, double dt, double alpha) override;
  void finalize() override{};

  void print(FILE *f) const override;

private:
  Agent *agent;
};

/**
 * An Agent serves a physics module to a Coupling.
 *
 * In the constructor, an Agent will:
 * - load a physics module into COM
 * - define the persistent buffer data
 *
 * In the init() method, an Agent will:
 * - initialize the physics module
 * - populate the buffer data
 *
 * In the finalize() method, an Agent will:
 * - deallocate buffer data
 * - finalize the physics module
 *
 * In the destructor, an Agent will:
 * - unload a physics module
 *
 * The Agent is responsible for providing access to its buffer data. It provides
 * input() and output() methods to read and write the data, used by coupling
 * schemes to load data into physics module buffers and to take snapshots for
 * visualization and restart capabilities.
 *
 * The Agent provides one main action to the Coupling. This action invokes the
 * solve method of the physics module. It is accessible through the
 * get_main_action() method for scheduling.
 *
 * The Agent interacts with schedulers on behalf of the physics module to
 * provide boundary conditions and grid motion. These are provided to the
 * physics module as callback routines, obtain_bc() and obtain_gm(),
 * respectively, so control is kept in the physics module. For example, the
 * callback routines can be used for subcycling or Runge-Kutta iterations.
 * Subactions to these schedulers can be registered by calling add_bcaction()
 * and add_gmaction(), respectively.
 *
 */
class Agent : public COM_Object {
  friend class PhysicsAction;

public:
  /**
   * Construct an Agent
   * @param cp pointer to owning Coupling
   * @param agentname name of the agent
   * @param com MPI communicator
   * @param modulelname name of module library
   * @param modulewname name of window to pass to module at load time
   * @param surfname prefix to use for surface buffers
   * @param volname prefix to use for volume buffers
   */
  Agent(Coupling *cp, std::string agentname, MPI_Comm com,
        std::string modulelname, std::string modulewname, std::string surfname,
        std::string volname);
  ~Agent() override;

public:
  /**
   * @name Metadata getters
   */
  ///@{
  /**
   * Get pointer to Coupling owning the Agent
   * @return pointer to Coupling
   */
  Coupling *get_coupling() { return coupling; }
  /**
   * Get name of Agent
   * @return Agent name
   */
  const std::string &get_agent_name() const { return agent_name; }
  /**
   * Get MPI communicator
   * @return MPI Communicator
   */
  MPI_Comm get_communicator() const { return communicator; }
  /**
   * Get MPI comm rank
   * @return MPI rank
   */
  int get_comm_rank() const { return comm_rank; }
  /**
   * Get physics module library name
   * @return
   */
  const std::string &get_module_lname() const { return module_lname; }
  /**
   * Get physics module window name
   * @return
   */
  const std::string &get_module_wname() const { return module_wname; }
  ///@}

  /**
   * @name Module interaction
   * Methods to interact with the physics module.
   */
  ///@{
  /**
   * Get name of solver surface window.
   * @return surface window name
   */
  const std::string &get_surface_window() const { return surf_window; }
  /**
   * Get name of solver volume window.
   * @return volume window name
   */
  const std::string &get_volume_window() const { return vol_window; }
  /**
   * Get the update solution method of the physics module
   * @return PhysicsAction pointer
   */
  Action *get_main_action() { return physics_action; }
  ///@}

  /**
   * @name Scheduler interaction
   * Used in constructor of Coupling to define a coupling scheme.
   */
  ///@{
  /**
   * Add Action to initialization callback Scheduler.
   * @param act pointer to an Action
   */
  void add_icaction(Action *act);
  /**
   * Add Action to boundary condition initialization Scheduler.
   * @param act pointer to an Action
   */
  void add_bcinitaction(Action *act);
  /**
   * Add Action to boundary condition Scheduler at level l.
   * @param act pointer to an Action
   * @param l level (Default: 1)
   */
  void add_bcaction(Action *act, int l = 1);
  /**
   * Add Action to grid motion Scheduler.
   * @param act pointer to an Action
   */
  void add_gmaction(Action *act);
  /**
   * Call the same method on all Scheduler owned by Agent.
   * @param fn member function pointer to Scheduler method
   * @param t time to pass to Scheduler method
   */
  void callMethod(Scheduler_voidfn1_t fn, double t);
  /**
   * Call Scheduler::schedule() method on all Scheduler owned by Agent.
   */
  void schedule();
  ///@}

  /**
   * @name Coupling interaction
   */
  ///@{
  /**
   * Initialize the physics module and allocate buffer data.
   * @param t current time
   * @param dt current time step
   */
  void init(double t, double dt);
  /**
   * Determine the maximum time step allowed by physics module.
   * @param t current time
   * @param dt current time step
   * @return maximum time step
   */
  virtual double max_timestep(double t, double dt) { return dt; }
  /**
   * Finalize the physics module and deallocate buffer data.
   */
  void finalize();
  ///@}

  /**
   * @name I/O Methods
   */
  ///@{
  /**
   * Read from file at time t
   * @param t time
   */
  void input(double t);
  /**
   * Write restart files at time t.
   * @param t time
   */
  void output_restart_files(double t);
  /**
   * Write visualization files at time t.
   * @param t time
   */
  virtual void output_visualization_files(double t) = 0;
  /**
   * Read in files through control file. Returns 0 on success. Returns -1 if
   * control file not found.
   * @param t current time
   * @param base file name prefix, typically the material name
   * @param window name of window where SimIN will put all file data
   * @return 0 on success, -1 if control file not found.
   */
  int read_by_control_file(double t, const std::string &base,
                           const std::string &window);
  /**
   * Write a (possibly aggregate) attribute to file. If ref exists, then refer
   * to it for mesh data.
   * @param t current time
   * @param base file name prefix, typically the material name
   * @param attr name of (possibly aggregate) attribute
   * @param ref optional file to override for mesh
   */
  void write_data_files(double t, const std::string &base,
                        const std::string &attr, const char *ref = nullptr);
  /**
   * Write control file. Note: does not write associated data files, use with
   * write_data_files()
   * @param t current time
   * @param base file name prefix, typically a glob of the material names
   * @param window name of window SimOUT will write to file
   */
  void write_control_file(double t, const std::string &base,
                          const std::string &window);
  ///@}
  /**
   * Get a string that encodes the given time. If the string is xx.yyyyyy, it
   * means 0.yyyyyy*10^xx nanoseconds. This format ensures that sorting the
   * strings alphabetically gives the right ordering of time.
   * @param t time
   * @return time encoded as string
   */
  static std::string get_time_string(double t);

  /**
   * @name Action DataItem access
   * For creating DataItem by Action. Called from Action constructors.
   */
  ///@{
  /**
   * Register the creating of a new DataItem to Agent
   * @param target_window Target window
   * @param target_attr Target attribute name
   * @param loc COM location ('w', 'p', 'n', or 'e')
   * @param type COM type (e.g., COM_INTEGER, COM_DOUBLE, etc.)
   * @param ncomp number of components
   * @param unit name of units
   */
  void register_new_dataitem(std::string target_window, std::string target_attr,
                             char loc, int type, int ncomp, std::string unit);
  /**
   * Register the cloning of a DataItem to Agent
   * @param cond check existence of parent attribute before cloning
   * @param target_window Target window
   * @param target_attr Target attribute name
   * @param parent_window Parent window
   * @param parent_attr Parent attribute name
   * @param wg with ghosts (Default: 1)
   * @param ptnname Name of attribute to use as filter (Default: "")
   * @param val Value to pass to filter (Default: 0)
   */
  void register_clone_dataitem(bool cond, std::string target_window,
                               std::string target_attr,
                               std::string parent_window,
                               std::string parent_attr, int wg = 1,
                               std::string ptnname = "", int val = 0);
  /**
   * Register the use-inherit of a DataItem to Agent
   * @param target_window Target window
   * @param target_attr Target attribute name
   * @param parent_window Parent window
   * @param parent_attr Parent attribute name
   * @param wg with ghosts (Default: 1)
   * @param ptnname Name of attribute to use as filter (Default: "")
   * @param val Value to pass to filter (Default: 0)
   */
  void register_use_dataitem(std::string target_window, std::string target_attr,
                             std::string parent_window, std::string parent_attr,
                             int wg = 1, std::string ptnname = "", int val = 0);
  /**
   * Process all registered DataItem requests by Action on all windows. NOTE:
   * Calls COM_init_window_done() only on tmpBuf.
   * @param tmpBuf window name
   */
  void create_registered_dataitems(const std::string &tmpBuf);
  /**
   * Process all registered DataItem requests by Action on the specified
   * window. NOTE: Does NOT call COM_init_window_done().
   * @param target_window Target window
   */
  void create_registered_window_dataitems(const std::string &target_window);
  ///@}

  /**
   * @name Interpolation
   */
  ///@{
  /**
   * Register an Interpolate Action to the Agent. Used for backup.
   * @param ip
   */
  void register_interpolate(InterpolateBase *ip);
  /**
   * Return old time step. Used by interpolation Action.
   * @return old time step
   */
  double get_old_dt() const { return old_dt; }
  ///@}

  /**
   * Print to C-style stream for debugging.
   * @param f C stream
   */
  void print(FILE *f) const;

  /**
   * @name Predictor-corrector iteration methods
   */
  ///@{
  virtual void init_convergence(int iPredCorr);
  /**
   * Store (or restore if not converged) solutions
   * @param converged toggle if to store or restore
   */
  void store_solutions(bool converged);
  /**
   * Check convergence of PC iteration
   * @return true if converged
   */
  virtual bool check_convergence() const { return true; }
  /**
   * Check convergence helper to compute the ratio ||cur - pre|| / ||cur||
   * @param cur_hdl handle to current value
   * @param pre_hdl handle to previous value
   * @param tol tolerance to accept convergence
   * @param attr name of attribute
   * @return true if ratio is below tolerance
   */
  bool check_convergence_helper(int cur_hdl, int pre_hdl, double tol,
                                const std::string &attr) const;
  ///@}

  /**
   * @name Callback methods
   */
  ///@{
  /**
   * Initialization callback function used by physics module .initialize
   * @param surf_win Surface window name
   * @param vol_win Volume window name
   * @param option extra options
   */
  void init_callback(const char *surf_win, const char *vol_win,
                     void *option = nullptr);
  /**
   * Run boundary condition Scheduler from physics module .update_solution
   * @param alpha alpha time
   * @param level level
   */
  void obtain_bc(const double *alpha, const int *level = nullptr);
  /**
   * Run grid motion Scheduler from physics module .update_solution
   * @param alpha alpha time
   */
  void obtain_gm(const double *alpha);
  ///@}

  // TODO: Sort below methods
  virtual void read_restart_data() {}

  void assign_dataitems();
  void add_data() {}

protected:
  /**
   * Split surface window into different parts by .bcflag
   * @param surf_all All surface data.
   * @param surf_i Interacting surface window (bcflag = 0 or 1)
   * @param surf_nb Non-burning surface window (bcflag = 0)
   * @param surf_b Burning surface window (bcflag = 1)
   * @param surf_ni Non-interacting surface window (bcflag = 2)
   */
  static void split_surface_window(const std::string &surf_all,
                                   const std::string &surf_i,
                                   const std::string &surf_nb,
                                   const std::string &surf_b,
                                   const std::string &surf_ni);

protected:
  typedef std::vector<DataItemBase *> DataItemList;
  typedef std::map<int, Scheduler *> SchedulerMap;
  typedef std::vector<InterpolateBase *> InterpolateList;

  /**
   * @name Meta data
   */
  ///@{
  Coupling *coupling;           ///< Pointer to Coupling owning the Agent
  const std::string agent_name; ///< Agent name
  MPI_Comm communicator;        ///< MPI Communicator
  int comm_rank;                ///< MPI rank

  const std::string module_lname; ///< Physics module library name
  const std::string module_wname; ///< Physics module window name
  ///@}

  /**
   * @name Schedulers
   */
  ///@{
  Scheduler *icScheduler; ///< Initialization callback Scheduler
  Scheduler
      *bcInitScheduler; ///< Boundary condition initialization Scheduler. Note:
                        ///< This is init at the start of every time step.
  SchedulerMap
      bcSchedulers; ///< Boundary condition Scheduler with multi-level support
  Scheduler *gmScheduler; ///< Grid motion Scheduler
  ///@}

  /**
   * @name Callback Handles
   * Handles to routines provided by the Agent to the physics module
   */
  ///@{
  int ic_handle{-1}; ///< COM handle to init_callback()
  int bc_handle{-1}; ///< COM handle to obtain_bc()
  int gm_handle{-1}; ///< COM handle to obtain_gm()
  ///@}

  /**
   * @name Callback Data
   */
  ///@{
  double initial_time{0.0};      ///< Initial simulation time (UNUSED)
  double current_time{0.0};      ///< Current simulation time (init_callback)
  double current_deltatime{0.0}; ///< Current simulation time step
                                 ///< (init_callback, obtain_bc, obtain_gm)
  double timestamp{0.0}; ///< Temporary current simulation time (obtain_bc)
  double old_dt{0.0};    ///< Old simulation time step. Used for interpolation.
  bool with_gm{false};   ///< Set true if physics module requires gm_handle

  std::string surf_window; ///< Solver Surface window name (init_callback)
  std::string vol_window;  ///< Solver Volume window name (init_callback)
  ///@}

  /**
   * @name Physics Module Handles
   * Handles to routines provided by the physics module to the Agent
   */
  ///@{
  int init_handle{-1};     ///< COM handle to .initialize method
  int update_handle{-1};   ///< COM handle to .update_solution method
  int finalize_handle{-1}; ///< COM handle to .finalize method
  int pre_out_handle{-1};  ///< COM handle to .pre_out_output method (optional)
  int post_out_handle{-1}; ///< COM handle to .post_out_output method (optional)
  ///@}

  /**
   * @name Interpolation Data
   */
  ///@{
  InterpolateList interpolateList; ///< List of registered interpolation actions
  bool dobackup{false}; ///< do backup in interpolation (run_bcinitactions)
  ///@}

  /**
   * @name Predictor-corrector iteration data
   */
  ///@{
  std::vector<std::array<int, 2>>
      pc_hdls; ///< Handles for DataItems to be stored/restored for PC
               ///< iterations as pair: (live, backup)
  ///@}

  /**
   * @name I/O Handles and Data
   * SimIN and SimOUT function handles and window data.
   */
  ///@{
  int read_files_handle{-1};      ///< COM handle to IN.read_window
  int read_by_control_handle{-1}; ///< COM handle to IN.read_by_control_file
  int obtain_attr_handle{-1};     ///< COM handle to IN.obtain_dataitem
  int write_attr_handle{-1};      ///< COM handle to OUT.write_dataitem
  int write_ctrl_handle{-1}; ///< COM handle to OUT.write_rocin_control_file

  const std::string surf_name; ///< Material name assigned to the surface files
  const std::string vol_name;  ///< Material name assigned to the volume files

  const std::string surf_window_in; ///< Window passed to SimIN for surface data
  const std::string vol_window_in;  ///< Window passed to SimIN for volume data

  const std::string inDir;  ///< Directory to read input files.
  const std::string outDir; ///< Directory to write output files.
  ///@}

  /**
   * @name Buffer Windows
   */
  ///@{
  const std::string surf_all; ///< All surface data.
  const std::string surf_i;   ///< Interacting surface window (bcflag = 0 or 1)
  const std::string surf_nb;  ///< Non-burning surface window (bcflag = 0)
  const std::string surf_b;   ///< Burning surface window (bcflag = 1)
  const std::string surf_ni;  ///< Non-interacting surface window (bcflag = 2)
  ///@}

private:
  /**
   * Implements all COM_delete_window() calls.
   */
  virtual void finalize_windows() = 0;

  /**
   * Called by init_callback() to parse the optional data from solver
   * @param option_data passed from solver
   */
  virtual void parse_ic_options(void *option_data) {}
  /**
   * When surface input files are not found (e.g., initial start), fallback to
   * populating window here.
   * @param surface_window_in name of input window for surface mesh
   */
  virtual void input_fallback_surf(const std::string &surface_window_in) {}
  /**
   * When volume input files are not found (e.g., initial start), fallback to
   * populating window here.
   * @param volume_window_in name of input window for volume mesh
   */
  virtual void input_fallback_vol(const std::string &volume_window_in) {}
  /**
   * Create all buffers needed by Agent. Called during init_callback().
   */
  virtual void create_buffer_all() {}

private:
  /**
   * @name Module management
   */
  ///@{
  /**
   * Method for COM dynamic loading called in constructor.
   */
  void load_module();
  /**
   * Method to initialize the physics module.
   * @param t current time
   */
  void init_module(double t);
  /**
   * Method for COM dynamic unloading called in destructor.
   */
  void unload_module();
  ///@}

  /**
   * Initialize function handles for SimIO
   */
  void init_function_handles();
  /**
   * Initialize callback handles for init_callback(), obtain_bc(), and
   * obtain_gm()
   */
  void init_callback_handles();
  /**
   * Create internal surf_all buffer
   */
  void create_buffer_internal();

  /**
   * Run the BC init action scheduler. Called by PhysicsAction::run()
   * @param t current time
   * @param dt current time step
   */
  void run_bcinitactions(double t, double dt);

  // TODO: Remove these methods after confirming the Invoker Actions in Rocstar
  // are not necessary.
  void init_icactions(double t);
  void init_bcinitactions(double t);
  void init_bcactions(double t);
  void init_gmactions(double t);

private:
  PhysicsAction *physics_action; ///< Main physics module action
  DataItemList dataItemList;     ///< List of DataItems registered to Agent
};

#endif
