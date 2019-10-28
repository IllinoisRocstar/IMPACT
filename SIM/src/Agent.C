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

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Agent.h"
#include "Coupling.h"

#define MAN_DEBUG(l, x)

void DataItemBase::create(std::string bufname) {}

void DataItemBase::assign(std::string bufname) {
  // check if target_window is created
  //  if (COM_get_window_handle( target_window) <= 0) {
  //    COM_new_window( target_window);
  //    COM_use_dataitem( target_window, bufname+".all");
  //  }
  COM_assertion_msg(COM_get_window_handle(target_window) > 0,
                    "ERROR: assign error.\n");

  COM_use_dataitem(target_window, bufname + attr);
  // when to call init_done ??????????????
}

NewDataItem::NewDataItem(Agent *ag, std::string target_window_,
                         std::string attr_, char loc_, int type_, int ncomp_,
                         const char *unit_)
    : DataItemBase(ag, target_window_, attr_),
      loc(loc_),
      type(type_),
      ncomp(ncomp_),
      unit(unit_) {}

void NewDataItem::create(std::string bufname) {
  // test if already created
  if (COM_get_dataitem_handle(bufname + attr) > 0) {
    char loc_;
    int type_;
    int ncomp_;
    std::string unit_;
    COM_get_dataitem(bufname + attr, &loc_, &type_, &ncomp_, &unit_);
    if (loc_ != loc || type_ != type || ncomp_ != ncomp || unit_ != unit) {
      std::cout << "ROCMAN Error: NewDataItem::create(): Could not create "
                << bufname + attr << " in two different ways " << std::endl;
      MPI_Abort(MPI_COMM_WORLD, -1);
    } else
      return;
  }
  MAN_DEBUG(2, ("NewDataItem::create: %s%s.\n", bufname.c_str(), attr.c_str()));
  COM_new_dataitem(bufname + attr, loc, type, ncomp, unit);
  COM_resize_array(bufname + attr);
}

CloneDataItem::CloneDataItem(Agent *ag, int cond, std::string target_window_,
                             std::string attr_, std::string parent_window_,
                             std::string parent_attr_, int wg_,
                             const char *ptnname_, int val_)
    : DataItemBase(ag, target_window_, attr_),
      parent_window(parent_window_),
      parent_attr(parent_attr_),
      wg(wg_),
      val(val_),
      condition(cond) {
  ptnname = ptnname_ ? strdup(ptnname_) : NULL;
}

void CloneDataItem::create(std::string bufname) {
  if (parent_window.empty()) {  // filling
    parent_window = agent->get_surface_window();
  }
  if (COM_get_dataitem_handle(bufname + attr) > 0) {
    char loc1, loc2;
    int type1, type2;
    int ncomp1, ncomp2;
    std::string unit1, unit2;
    COM_get_dataitem(bufname + attr, &loc1, &type1, &ncomp1, &unit1);
    COM_get_dataitem(parent_window + parent_attr, &loc2, &type2, &ncomp2,
                     &unit2);
    if (loc1 != loc2 || type1 != type2 || ncomp1 != ncomp2 || unit1 != unit2) {
      std::cout << "ROCMAN Error: CloneDataItem::create(): Could not create "
                << target_window + attr << " in two different ways "
                << std::endl;
      MPI_Abort(MPI_COMM_WORLD, -1);
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
    if (COM_get_dataitem_handle(parent_window + parent_attr) <= 0) return;
  if (ptnname) {
    if (ptnname[0] == '.')
      ptnname = strdup((agent->get_surface_window() + ptnname).c_str());
  }
  COM_clone_dataitem(bufname + attr, parent_window + parent_attr, wg, ptnname,
                     val);
}

UseDataItem::UseDataItem(Agent *ag, std::string target_window_,
                         std::string attr_, std::string parent_window_,
                         std::string parent_attr_, int wg_,
                         const char *ptnname_, int val_)
    : DataItemBase(ag, target_window_, attr_),
      parent_window(parent_window_),
      parent_attr(parent_attr_),
      wg(wg_),
      ptnname(ptnname_),
      val(val_) {}

void UseDataItem::create(std::string bufname) {
  if (parent_window.empty()) {
    parent_window = agent->get_surface_window();
  }
  if (COM_get_dataitem_handle(bufname + attr) > 0) {
    char loc1, loc2;
    int type1, type2;
    int ncomp1, ncomp2;
    std::string unit1, unit2;
    COM_get_dataitem(bufname + attr, &loc1, &type1, &ncomp1, &unit1);
    COM_get_dataitem(parent_window + parent_attr, &loc2, &type2, &ncomp2,
                     &unit2);
    if (loc1 != loc2 || type1 != type2 || ncomp1 != ncomp2 || unit1 != unit2) {
      std::cout << "ROCMAN Error: NewDataItem::create(): Could not create "
                << target_window + attr << " in two different ways "
                << std::endl;
      MPI_Abort(MPI_COMM_WORLD, -1);
    } else
      return;
  }
  MAN_DEBUG(2, ("UseDataItem::create: %s%s %s%s.\n", bufname.c_str(),
                attr.c_str(), parent_window.c_str(), parent_attr.c_str()));
  COM_use_dataitem(bufname + attr, parent_window + parent_attr, wg, ptnname,
                   val);
}

void Agent::register_new_dataitem(std::string target_window_, std::string attr_,
                                  char loc_, int type_, int ncomp_,
                                  const char *unit_) {
  NewDataItem *newAttr =
      new NewDataItem(this, target_window_, attr_, loc_, type_, ncomp_, unit_);
  dataitemList.push_back(newAttr);
}

void Agent::register_clone_dataitem(int cond, std::string target_window_,
                                    std::string attr_,
                                    std::string parent_window_,
                                    std::string parent_attr_, int wg_,
                                    const char *ptnname_, int val_) {
  CloneDataItem *newAttr =
      new CloneDataItem(this, cond, target_window_, attr_, parent_window_,
                        parent_attr_, wg_, ptnname_, val_);
  dataitemList.push_back(newAttr);
}

void Agent::register_use_dataitem(std::string target_window_, std::string attr_,
                                  std::string parent_window_,
                                  std::string parent_attr_, int wg_,
                                  const char *ptnname_, int val_) {
  UseDataItem *newAttr =
      new UseDataItem(this, target_window_, attr_, parent_window_, parent_attr_,
                      wg_, ptnname_, val_);
  dataitemList.push_back(newAttr);
}

void Agent::create_registered_dataitems(std::string tmpBuf) {
  unsigned int n = dataitemList.size();
  for (unsigned int i = 0; i < n; i++) {
    MAN_DEBUG(2, ("[%d] creating %s %s\n", comm_rank,
                  dataitemList[i]->target_window.c_str(),
                  dataitemList[i]->attr.c_str()));
    dataitemList[i]->create(dataitemList[i]->target_window);
  }
  COM_window_init_done(tmpBuf);
}

void Agent::create_registered_window_dataitems(std::string target_window) {
  unsigned int n = dataitemList.size();
  for (unsigned int i = 0; i < n; i++) {
    if (dataitemList[i]->target_window == target_window) {
      MAN_DEBUG(2, ("[%d] creating %s %s\n", comm_rank,
                    dataitemList[i]->target_window.c_str(),
                    dataitemList[i]->attr.c_str()));
      dataitemList[i]->create(dataitemList[i]->target_window);
    }
  }
}

void Agent::create_buffer_all() {
  MAN_DEBUG(1, ("Rocman: Agent %s::create_buffer_all called %s.\n",
                get_agent_name().c_str(), tmp_window.c_str()));
  COM_new_window(tmp_window);
  // COM_use_dataitem(tmp_window, surf_window + ".all", 1);
  COM_use_dataitem(tmp_window, surf_window + ".mesh", 1);
  COM_use_dataitem(tmp_window, surf_window + ".data", 1);
  // create_registered_dataitems(tmp_window);
  create_registered_window_dataitems(tmp_window);
  COM_window_init_done(tmp_window);
}

void Agent::assign_dataitems() {
  unsigned int n = dataitemList.size();
  for (unsigned int i = 0; i < n; i++) {
    dataitemList[i]->assign(tmp_window);
  }
}

void Agent::init_module(double t, double dt) {
  MAN_DEBUG(1, ("[%d] Rocman: %s::init_module with current_time:%e "
                "current_deltatime:%e.\n",
                comm_rank, get_agent_name().c_str(), t, dt));

  current_time = initial_time = t;
  current_deltatime = dt;
  old_dt = 0.0;
}

void Agent::callMethod(Scheduler_voidfn1_t fn, double t) {
  unsigned int i;

  (get_bcInitScheduler().*fn)(t);
  for (i = 0; i < get_bcScheduler_size(); i++) {
    (get_bcScheduler(i)->*fn)(t);
  }
  (get_gmScheduler().*fn)(t);
}

void Agent::schedule() {
  get_icScheduler().schedule();

  get_bcInitScheduler().schedule();
  for (unsigned int i = 0; i < get_bcScheduler_size(); i++) {
    get_bcScheduler(i)->schedule();
  }
  get_gmScheduler().schedule();
}

void Agent::finalize() {
  MAN_DEBUG(1, ("[%d] Rocman: %s::finalize called.\n", comm_rank,
                get_agent_name().c_str()));

  get_icScheduler().finalize_actions();
  get_bcInitScheduler().finalize_actions();
  for (unsigned int i = 0; i < get_bcScheduler_size(); i++)
    get_bcScheduler(i)->finalize_actions();
  get_gmScheduler().finalize_actions();

  // at restart, we don't want windows in physics modules deleted
  if (!get_coupling()->in_restart()) COM_call_function(finalize_handle);
}

double Agent::max_timestep(double t, double dt) { return dt; }

/*
void Agent::run_initactions(double t, double dt)
{
  MAN_DEBUG(1, ("Rocman: %s::run_initactions called with t:%e dt:%e.\n",
get_agent_name().c_str(), t, dt)); initScheduler.set_alpha(0.0);
  //initScheduler.init_actions(t);
  initScheduler.run_actions(t, dt);
}
*/

void Agent::print(FILE *f) {
  unsigned int i;

  // agent
  fprintf(f,
          "graph: { title: \"%s\" label: \"%s\" \n\
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
          get_agent_name().c_str(), get_agent_name().c_str());

  char *bcinit = get_bcInitScheduler().print(f, get_agent_name().c_str());

  // main physics
  std::string main_action = get_agent_name() + "-main";
  fprintf(f,
          "graph: { title: \"%s\" label: \"%s\" \n\
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

  for (i = 0; i < get_bcScheduler_size(); i++)
    get_bcScheduler(i)->print(f, get_agent_name().c_str());

  get_gmScheduler().print(f, get_agent_name().c_str());

  std::string main_physics = get_agent_name() + "-physics";
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
