#ifndef _IMPACT_USERSCHEDULER_H_
#define _IMPACT_USERSCHEDULER_H_

#include "Scheduler.h"

/**
 * UserScheduler will follow the order of actions specified by the user.
 *
 * Using add_action(), the actions are performed in FIFO order.
 */
class UserScheduler : public Scheduler {
  using Scheduler::Scheduler;

public:
  void schedule() override;
};

#endif //_IMPACT_USERSCHEDULER_H_
