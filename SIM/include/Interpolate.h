//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef _INTERPOLATE_H_
#define _INTERPOLATE_H_

#include "rocman.h"
#include "SolidAgent.h"

class InterpolateBase : public Action {
public:
  InterpolateBase(Agent *ag, Agent *bkag, int cond=0);
//  InterpolateBase(const char *at[]) : Action(4, at, NULL, NULL, "InterpolateBase") {}
  virtual void init(double t);
  virtual void run(double t, double dt, double alpha) = 0;
    // BACKUP in "man_basic.f90"
  void backup();
protected:
  void extrapolate_Linear(double dt, double dt_old, double time_old, int a_old, double time_new, int a_new, double time_out, int a_out, int a_gra=-100);
protected:
  Agent *agent;
  Agent *bkagent;
  int attr_hdls[4];
  int bkup_hdls[3];
  int conditional;              // conditional action, check alp for on/off
  int on;
};

class Extrapolate_Linear : public InterpolateBase {
public:
  Extrapolate_Linear(Agent *ag, Agent *bkag, const std::string attr, const std::string alpsuf="_alp", int cond=0);
  virtual void init(double t);
  void run(double t, double dt, double alpha);
};

class Extrapolate_Central : public InterpolateBase {
public:
  Extrapolate_Central(Agent *ag, Agent *bkag, const std::string attr, const std::string alpsuf="_alp", int cond=0);
  virtual void init(double t);
  void run(double t, double dt, double alpha);
};

class Interpolate_Linear : public InterpolateBase {
public:
  Interpolate_Linear(Agent *ag, Agent *bkag, const std::string attr, const std::string alpsuf="_alp");
  void run(double t, double dt, double alpha);
};

class Interpolate_Constant : public InterpolateBase {
public:
  Interpolate_Constant(Agent *ag, Agent *bkag, const std::string attr, const std::string alpsuf="_alp");
  void run(double t, double dt, double alpha);
};

class Interpolate_Central : public InterpolateBase {
public:
  Interpolate_Central(Agent *ag, Agent *bkag, const std::string attr, const std::string alpsuf="_alp");
  void run(double t, double dt, double alpha);
};



#endif



