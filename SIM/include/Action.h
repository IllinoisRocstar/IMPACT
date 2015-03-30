//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef _ACTION_H_
#define _ACTION_H_

#include <vector>
#include <cstring>

class Scheduler;
class UserScheduler;

class Action
{
protected:
  enum { IN=1, OUT=2, INOUT=3 };
  const char *action_name;
  char **attr;
  int *idx;
  int count;
  void *usr_ptr;

  std::vector<int> inout;
public:
  Action(void *p=0, const char *name=NULL);
  //  Action(int n, const char *at[], int *i=NULL, void *p=0, char *name=NULL);
  Action(int n, const char **at, int *i=NULL, void *p=0, const char *name=NULL);
  Action(int n, const std::string at[], int *i=NULL, void *p=0, const char *name=NULL);

  virtual ~Action();
  virtual void declare(Scheduler &);
  virtual void init(double t) { /* printf("Rocman: Action %s init. \n", name());*/ }
  virtual void run(double t, double dt, double alpha) {/* printf("Action %s runs. \n", name()); */}
  virtual void finalize() { /* printf("Rocman: Action %s finalized. \n", name()); */ }
  virtual inline const char *name() { return action_name; }
  inline void set_name(const char *name) { action_name = strdup(name); }
  virtual void print(FILE *f);
  virtual void print_toposort(FILE *f) { fprintf(f, "%s ", name()); }
  virtual void schedule()  {}			// do nothing

protected:
  // Obtain dataitem handle of ith dataitem
  int get_dataitem_handle( int i);
  int get_dataitem_handle_const( int i);
  int get_dataitem_handle( const std::string str);

  // set dataitems
  void set_attr(int n, const std::string at[], int *id=NULL);
  void set_attr(int n, const char * at[], int *id=NULL);

  // Set I/O properties of arguments
  void set_io( int n, const int *io) 
  { inout.clear(); inout.insert( inout.end(), io, io+n); if (count==0) count=n; }

  void set_io( const char *io);   // TODO
  int get_io(int i) { return inout[i]; }
};

// a list of subactions managed by a sub-scheduler
class SchedulerAction: public Action
{
private:
  Scheduler * sched;
public:
  SchedulerAction(Scheduler *s, int n, const char *at[], int i[], void *p=0, char *name=NULL): Action(n, at, i, p, name), sched(s)  {}
  virtual ~SchedulerAction();
  virtual void declare(Scheduler &) = 0;
  virtual void init(double t); 
  virtual void run(double t, double dt, double alpha);
  virtual void finalize();
  virtual void print(FILE *f, char *container_name);
  virtual void print_toposort(FILE *f);
  virtual void schedule();
};

class UserSchedulerAction: public Action
{
private:
  UserScheduler * sched;
public:
  UserSchedulerAction(UserScheduler *s, int n, const char *at[], int i[], void *p=0, char *name=NULL): Action(n, at, i, p, name), sched(s)  {}
  virtual ~UserSchedulerAction();
//  virtual void declare(Scheduler &) = 0;
  virtual void init(double t); 
  virtual void run(double t, double dt, double alpha);
  virtual void finalize();
  virtual void print(FILE *f, char *container_name);
  virtual void print_toposort(FILE *f);
  virtual void schedule();
};


#endif



