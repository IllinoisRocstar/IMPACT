#ifndef _INTERPOLATE_H_
#define _INTERPOLATE_H_

#include "Action.h"

class Agent;

class InterpolateBase : public Action {
public:
  /**
   * Abstract interpolation and extrapoaltion Action
   * @param ag Agent providing time stepping information
   * @param bkag Agent providing backup
   * @param attr attribute name for interpolation
   * @param cond conditional action, check if *_alp exists
   * @param ord interpolation order
   */
  InterpolateBase(Agent *ag, Agent *bkag, const std::string &attr, bool cond,
                  int ord);

  void init(double t) override;
  void finalize() override {}

  // BACKUP in "man_basic.f90"
  void backup();

protected:
  static void extrapolate_Linear(double dt, double dt_old, double time_old,
                                 int a_old, double time_new, int a_new,
                                 double time_out, int a_out, int a_gra = -100);

protected:
  Agent *agent;     ///< Agent providing time stepping information
  Agent *bkagent;   ///< Agent providing backup
  int attr_hdls[4]; ///< COM handles for interpolation attributes
  int bkup_hdls[3]; ///< COM handles for backup attributes
  bool conditional; ///< conditional action, check alp for on/off
  int order;        ///< interpolation order
};

class Extrapolate_Linear : public InterpolateBase {
public:
  Extrapolate_Linear(Agent *ag, Agent *bkag, const std::string &attr, bool cond,
                     int ord);
  void init(double t) override;
  void run(double t, double dt, double alpha) override;
};

class Extrapolate_Central : public InterpolateBase {
public:
  Extrapolate_Central(Agent *ag, Agent *bkag, const std::string &attr,
                      bool cond, int ord);
  void init(double t) override;
  void run(double t, double dt, double alpha) override;
};

class Interpolate_Linear : public InterpolateBase {
public:
  Interpolate_Linear(Agent *ag, Agent *bkag, const std::string &attr, int ord);
  void run(double t, double dt, double alpha) override;
};

class Interpolate_Constant : public InterpolateBase {
public:
  Interpolate_Constant(Agent *ag, Agent *bkag, const std::string &attr,
                       int ord);
  void run(double t, double dt, double alpha) override;
};

class Interpolate_Central : public InterpolateBase {
public:
  Interpolate_Central(Agent *ag, Agent *bkag, const std::string &attr, int ord);
  void run(double t, double dt, double alpha) override;
};

#endif //_INTERPOLATE_H_
