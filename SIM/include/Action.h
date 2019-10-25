//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef _ACTION_H_
#define _ACTION_H_

#include <cstdio>
#include <string>
#include <vector>

class Scheduler;

class Action {
 protected:
  enum { IN = 1, OUT = 2, INOUT = 3 };

  const char *action_name;
  char **attr;
  int *idx;
  int count;
  void *usr_ptr;

  std::vector<int> inout;

 public:
  // Constructors
  Action(void *p = 0, const char *name = NULL);
  Action(int n, const char **at, int *i = NULL, void *p = 0,
         const char *name = NULL);
  Action(int n, const std::string at[], int *i = NULL, void *p = 0,
         const char *name = NULL);

  // Deconstructor
  virtual ~Action();

  // Core pure virtual methods
  virtual void init(double t) {}
  virtual void run(double t, double dt, double alpha) {}
  virtual void finalize() {}

  // Scheduler interaction
  virtual void declare(Scheduler &);

  // Visualization methods
  virtual inline const char *name() { return action_name; }
  void set_name(const char *name);

  virtual void print(FILE *f);
  virtual void print_toposort(FILE *f) { fprintf(f, "%s ", name()); }

  // Other
  virtual void schedule(){};

 protected:
  // Obtain dataitem handle of ith dataitem
  int get_dataitem_handle(int i);
  int get_dataitem_handle_const(int i);
  int get_dataitem_handle(const std::string str);

  // set dataitems
  void set_attr(int n, const std::string at[], int *id = NULL);
  void set_attr(int n, const char *at[], int *id = NULL);

  // Set I/O properties of arguments
  void set_io(int n, const int *io) {
    inout.clear();
    inout.insert(inout.end(), io, io + n);
    if (count == 0) count = n;
  }

  void set_io(const char *io);  // TODO
  int get_io(int i) { return inout[i]; }
};

#endif
