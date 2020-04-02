//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <typeinfo>
#include <utility>

#include "Action.h"
#include "Scheduler.h"
#include "com.h"

Action::Action(const std::string &name) : Action(ActionDataList(), name) {}

Action::Action(ActionDataList actionDataList, std::string name)
    : action_name(std::move(name)), action_data(std::move(actionDataList)) {
  if (action_name.empty())
    action_name = typeid(*this).name();
}

// Declare the input and output variables
void Action::declare(Scheduler &sched) const {
  for (const auto &ad : action_data) {
    switch (ad.inout) {
    case IN:
      sched.reads(this, ad.attr, ad.idx);
      break;
    case OUT:
      sched.writes(this, ad.attr, ad.idx);
      break;
    }
  }
}

void Action::print(FILE *f) const {
  fprintf(f, "node: { title:\"%s\" label:\"%s\"}\n", name().c_str(),
          name().c_str());
}

void Action::print_toposort(FILE *f) const {
  fprintf(f, "%s ", name().c_str());
}

int Action::get_dataitem_handle(int i, bool optional) const {
  int hdl = -1;
  switch (action_data[i].inout) {
  case IN:
    hdl = COM_get_dataitem_handle_const(action_data[i].attr);
    break;
  case OUT:
    hdl = COM_get_dataitem_handle(action_data[i].attr);
    break;
  }
  if (!optional && hdl <= 0)
    COM_abort_msg(EXIT_FAILURE,
                  "IMPACT ERROR: Action cannot access DataItem '" +
                      action_data[i].attr + "' as it does not exist.");
  return hdl;
}
