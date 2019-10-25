//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "SchedulerAction.h"

class Scheduler {
  friend class SchedulerAction;

 public:
  Scheduler();
  virtual ~Scheduler() {}

  virtual void add_action(Action *);

  void reads(Action *, const char *attr, int idx);
  void writes(Action *, const char *attr, int idx);

  virtual void schedule();

  void init_actions(double t);
  void run_actions(double t, double dt);
  void finalize_actions();

  void print(const char *fname);  // called by user
  char *print(FILE *f,
              const char *container_name);  // print a scheduler to file
  void printDDG(FILE *f);

  const char *name() { return scheduler_name.c_str(); }
  void set_name(const char *name) { scheduler_name = name; }

  void restarting(double t) { inited = 0; }  // clear init state for restarting
  void set_alpha(double alpha) { alphaT = alpha; }
  bool isEmpty() { return actions.empty(); }

  void print_toposort(FILE *f);

 protected:
  std::string scheduler_name;

  struct ActionItem;
  typedef std::vector<ActionItem *> ActionList;

  struct ActionItem {
    Action *myaction;

    std::vector<const char *> read_attr;
    std::vector<int> read_idx;
    ActionList input;  // read_n

    std::vector<const char *> write_attr;
    std::vector<int> write_idx;
    ActionList output;  // write_n

    int print_flag;  // for print

    ActionItem(Action *a) : myaction(a) {}

    inline unsigned int n_write() { return write_attr.size(); }
    inline unsigned int n_read() { return read_attr.size(); }
    inline const char *name() { return myaction->name(); }
    inline Action *action() { return myaction; }
    int fulfilled();  // all output action identified
    int hasInput(const char *attr, int idx);
    int hasOutput(const char *attr, int idx);
    void print(FILE *f);
  };

  ActionList actions;  // all actions registered
  ActionList roots;    // forest
  ActionList sort;     // topological sort order

  double alphaT;  //

  int scheduled;  // flag: if has been scheduled
  int inited;     // flag: true if init called
 protected:
  void buildDDG();

 private:
  void discoverSets(ActionList& pool); // returns true if disjoint sets discovered
  void topological_sort();
  void sanityCheck();
  void print_helper(FILE *f, ActionItem *aitem);
  void printActions();  // for debugging
};

typedef void (Scheduler::*Scheduler_voidfn1_t)(double);

#endif
