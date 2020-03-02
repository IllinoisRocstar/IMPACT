#ifndef _IMPACT_DDGSCHEDULER_H_
#define _IMPACT_DDGSCHEDULER_H_

#include "Scheduler.h"

/**
 * DDGScheduler will connect all ActionData inputs and outputs to create a
 * graph. No dangling inputs or outputs are allowed. The graph must be acyclic.
 *
 * The directed acyclic graph (DAG) is passed through a topological sort
 * algorithm to determine the order the Actions must be performed.
 */
class DDGScheduler : public Scheduler {
  using Scheduler::Scheduler;

public:
  void schedule() override;

private:
  /**
   * Build Data Dependency Graph.
   */
  void buildDDG();
  /**
   * Check all inputs and outputs are fulfilled.
   */
  void sanityCheck() const;
  /**
   * Use DDG build with buildDDG to sort actions.
   */
  void topological_sort();

  /**
   * TODO: returns true if disjoint sets discovered
   * @param pool
   */
  static void discoverSets(const ActionList &pool);
};

#endif // _IMPACT_DDGSCHEDULER_H_
