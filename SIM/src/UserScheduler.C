//
// Created by agondolo on 8/31/18.
//

#include "UserScheduler.h"
#include "com.h"

void UserScheduler::schedule() {
  if (scheduled)
    COM_abort_msg(EXIT_FAILURE, "ERROR: Scheduler has already been scheduled.");

  // TODO: Remove hard-coded prints to stdout. Use a logger class.
  FILE *f = stdout;

  // schedule all sub-schedulers
  for (auto &&action : actions)
    action->action()->schedule();

  if (verbose) {
    fprintf(f, "-------------- UserScheduler %s -------------\n",
            scheduler_name.c_str());
    this->printActions(f);
  }

  for (auto i = 0; i < actions.size(); ++i) {
    if (i + 1 < actions.size()) {
      actions[i]->output.push_back(actions[i + 1]);
      actions[i + 1]->input.push_back(actions[i]);
    }
  }

  for (const auto &action : actions) {
    sort.push_back(action);
    if (roots.empty())
      roots.push_back(action);
  }

  // debugging
  if (verbose) {
    fprintf(f, "Sorted:\n");
    print_toposort(f);
    fprintf(f, "\n");

    // print out the scheduler graph using GDL
    // print("Dependency.gdl");
  }

  scheduled = true;
}
