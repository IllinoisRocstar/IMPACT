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

#include "com.h"

#include "Action.h"
#include "Scheduler.h"

#define MAN_INTEG_SIZE 9

class Agent;
class Coupling;

class DataItemBase {
 public:
  Agent *agent;
  std::string target_window;
  std::string attr;

 public:
  DataItemBase(Agent *ag, std::string target_window_, std::string attr_)
      : agent(ag), target_window(target_window_), attr(attr_) {}
  virtual ~DataItemBase() {}
  virtual void create(std::string bufname);
  virtual void assign(std::string bufname);
};

class NewDataItem : public DataItemBase {
 protected:
  // new_dataitem
  char loc;
  int type;
  int ncomp;
  const char *unit;

 public:
  NewDataItem(Agent *ag, std::string target_window_, std::string attr_,
              char loc_, int type_, int ncomp_, const char *unit_);
  void create(std::string bufname);
};

// clone_dataitem
class CloneDataItem : public DataItemBase {
 protected:
  std::string parent_window;
  std::string parent_attr;
  int wg;
  const char *ptnname;
  int val;
  int condition;

 public:
  CloneDataItem(Agent *ag, int cond, std::string target_window_,
                std::string attr_, std::string parent_window_,
                std::string parent_attr_, int wg_ = 1, const char *ptnname_ = 0,
                int val_ = 0);
  void create(std::string bufname);
};

// use_dataitem
class UseDataItem : public DataItemBase {
 protected:
  std::string parent_window;
  std::string parent_attr;
  int wg;
  const char *ptnname;
  int val;

 public:
  UseDataItem(Agent *ag, std::string target_window_, std::string attr_,
              std::string parent_window_, std::string parent_attr_, int wg_ = 1,
              const char *ptnname_ = 0, int val_ = 0);
  void create(std::string bufname);
};

typedef std::vector<Scheduler *> SchedulerList;

class Agent : public COM_Object {
 protected:
  const std::string agent_name;
  MPI_Comm communicator;
  Coupling *coupling;  //

 private:
  Action action;
  SchedulerList bcScheduler;
  Scheduler icScheduler;
  Scheduler bcInitScheduler;
  Scheduler gmScheduler;

 protected:
  // SimIN and SimOUT function handles
  int read_by_control_handle;
  int read_files_handle;
  int obtain_attr_handle;
  int write_attr_handle;
  int write_ctrl_handle;

  // Handles to routines provided by physics modules
  int comm_rank;
  int init_handle, update_handle, finalize_handle;
  int pre_hdf_handle, post_hdf_handle;

  double initial_time, timestamp, current_time, current_deltatime, old_dt;

  // surf_window can be solidWin of INITIALIZE_SOLID() in SolidAgent ,
  //  or fluidSurf of INITIALIZE_FLUID() in FluidAgent
  std::string surf_window, vol_window;
  std::string tmp_window;

  void *option_data;

  typedef std::vector<DataItemBase *> DataItemList;
  DataItemList dataitemList;

 public:
  Agent(Coupling *cp, const char *agentname, MPI_Comm com)
      : agent_name(agentname), communicator(com), coupling(cp), action(this) {}
  virtual ~Agent() {}

  // Methods for COM dynamic loading.
  virtual void load_module() {}
  virtual void unload_module() {}
  virtual void init_module(double t, double dt);

  //
  void callMethod(Scheduler_voidfn1_t fn, double t);
  void schedule();

  //
  virtual void finalize();

  // I/O Methods
  virtual void input(double t) = 0;
  virtual void output_restart_files(double t) = 0;
  virtual void output_visualization_files(double t) = 0;

  // Getters for COM windows.
  std::string get_surface_window() const { return surf_window; }
  std::string get_volume_window() const { return vol_window; }

  double max_timestep(double t, double dt);

  // Getter for the update solution method of the physics library.
  virtual Action *get_main_action() { return &action; }

  // For PC iteration
  virtual int check_convergence(double tol = 0) { return 1; };

  void print(FILE *f);

  // for creating dataitems by Actions
  void register_new_dataitem(std::string target_window_, std::string attr_,
                             char loc_, int type_, int ncomp_,
                             const char *unit_);
  void register_clone_dataitem(int cond, std::string target_window_,
                               std::string attr_, std::string parent_window_,
                               std::string parent_attr_, int wg_ = 1,
                               const char *ptnname_ = 0, int val_ = 0);
  void register_use_dataitem(std::string target_window_, std::string attr_,
                             std::string parent_window_,
                             std::string parent_attr_, int wg_ = 1,
                             const char *ptnname_ = 0, int val_ = 0);

  void create_registered_dataitems(std::string tmpBuf);
  void create_registered_window_dataitems(std::string target_window);
  virtual void create_buffer_all();
  void assign_dataitems();

  virtual void read_restart_data() {}

  void add_data() { /* TODO */
  }

  // Getters for various fields.
  std::string get_agent_name() const { return agent_name; }
  virtual Coupling *get_coupling() { return coupling; }
  MPI_Comm get_communicator() const { return communicator; }
  int get_comm_rank() const { return comm_rank; }

  virtual size_t get_bcScheduler_size() { return bcScheduler.size(); }
  virtual Scheduler *get_bcScheduler(int i) { return bcScheduler[i]; }
  virtual Scheduler &get_icScheduler() { return icScheduler; }
  virtual Scheduler &get_bcInitScheduler() { return bcInitScheduler; }
  virtual Scheduler &get_gmScheduler() { return gmScheduler; }

  // For PC iterations
  double get_old_dt() const { return old_dt; }
};

#endif
