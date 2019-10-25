//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <cstdlib>
#include <cstring>
#include <typeinfo>

#include "Action.h"
#include "Scheduler.h"
#include "com.h"

Action::Action(void *p, const char *name)
    : action_name(name), attr(NULL), idx(NULL), count(0), usr_ptr(p) {}

Action::Action(int n, const char **at, int *id, void *p, const char *name)
    : action_name(name), usr_ptr(p) {
  set_attr(n, at, id);
  if (action_name == NULL) action_name = typeid(*this).name();
}

Action::Action(int n, const std::string at[], int *id, void *p,
               const char *name)
    : action_name(name), usr_ptr(p) {
  set_attr(n, at, id);
  if (action_name == NULL) action_name = typeid(*this).name();
}

Action::~Action() {
  for (int i = 0; i < count; ++i) free(attr[i]);
  delete[] attr;
  delete[] idx;
}

// Declare the input and output variables
void Action::declare(Scheduler &sched) {
  for (int i = 0; i < count; ++i) {
    if (inout[i] & IN) sched.reads(this, attr[i], idx[i]);
    if (inout[i] & OUT) sched.writes(this, attr[i], idx[i]);
  }
}

void Action::set_name(const char *name) { action_name = strdup(name); }

void Action::print(FILE *f) {
  fprintf(f, "node: { title:\"%s\" label:\"%s\"}\n", name(), name());
}

// Obtain dataitem handle of ith dataitem
int Action::get_dataitem_handle(int i) {
  COM_assertion_msg(
      attr[i] != NULL,
      (std::string("DataItem \"") + attr[i] + "\" does not exist").c_str());
  int hdl = COM_get_dataitem_handle(attr[i]);
  COM_assertion_msg(
      hdl > 0,
      (std::string("DataItem \"") + attr[i] + "\" does not exist").c_str());
  return hdl;
}

int Action::get_dataitem_handle(const std::string str) {
  int hdl = COM_get_dataitem_handle(str);
  COM_assertion_msg(
      hdl > 0,
      (std::string("DataItem \"") + str + "\" does not exist").c_str());
  return hdl;
}

// Obtain dataitem handle of ith dataitem
int Action::get_dataitem_handle_const(int i) {
  int hdl = COM_get_dataitem_handle_const(attr[i]);
  COM_assertion_msg(
      hdl > 0,
      (std::string("DataItem \"") + attr[i] + "\" does not exist").c_str());
  return hdl;
}

void Action::set_attr(int n, const char *at[], int *id) {
  int i;
  count = n;
  attr = new char *[count];
  for (i = 0; i < count; ++i) attr[i] = strdup(at[i]);
  idx = new int[count];
  for (i = 0; i < count; ++i) idx[i] = id ? id[i] : 0;
}

void Action::set_attr(int n, const std::string at[], int *id) {
  int i;
  count = n;
  attr = new char *[count];
  for (i = 0; i < count; ++i) attr[i] = strdup(at[i].c_str());
  idx = new int[count];
  for (i = 0; i < count; ++i) idx[i] = id ? id[i] : 0;
}
