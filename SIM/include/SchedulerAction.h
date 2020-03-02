#ifndef _IMPACT_SCHEDULERACTION_H_
#define _IMPACT_SCHEDULERACTION_H_

#include "Action.h"
#include "Scheduler.h"

/**
 * A SchedulerAction manages a list of Action using a sub-scheduler.
 */
class SchedulerAction : public Action {
public:
  explicit SchedulerAction(Scheduler *s, const char *name = nullptr)
      : Action(name), sub_schedule(s) {}
  SchedulerAction(Scheduler *s, ActionDataList adl, std::string name = "")
      : Action(std::move(adl), std::move(name)), sub_schedule(s) {}

  ~SchedulerAction() override;

public:
  void init(double t) override;
  void run(double t, double dt, double alpha) override;
  void finalize() override;

public:
  void print(FILE *f) const override;
  void print_toposort(FILE *f) const override;

  void schedule() override;

private:
  Scheduler *sub_schedule;
};

#endif //_IMPACT_SCHEDULERACTION_H_
