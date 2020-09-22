//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** \file Agent.C
 *  * Contains the base implementation of Agent. Each agent represent
 *  * one instance of a physics module (e.g. rocflo, rocflu, rocsolid or
 * rocfrac)
 *       */

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <utility>

#include "Agent.h"
#include "Coupling.h"
#include "Interpolate.h"
#include "RocBlas-SIM.h"
#include <array>

// TODO: a new debug verbosity macro will be implemented to
//       replace these local macros
#define MAN_DEBUG(l, x) ((void)0)

void DataItemBase::assign(const std::string &bufname) {
  // check if target_window exists
  if (COM_get_window_handle(target_window) <= 0) {
    /* Opt for abort failure instead of silently creating the window.
    COM_new_window(target_window);
    COM_use_dataitem(target_window, bufname + ".all");
    */
    COM_abort_msg(EXIT_FAILURE, "IMPACT ERROR: assign error.");
  }

  COM_use_dataitem(target_window, bufname + attr);
  // when to call init_done ??????????????
}

NewDataItem::NewDataItem(Agent *ag, std::string target_window_,
                         std::string attr_, char loc_, int type_, int ncomp_,
                         std::string unit_)
    : DataItemBase(ag, std::move(target_window_), std::move(attr_)), loc(loc_),
      type(type_), ncomp(ncomp_), unit(std::move(unit_)) {}

void NewDataItem::create(const std::string &bufname) {
  // test if already created
  if (COM_get_dataitem_handle(bufname + attr) > 0) {
    char loc_;
    int type_;
    int ncomp_;
    std::string unit_;
    COM_get_dataitem(bufname + attr, &loc_, &type_, &ncomp_, &unit_);
    if (loc_ != loc || type_ != type || ncomp_ != ncomp || unit_ != unit) {
      COM_abort_msg(EXIT_FAILURE,
                    "IMPACT ERROR: NewDataItem::create(): Could not create " +
                        bufname + attr + " in two different ways ");
    } else {
      return;
    }
  }
  MAN_DEBUG(2, ("NewDataItem::create: %s%s.\n", bufname.c_str(), attr.c_str()));
  COM_new_dataitem(bufname + attr, loc, type, ncomp, unit);
  COM_resize_array(bufname + attr);
}

CloneDataItem::CloneDataItem(Agent *ag, bool cond, std::string target_window_,
                             std::string attr_, std::string parent_window_,
                             std::string parent_attr_, int wg_,
                             std::string ptnname_, int val_)
    : DataItemBase(ag, std::move(target_window_), std::move(attr_)),
      condition(cond), parent_window(std::move(parent_window_)),
      parent_attr(std::move(parent_attr_)), wg(wg_),
      ptnname(std::move(ptnname_)), val(val_) {}

void CloneDataItem::create(const std::string &bufname) {
  if (parent_window.empty()) // filling
    parent_window = agent->get_surf_win();

  if (COM_get_dataitem_handle(bufname + attr) > 0) {
    char loc1, loc2;
    int type1, type2;
    int ncomp1, ncomp2;
    std::string unit1, unit2;
    COM_get_dataitem(bufname + attr, &loc1, &type1, &ncomp1, &unit1);
    COM_get_dataitem(parent_window + parent_attr, &loc2, &type2, &ncomp2,
                     &unit2);
    if (loc1 != loc2 || type1 != type2 || ncomp1 != ncomp2 || unit1 != unit2) {
      COM_abort_msg(EXIT_FAILURE,
                    "IMPACT ERROR: CloneDataItem::create(): Could not create " +
                        bufname + attr + " in two different ways ");
    } else {
      MAN_DEBUG(
          2,
          ("CloneDataItem::create: %s%s %s%s condition:%d handle:%d SKIPPED.\n",
           bufname.c_str(), attr.c_str(), parent_window.c_str(),
           parent_attr.c_str(), condition,
           COM_get_dataitem_handle(parent_window + parent_attr)));
      return;
    }
  }
  MAN_DEBUG(2, ("CloneDataItem::create: %s%s %s%s condition:%d handle:%d.\n",
                bufname.c_str(), attr.c_str(), parent_window.c_str(),
                parent_attr.c_str(), condition,
                COM_get_dataitem_handle(parent_window + parent_attr)));

  if (condition)
    if (COM_get_dataitem_handle(parent_window + parent_attr) <= 0)
      return;

  if (!ptnname.empty())
    if (ptnname[0] == '.')
      ptnname = agent->get_surf_win() + ptnname;

  COM_clone_dataitem(bufname + attr, parent_window + parent_attr, wg, ptnname,
                     val);
}

UseDataItem::UseDataItem(Agent *ag, std::string target_window_,
                         std::string attr_, std::string parent_window_,
                         std::string parent_attr_, int wg_,
                         std::string ptnname_, int val_)
    : DataItemBase(ag, std::move(target_window_), std::move(attr_)),
      parent_window(std::move(parent_window_)),
      parent_attr(std::move(parent_attr_)), wg(wg_),
      ptnname(std::move(ptnname_)), val(val_) {}

void UseDataItem::create(const std::string &bufname) {
  if (parent_window.empty())
    parent_window = agent->get_surf_win();

  if (COM_get_dataitem_handle(bufname + attr) > 0) {
    char loc1, loc2;
    int type1, type2;
    int ncomp1, ncomp2;
    std::string unit1, unit2;
    COM_get_dataitem(bufname + attr, &loc1, &type1, &ncomp1, &unit1);
    COM_get_dataitem(parent_window + parent_attr, &loc2, &type2, &ncomp2,
                     &unit2);
    if (loc1 != loc2 || type1 != type2 || ncomp1 != ncomp2 || unit1 != unit2)
      COM_abort_msg(EXIT_FAILURE,
                    "IMPACT ERROR: NewDataItem::create(): Could not create " +
                        bufname + attr + " in two different ways ");
    else
      return;
  }
  MAN_DEBUG(2, ("UseDataItem::create: %s%s %s%s.\n", bufname.c_str(),
                attr.c_str(), parent_window.c_str(), parent_attr.c_str()));

  COM_use_dataitem(bufname + attr, parent_window + parent_attr, wg, ptnname,
                   val);
}

PhysicsAction::PhysicsAction(Agent *ag)
    : Action(ag->get_agent_name()), agent(ag) {}

void PhysicsAction::run(double t, double dt, double alpha) {
  MAN_DEBUG(
      1,
      ("[%d] Rocman: Agent %s::PhysicsAction run with t:%e dt:%e alpha:%e.\n",
       agent->comm_rank, agent->get_agent_name().c_str(), t, dt, alpha));

  agent->current_deltatime = dt; // Needed by bc and gm callback
  agent->timestamp = t;          // Needed by bc callback

  if (agent->get_coupling()->get_ipc() == 1)
    agent->old_dt = dt; // Needed by interpolation actions on the bc scheduler

  // include Interpolate::backup
  agent->run_bcinitactions(t, dt);

  if (agent->with_gm)
    COM_call_function(agent->update_handle, &t, &dt, &agent->bc_handle,
                      &agent->gm_handle);
  else
    COM_call_function(agent->update_handle, &t, &dt, &agent->bc_handle);

  agent->current_time = t;
}

void PhysicsAction::print(FILE *f) const { agent->print(f); }

Agent::Agent(Coupling *cp, std::string agentname, MPI_Comm com,
             std::string modulelname, std::string modulewname,
             std::string surfname, std::string volname)
    : coupling(cp), agent_name(std::move(agentname)), communicator(com),
      comm_rank(COMMPI_Comm_rank(com)), module_lname(std::move(modulelname)),
      module_wname(std::move(modulewname)), surf_name(std::move(surfname)),
      vol_name(std::move(volname)), surf_window_in(surf_name + "IN"),
      vol_window_in(vol_name + "IN"), inDir(module_wname + "/Rocin/"),
      outDir(module_wname + "/Rocout/"), surf_all(surf_name + "_all"),
      surf_i(surf_name + "_i"), surf_nb(surf_name + "_nb"),
      surf_b(surf_name + "_b"), surf_ni(surf_name + "_ni") {
  icScheduler = new UserScheduler();
  icScheduler->set_name(agent_name + "-ic");
  bcInitScheduler = new UserScheduler();
  bcInitScheduler->set_name(agent_name + "-bcInit");
  // Note: bcSchedulers are initialized when first used in add_bcaction()
  gmScheduler = new UserScheduler();
  gmScheduler->set_name(agent_name + "-gm");
  physics_action = new PhysicsAction(this);
  physics_action->set_name(agent_name + "-PhysicsAction");

  load_module();
}

Agent::~Agent() {
  delete icScheduler;
  delete bcInitScheduler;
  for (auto &&bcScheduler : bcSchedulers)
    delete bcScheduler.second;
  delete gmScheduler;
  for (auto &&dataItem : dataItemList)
    delete dataItem;
  delete physics_action;

  unload_module();
}

void Agent::register_new_dataitem(std::string target_window,
                                  std::string target_attr, char loc, int type,
                                  int ncomp, std::string unit) {
  auto *newAttr =
      new NewDataItem(this, std::move(target_window), std::move(target_attr),
                      loc, type, ncomp, std::move(unit));
  dataItemList.push_back(newAttr);
}

void Agent::register_clone_dataitem(bool cond, std::string target_window,
                                    std::string target_attr,
                                    std::string parent_window,
                                    std::string parent_attr, int wg,
                                    std::string ptnname, int val) {
  auto *newAttr =
      new CloneDataItem(this, cond, std::move(target_window),
                        std::move(target_attr), std::move(parent_window),
                        std::move(parent_attr), wg, std::move(ptnname), val);
  dataItemList.push_back(newAttr);
}

void Agent::register_use_dataitem(std::string target_window,
                                  std::string target_attr,
                                  std::string parent_window,
                                  std::string parent_attr, int wg,
                                  std::string ptnname, int val) {
  auto *newAttr =
      new UseDataItem(this, std::move(target_window), std::move(target_attr),
                      std::move(parent_window), std::move(parent_attr), wg,
                      std::move(ptnname), val);
  dataItemList.push_back(newAttr);
}

void Agent::create_registered_dataitems(const std::string &tmpBuf) {
  for (auto &&dataItem : dataItemList) {
    MAN_DEBUG(2, ("[%d] creating %s %s\n", comm_rank,
                  dataItem->target_window.c_str(), dataItem->attr.c_str()));
    dataItem->create(dataItem->target_window);
  }
  COM_window_init_done(tmpBuf);
}

void Agent::create_registered_window_dataitems(
    const std::string &target_window) {
  for (auto &&dataItem : dataItemList) {
    if (dataItem->target_window == target_window) {
      MAN_DEBUG(2, ("[%d] creating %s %s\n", comm_rank,
                    dataItem->target_window.c_str(), dataItem->attr.c_str()));
      dataItem->create(dataItem->target_window);
    }
  }
}

void Agent::create_buffer_internal() {
  MAN_DEBUG(1, ("[%d] Rocman: Agent %s::create_buffer_internal called %s.\n",
                comm_rank, get_agent_name().c_str(), surf_all.c_str()));
  COM_new_window(surf_all);
  /* Use everything except pconn
  COM_use_dataitem(surf_all, surf_window + ".all", 1);
  */
  COM_use_dataitem(surf_all, surf_window + ".mesh", 1);
  COM_use_dataitem(surf_all, surf_window + ".data", 1);
  // create_registered_dataitems(surf_all);
  create_registered_window_dataitems(surf_all);
  COM_window_init_done(surf_all);

  // Split surface windows
  if (has_bcflag)
    split_surface_window(surf_all, surf_i, surf_nb, surf_b, surf_ni);
}

void Agent::assign_dataitems() {
  for (auto &&dataItem : dataItemList)
    dataItem->assign(surf_all);
}

void Agent::load_module() {
#ifndef STATIC_LINK // dynamic loading
  MAN_DEBUG(3, ("[%d] Rocstar: Agent::load_module %s %s.\n", comm_rank,
                module_lname.c_str(), module_wname.c_str()));

  COM_load_module(module_lname, module_wname);
#endif
}

void Agent::unload_module() {
#ifndef STATIC_LINK // dynamic loading
  MAN_DEBUG(3, ("[%d] Rocstar: Agent::unload_module %s %s.\n", comm_rank,
                module_lname.c_str(), module_wname.c_str()));

  // in restarting, close_module does not dlclose the shared lib
  if (get_coupling()->is_restart())
    COM_close_module(module_lname, module_wname);
  else
    COM_unload_module(module_lname, module_wname);
#endif
}

void Agent::init(double t, double dt) {
  MAN_DEBUG(1, ("[%d] Rocman: %s::init with current_time:%e "
                "current_deltatime:%e.\n",
                comm_rank, get_agent_name().c_str(), t, dt));

  initial_time = t;
  current_time = t;
  current_deltatime = dt;
  old_dt = 0.0;

  init_callback_handles();
  init_function_handles();

  input(t);

  init_module(t);
}

void Agent::init_function_handles() {
  init_handle = COM_get_function_handle(module_wname + ".initialize");
  update_handle = COM_get_function_handle(module_wname + ".update_solution");
  finalize_handle = COM_get_function_handle(module_wname + ".finalize");

  COM_assertion_msg(init_handle > 0, "ERROR: Agent::init_function_handles.\n");
  COM_assertion_msg(update_handle > 0,
                    "ERROR: Agent::init_function_handles.\n");
  COM_assertion_msg(finalize_handle > 0,
                    "ERROR: Agent::init_function_handles.\n");

  // optional
  pre_out_handle = COM_get_function_handle(module_wname + ".pre_out_output");
  post_out_handle = COM_get_function_handle(module_wname + ".post_out_output");

  COM_set_profiling_barrier(init_handle, communicator);
  COM_set_profiling_barrier(update_handle, communicator);
  COM_set_profiling_barrier(finalize_handle, communicator);
  if (pre_out_handle != -1)
    COM_set_profiling_barrier(pre_out_handle, communicator);
  if (post_out_handle != -1)
    COM_set_profiling_barrier(post_out_handle, communicator);

  /*
  compute_integrals_handle =
      COM_get_function_handle(module_wname + ".compute_integrals");
  */

  read_files_handle = COM_get_function_handle("IN.read_window");
  read_by_control_handle = COM_get_function_handle("IN.read_by_control_file");
  obtain_attr_handle = COM_get_function_handle("IN.obtain_dataitem");
  write_attr_handle = COM_get_function_handle("OUT.write_dataitem");
  write_ctrl_handle = COM_get_function_handle("OUT.write_rocin_control_file");

  COM_assertion_msg(read_files_handle > 0,
                    "ERROR: Agent::init_function_handles.\n");
  COM_assertion_msg(read_by_control_handle > 0,
                    "ERROR: Agent::init_function_handles.\n");
  COM_assertion_msg(obtain_attr_handle > 0,
                    "ERROR: Agent::init_function_handles.\n");
  COM_assertion_msg(write_attr_handle > 0,
                    "ERROR: Agent::init_function_handles.\n");
  COM_assertion_msg(write_ctrl_handle > 0,
                    "ERROR: Agent::init_function_handles.\n");

  COM_set_profiling_barrier(read_files_handle, communicator);
  COM_set_profiling_barrier(read_by_control_handle, communicator);
  COM_set_profiling_barrier(obtain_attr_handle, communicator);
  COM_set_profiling_barrier(write_attr_handle, communicator);
  COM_set_profiling_barrier(write_ctrl_handle, communicator);
}

void Agent::add_icaction(Action *act) { icScheduler->add_action(act); }

void Agent::add_bcinitaction(Action *act) { bcInitScheduler->add_action(act); }

void Agent::add_bcaction(Action *act, int l) {
  if (bcSchedulers.count(l) == 0) {
    bcSchedulers[l] = new UserScheduler();
    bcSchedulers[l]->set_name(agent_name + "-bc" + std::to_string(l));
  }

  bcSchedulers[l]->add_action(act);
}

void Agent::init_callback_handles() {
  // create window and global variables
  std::string global = agent_name + ".global";

  COM_new_window(agent_name);
  COM_new_dataitem(global, 'w', COM_VOID, 1, "");
  COM_set_object(global, 0, this);

  COM_set_member_function(
      agent_name + ".init_callback", (Member_func_ptr)&Agent::init_callback,
      global, "biiI", {COM_RAWDATA, COM_STRING, COM_STRING, COM_VOID});
  ic_handle = COM_get_function_handle(agent_name + ".init_callback");

  COM_set_member_function(agent_name + ".obtain_bc",
                          (Member_func_ptr)&Agent::obtain_bc, global, "biI",
                          {COM_RAWDATA, COM_DOUBLE, COM_INT});
  bc_handle = COM_get_function_handle(agent_name + ".obtain_bc");

  COM_set_member_function(agent_name + ".obtain_gm",
                          (Member_func_ptr)&Agent::obtain_gm, global, "bi",
                          {COM_RAWDATA, COM_DOUBLE});
  gm_handle = COM_get_function_handle(agent_name + ".obtain_gm");

  COM_window_init_done(agent_name);
}

void Agent::add_gmaction(Action *act) { gmScheduler->add_action(act); }

void Agent::callMethod(Scheduler_voidfn1_t fn, double t) {
  (bcInitScheduler->*fn)(t);
  for (auto &&bcScheduler : bcSchedulers)
    (bcScheduler.second->*fn)(t);
  (gmScheduler->*fn)(t);
}

void Agent::schedule() {
  icScheduler->schedule();
  bcInitScheduler->schedule();
  for (auto &&bcScheduler : bcSchedulers)
    bcScheduler.second->schedule();
  gmScheduler->schedule();
}

void Agent::finalize() {
  MAN_DEBUG(1, ("[%d] Rocman: %s::finalize called.\n", comm_rank,
                get_agent_name().c_str()));

  icScheduler->finalize_actions();
  bcInitScheduler->finalize_actions();
  for (auto &&bcScheduler : bcSchedulers)
    bcScheduler.second->finalize_actions();
  gmScheduler->finalize_actions();

  // at restart, we don't want windows in physics modules deleted
  if (!get_coupling()->is_restart())
    COM_call_function(finalize_handle);

  finalize_windows(); // pure virtual

  COM_delete_window(surf_all);
  COM_delete_window(agent_name);
}

void Agent::split_surface_window(const std::string &surf_all,
                                 const std::string &surf_i,
                                 const std::string &surf_nb,
                                 const std::string &surf_b,
                                 const std::string &surf_ni) {
  std::string bcflag = surf_all + ".bcflag";
  std::string all = surf_all + ".all";

  COM_new_window(surf_i); // bcflag == 0 or bcflag == 1
  COM_use_dataitem(surf_i, all, 1, bcflag, 0);
  COM_use_dataitem(surf_i, all, 1, bcflag, 1);
  COM_window_init_done(surf_i);

  COM_new_window(surf_nb); // bcflag == 0
  COM_use_dataitem(surf_nb, all, 1, bcflag, 0);
  COM_window_init_done(surf_nb);

  COM_new_window(surf_b); // bcflag == 1
  COM_use_dataitem(surf_b, all, 1, bcflag, 1);
  COM_window_init_done(surf_b);

  COM_new_window(surf_ni); // bcflag == 2
  COM_use_dataitem(surf_ni, all, 1, bcflag, 2);
  COM_window_init_done(surf_ni);
}

/*
void Agent::run_initactions(double t, double dt)
{
  MAN_DEBUG(1, ("Rocman: %s::run_initactions called with t:%e dt:%e.\n",
get_agent_name().c_str(), t, dt)); initScheduler.set_alpha(0.0);
  //initScheduler.init_actions(t);
  initScheduler.run_actions(t, dt);
}
*/

void Agent::init_module(double t) {
  MAN_DEBUG(3, ("Rocstar: Agent::init_module t=%e.\n", t));

  // Call initialization routine of physics module
  // rocman.f90:INITIALIZE()
  COM_call_function(init_handle, &t, &communicator, &ic_handle,
                    surf_window_in.c_str(), vol_window_in.c_str(),
                    &obtain_attr_handle);

  // Delete input buffer windows
  COM_delete_window(surf_window_in);
  COM_delete_window(vol_window_in);
}

void Agent::input(double t) {
  if (read_by_control_file(t, surf_name, surf_window_in) == -1) {
    COM_new_window(surf_window_in);
    input_fallback_surf(surf_window_in); // virtual
    COM_window_init_done(surf_window_in);
  }
  if (read_by_control_file(t, vol_name, vol_window_in) == -1) {
    COM_new_window(vol_window_in);
    input_fallback_vol(vol_window_in); // virtual
    COM_window_init_done(vol_window_in);
  }
}

void Agent::output_restart_files(double t) {
  if (pre_out_handle != -1)
    COM_call_function(pre_out_handle);

  // Write out surface sub-windows
  if (has_bcflag) {
    write_data_files(t, surf_b, surf_b + ".all");
    write_data_files(t, surf_nb, surf_nb + ".all");
    write_data_files(t, surf_ni, surf_ni + ".all");
  } else {
    write_data_files(t, surf_all, surf_all + ".all");
  }
  write_control_file(t, surf_name + "*", surf_window);

  // Write out volume window
  write_data_files(t, vol_name, vol_window + ".all");
  write_control_file(t, vol_name, vol_window);

  if (post_out_handle != -1)
    COM_call_function(post_out_handle);
}

std::string Agent::get_time_string(double t) {
  char chrstring[15];

  std::sprintf(chrstring, "%.5e", t * 1.e10);

  std::string s;
  s.append(&chrstring[9]);
  s.append(".");
  s.append(&chrstring[0], 1);
  s.append(&chrstring[2], 5);

  return s;
}

void Agent::write_data_files(double t, const std::string &base,
                             const std::string &attr, const char *ref) {
  std::string fname_prfx = outDir + base + "_" + get_time_string(t) + "_";

  MAN_DEBUG(
      2,
      ("[%d] Rocman: Agent %s::write_data_file with file prefix %s at t:%e.\n",
       comm_rank, get_agent_name().c_str(), fname_prfx.c_str(), t));

  int attr_hdl = COM_get_dataitem_handle(attr);
  std::string timeLevel = get_time_string(t);
  COM_call_function(write_attr_handle, fname_prfx.c_str(), &attr_hdl,
                    base.c_str(), timeLevel.c_str(), ref, &communicator);
}

void Agent::write_control_file(double t, const std::string &base,
                               const std::string &window) {
  std::string fname_prfx = base + "_" + get_time_string(t) + "_";
  std::string ctrl_fname = outDir + base + "_in_" + get_time_string(t) + ".txt";

  // If base contains '*', then remove it from ctrl_fname
  ctrl_fname.erase(std::remove(ctrl_fname.begin(), ctrl_fname.end(), '*'),
                   ctrl_fname.end());

  MAN_DEBUG(1, ("[%d] Rocman: Agent %s::write_control_file %s with file "
                "prefix %s at t:%e.\n",
                comm_rank, get_agent_name().c_str(), ctrl_fname.c_str(),
                fname_prfx.c_str(), t));

  COM_call_function(write_ctrl_handle, window.c_str(), fname_prfx.c_str(),
                    ctrl_fname.c_str());
}

int Agent::read_by_control_file(double t, const std::string &base,
                                const std::string &window) {
  std::string ctrl_fname = inDir + base + "_in_" + get_time_string(t) + ".txt";

  MAN_DEBUG(2, ("[%d] Agent %s::read_by_control_file run with file: %s t:%e.\n",
                comm_rank, get_agent_name().c_str(), ctrl_fname.c_str(), t));

  FILE *fp = fopen(ctrl_fname.c_str(), "r");

  // If not time zero (e.g., a restart), check outDir for control files.
  if (fp == nullptr && t != 0.0) {
    ctrl_fname = outDir + base + "_in_" + get_time_string(t) + ".txt";
    fp = fopen(ctrl_fname.c_str(), "r");
  }

  if (fp != nullptr) {
    if (comm_rank == 0)
      std::cout << "IMPACT: Found control file " << ctrl_fname << std::endl;

    COM_call_function(read_by_control_handle, ctrl_fname.c_str(),
                      window.c_str(), &communicator);
    return 0;
  } else {
    if (comm_rank == 0)
      std::cout << "IMPACT: Did not find control file " << ctrl_fname
                << std::endl;

    return -1;
  }
}

void Agent::print(FILE *f) const {
  // agent
  fprintf(f, "graph: { title: \"%s\" label: \"%s\" \n\
        status: folded \n\
        display_edge_labels: yes \n\
        layoutalgorithm: tree   \n\
        scaling: maxspect   \n\
        color :  lightyellow           \n\
        node.color     : lightblue   \n\
        node.textcolor : lightblue   \n\
        node.bordercolor: black \n\
        node.borderwidth: 1    \n\
        edge.color     : lightblue   \n\
        edge.arrowsize : 7   \n\
        edge.thickness : 2   \n\
        edge.fontname:\"helvO08\"  \n\
        node.label: \"no type\" \n",
          agent_name.c_str(), agent_name.c_str());

  char *bcinit = bcInitScheduler->print(f, agent_name.c_str());

  // main physics
  std::string main_action = agent_name + "-main";
  fprintf(f, "graph: { title: \"%s\" label: \"%s\" \n\
        status: folded \n\
        display_edge_labels: yes \n\
        layoutalgorithm: tree   \n\
        scaling: maxspect   \n\
        color :  lightred           \n\
        node.color     : lightblue   \n\
        node.textcolor : black   \n\
        node.bordercolor: black \n\
        node.borderwidth: 1    \n\
        edge.color     : lightblue   \n\
        edge.arrowsize : 7   \n\
        edge.thickness : 2   \n\
        edge.fontname:\"helvO08\"  \n\
        node.label: \"no type\" \n",
          main_action.c_str(), main_action.c_str());

  for (const auto &bcScheduler : bcSchedulers)
    bcScheduler.second->print(f, agent_name.c_str());

  gmScheduler->print(f, agent_name.c_str());

  std::string main_physics = agent_name + "-physics";
  fprintf(f, "node: { title:\"%s\" label:\"%s\"}\n", main_physics.c_str(),
          main_physics.c_str());

  fprintf(f, "}\n");

  if (bcinit) {
    fprintf(f, "edge: { sourcename: \"%s\" targetname: \"%s\" label: \"\"}\n",
            bcinit, main_action.c_str());
    free(bcinit);
  }

  fprintf(f, "}\n");
}

void Agent::init_convergence(int iPredCorr) { store_solutions(iPredCorr == 1); }

void Agent::store_solutions(bool converged) {
  MAN_DEBUG(1, ("[%d] Agent %s::store_solutions converged=%d.\n", comm_rank,
                get_agent_name().c_str(), converged));

  if (converged) { //  Store internal data
    for (const auto &pc_hdl : pc_hdls)
      if (pc_hdl[0] > 0)
        COM_copy_dataitem(pc_hdl[1], pc_hdl[0]);
  } else {
    for (const auto &pc_hdl : pc_hdls)
      if (pc_hdl[1] > 0)
        COM_copy_dataitem(pc_hdl[0], pc_hdl[1]);
  }
}

bool Agent::check_convergence_helper(int cur_hdl, int pre_hdl, double tol,
                                     const std::string &attr) const {
  double nrm_diff = 0.0;
  double nrm_val = 0.0;

  COM_call_function(RocBlas::sub, &cur_hdl, &pre_hdl, &pre_hdl);
  COM_call_function(RocBlas::nrm2_scalar_MPI, &cur_hdl, &nrm_val,
                    &communicator);
  COM_call_function(RocBlas::nrm2_scalar_MPI, &pre_hdl, &nrm_diff,
                    &communicator);

  double ratio = nrm_diff;
  if (nrm_val != 0.0)
    ratio /= nrm_val;

  if (comm_rank == 0)
    std::cout << "Rocstar: Convergence ratio of " << attr << " is " << ratio
              << std::endl;

  return ratio <= tol;
}

void Agent::init_callback(const char *surf_win, const char *vol_win,
                          void *option) {
  MAN_DEBUG(1, ("Rocman: %s::init_callback called: surfwin: %s volwin: %s.\n",
                get_agent_name().c_str(), surf_win, vol_win));

  surf_window = surf_win;
  vol_window = vol_win;

  if (COM_get_dataitem_handle(surf_window + ".bcflag") > 0)
    has_bcflag = true;

  // create a window buffer to inherit from surface window
  // they are like ifluid_all, iburn_all, etc
  create_buffer_internal();

  // Let derived classes create their own buffers
  create_buffer_all(); // virtual call

  icScheduler->init_actions(current_time);
  if (icScheduler->isEmpty()) {
    if (comm_rank == 0)
      std::cerr << "IMPACT WARNING: No actions defined for "
                << "Initialization callback for " << agent_name << " module."
                << std::endl;
  } else {
    MAN_DEBUG(1, ("[%d] Rocman: %s::init_callback called IC with "
                  "current_time:%e current_deltatime:%e.\n",
                  comm_rank, get_agent_name().c_str(), current_time,
                  current_deltatime));
    icScheduler->run_actions(current_time, current_deltatime, 0.0);
    MAN_DEBUG(1, ("[%d] Rocman: %s::init_callback called IC DONE.\n", comm_rank,
                  get_agent_name().c_str()));
  }

  parse_ic_options(option); // virtual call

  MAN_DEBUG(
      1, ("Rocman: Agent::init_callback called: surfwin: %s volwin: %s DONE.\n",
          surf_win, vol_win));
}

void Agent::init_icactions(double t) { icScheduler->init_actions(t); }

void Agent::init_bcinitactions(double t) { bcInitScheduler->init_actions(t); }

void Agent::init_bcactions(double t) {
  for (auto &&bcScheduler : bcSchedulers)
    bcScheduler.second->init_actions(t);
}

void Agent::init_gmactions(double t) { gmScheduler->init_actions(t); }

void Agent::obtain_bc(const double *alpha, const int *level) {
  int l = (level == nullptr) ? 1 : *level;
  if (bcSchedulers.count(l) == 0) {
    if (comm_rank == 0)
      std::cerr << "IMPACT WARNING: No actions defined for "
                << "level-" << l << " boundary condition for " << agent_name
                << " module." << std::endl;
    return;
  }

  MAN_DEBUG(2, ("[%d] Rocman: Agent %s::obtain_bc called at level: %d t:%e.\n",
                comm_rank, get_agent_name().c_str(), l, timestamp));

  bcSchedulers[l]->run_actions(timestamp, current_deltatime, *alpha);
}

void Agent::obtain_gm(const double *alpha) {
  if (gmScheduler->isEmpty()) {
    if (comm_rank == 0)
      std::cerr << "IMPACT WARNING: No actions defined for "
                << "Grid Motion for " << agent_name << " module." << std::endl;
    return;
  }
  MAN_DEBUG(2, ("[%d] Rocman: Agent::obtain_gm called with alpha: %e.\n",
                comm_rank, *alpha));
  // ???????????????
  gmScheduler->run_actions(0.0, current_deltatime, *alpha);
}

void Agent::run_bcinitactions(double t, double dt) {
  MAN_DEBUG(1, ("Rocman: %s::run_bcinitaction called t=%e dt=%e.\n",
                get_agent_name().c_str(), t, dt));

  // backup first
  if (dobackup && get_coupling()->get_ipc() <= 1)
    for (auto &&i : interpolateList)
      i->backup();

  bcInitScheduler->run_actions(t, dt, -1.0);
}

void Agent::register_interpolate(InterpolateBase *ip) {
  /* AEG: FluidAgent forcibly does not backup when without a solid yet uses
   * interpolation with BurnAgent! *//*
  dobackup = true;
  */
  interpolateList.push_back(ip);
}
