//
// Created by agondolo on 9/5/18.
//

#ifndef _IMPACT_SCHEDULERACTION_H_
#define _IMPACT_SCHEDULERACTION_H_

#include "Action.h"
#include "Scheduler.h"

// a list of subactions managed by a sub-scheduler
class SchedulerAction : public Action {
 private:
  Scheduler *sched;

 public:
  SchedulerAction(Scheduler *s, int n, const char *at[], int i[], void *p = 0,
                  const char *name = NULL)
      : Action(n, at, i, p, name), sched(s) {}

  virtual ~SchedulerAction();

  virtual void init(double t);
  virtual void run(double t, double dt, double alpha);
  virtual void finalize();

  virtual void declare(Scheduler &) = 0;

  virtual void print(FILE *f) { Action::print(f); }
  virtual void print(FILE *f, char *container_name);
  virtual void print_toposort(FILE *f);

  virtual void schedule();
};

#endif  //_IMPACT_SCHEDULERACTION_H_
