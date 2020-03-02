#include <iostream>

#include "Interpolate.h"

#include "RocBlas.h"

#include "Agent.h"
#include "Coupling.h"

#define MAN_DEBUG(l, x) ((void)0)

InterpolateBase::InterpolateBase(Agent *ag, Agent *bkag,
                                 const std::string &attr, bool cond, int ord)
    : Action(
          {
              {attr, 0, IN},
              {attr + "_old", 0, IN},
              {attr + "_alp", 0, OUT},
              {attr + "_grad", 0, IN},
              {attr + "_old", 0, OUT}, // for backup
              {attr + "_grad", 0, OUT} // for backup
          },
          "InterpolateBase"),
      agent(ag),
      bkagent(bkag), attr_hdls{-1, -1, -1, -1}, bkup_hdls{-1, -1, -1},
      conditional(cond) {
  // register to Agent for create_dataitem() and backup() callback
  if (bkagent)
    bkagent->register_interpolate(this);
}

void InterpolateBase::init(double t) {
  MAN_DEBUG(3, ("%s::init called.\n", name().c_str()));

  // INIT_INTERP_HANDLES() in "man_basic.f90"
  attr_hdls[0] = get_dataitem_handle(0);
  attr_hdls[1] = get_dataitem_handle(1);       // old
  attr_hdls[2] = get_dataitem_handle(2, true); // alp
  attr_hdls[3] = get_dataitem_handle(3, true); // grad

  /* AEG: Does not affect the results of ACMRocflu test.
  for (int i = 0; i < action_data.size(); i++) {
    if (action_data[i].inout == OUT) {
      // init to 0
      double zero = 0.0;
      COM_call_function(RocBlas::copy_scalar, &zero, &attr_hdls[i]);
    }
  }
  */

  if (attr_hdls[1] <= 0 && attr_hdls[2] <= 0) {
    std::cout << "IMPACT Warning: Could not find dataitem "
              << action_data[2].attr << std::endl;
  } else if (attr_hdls[2] <= 0) {
    COM_abort_msg(EXIT_FAILURE, "IMPACT Error: Could not find DataItem " +
                                    action_data[2].attr);
  }

  // backup handles
  bkup_hdls[0] = get_dataitem_handle(0);
  bkup_hdls[1] = get_dataitem_handle(4);       // old
  bkup_hdls[2] = get_dataitem_handle(5, true); // grad
}

void InterpolateBase::backup() {
  MAN_DEBUG(3,
            ("Rocstar: InterpolateBase::backup (%s) called with dt_old:%e.\n",
             action_data[0].attr.c_str(), agent->get_old_dt()));

  // BACKUP() in "man_basic.f90"
  if (bkup_hdls[0] > 0 && bkup_hdls[1] > 0) {
    if (bkup_hdls[2] > 0) {
      // Compute gradient
      double dt_old = agent->get_old_dt();
      if (dt_old > 0.0) {
        COM_call_function(RocBlas::sub, &bkup_hdls[0], &bkup_hdls[1],
                          &bkup_hdls[2]);
        COM_call_function(RocBlas::div_scalar, &bkup_hdls[2], &dt_old,
                          &bkup_hdls[2]);
      } else {
        double v = 0.0;
        COM_call_function(RocBlas::copy_scalar, &v, &bkup_hdls[2]);
      }
    }
    COM_call_function(RocBlas::copy, &bkup_hdls[0], &bkup_hdls[1]);
  }
}

// INTERPOLATE_LINEAR
void InterpolateBase::extrapolate_Linear(double dt, double dt_old,
                                         double time_old, int a_old,
                                         double time_new, int a_new,
                                         double time_out, int a_out,
                                         int a_grad) {
  // printf("extrapolate_Linear: %f %d %f %d %f %d %d\n", time_old, a_old,
  // time_new, a_new, time_out, a_out, a_grad);

  if (time_out == time_new) {
    COM_call_function(RocBlas::copy, &a_new, &a_out);
  } else if (a_old <= 0) {
    COM_abort_msg(
        EXIT_FAILURE,
        "IMPACT Error: Could not find the old DataItem correspond to the "
        "attribute with handle " +
            std::to_string(a_out));
  } else if (time_out == time_old) {
    COM_call_function(RocBlas::copy, &a_old, &a_out);
  } else {
    // See the interpolation section in developers' guide for the algorithm

    COM_call_function(RocBlas::sub, &a_new, &a_old, &a_out);
    double a;
    if (time_old == 0.0) {
      a = time_out - 1.0;
    } else if (time_old == -0.5) {
      if (a_grad > 0) {
        double time = (dt_old + dt) / 2.0;
        COM_call_function(RocBlas::div_scalar, &a_out, &time, &a_out);
        COM_call_function(RocBlas::limit1, &a_grad, &a_out, &a_out);
        a = (time_out - 0.5) * dt;
      } else {
        a = 2.0 * (time_out - 0.5) * dt / (dt_old + dt);
      }
    } else if (time_old == -1.0) {
      COM_abort_msg(EXIT_FAILURE, "ERROR: Abort!");
    } else {
      COM_abort_msg(
          EXIT_FAILURE,
          "IMPACT Error: Unsupported interpolation mode with old time stamp " +
              std::to_string(time_old));
    }
    COM_call_function(RocBlas::axpy_scalar, &a, &a_out, &a_new, &a_out);
  }
}

Extrapolate_Linear::Extrapolate_Linear(Agent *ag, Agent *bkag,
                                       const std::string &attr, bool cond,
                                       int ord)
    : InterpolateBase(ag, bkag, attr, cond, ord) {
  action_name = "Extrapolate_Linear";
}

void Extrapolate_Linear::init(double t) {
  // check condition
  attr_hdls[2] = get_dataitem_handle(2, true); // alp

  if (conditional)
    if (attr_hdls[2] <= 0)
      return;

  InterpolateBase::init(t);
}

void Extrapolate_Linear::run(double t, double dt, double alpha) {
  if (attr_hdls[2] <= 0)
    return;

  MAN_DEBUG(3,
            ("Extrapolate_Linear::run (%s) called with t:%e dt:%e alpha:%e.\n",
             action_data[0].attr.c_str(), t, dt, alpha));

  COM_assertion_msg(alpha >= -1.e-6 && alpha <= 1 + 1.e-6, "ERROR: Abort!");

  if (order == 0 || agent->get_coupling()->new_start(t)) {
    COM_call_function(RocBlas::copy, &attr_hdls[0], &attr_hdls[2]);
    return;
  }

  if (agent->get_coupling()->get_ipc() <= 1 && attr_hdls[3] <= 0) {
    COM_call_function(RocBlas::copy, &attr_hdls[0], &attr_hdls[2]);
    return;
  }
  double base = 0.0;

  // linear interpolation.
  extrapolate_Linear(dt, agent->get_old_dt(), base, attr_hdls[1], base + 1.0,
                     attr_hdls[0], alpha, attr_hdls[2], attr_hdls[3]);
}

Extrapolate_Central::Extrapolate_Central(Agent *ag, Agent *bkag,
                                         const std::string &attr, bool cond,
                                         int ord)
    : InterpolateBase(ag, bkag, attr, cond, ord) {
  action_name = "Extrapolate_Central";
}

void Extrapolate_Central::init(double t) {
  // check condition
  // first check if window exists
  std::string dataitem = action_data[0].attr;
  std::string::size_type pos = dataitem.find('.');
  COM_assertion_msg(pos != std::string::npos, "ERROR: Here!");
  std::string wname = dataitem.substr(0, pos);
  if (COM_get_window_handle(wname) <= 0)
    return;

  // check if dataitem exists
  attr_hdls[2] = get_dataitem_handle(2, true); // alp
  // attr_hdls[2] = COM_get_dataitem_handle(action_data[2].attr); // alp
  if (conditional)
    if (attr_hdls[2] <= 0)
      return;

  InterpolateBase::init(t);
}

void Extrapolate_Central::run(double t, double dt, double alpha) {
  if (attr_hdls[2] <= 0)
    return;

  MAN_DEBUG(3,
            ("Extrapolate_Central::run (%s) called with t:%e dt:%e alpha:%e.\n",
             action_data[0].attr.c_str(), t, dt, alpha));

  COM_assertion_msg(alpha >= 0.0,
                    "ERROR: Extrapolate_Central called with invalid alpha!");

  COM_assertion_msg(alpha >= -1.e-6 && alpha <= 1 + 1.e-6, "ERROR: Abort!");

  if (order == 0 || agent->get_coupling()->new_start(t)) {
    COM_call_function(RocBlas::copy, &attr_hdls[0], &attr_hdls[2]);
    return;
  }

  if (agent->get_coupling()->get_ipc() <= 1 && attr_hdls[3] <= 0) {
    COM_call_function(RocBlas::copy, &attr_hdls[0], &attr_hdls[2]);
    return;
  }
  double base = -0.5;
  // linear interpolation.
  extrapolate_Linear(dt, agent->get_old_dt(), base, attr_hdls[1], base + 1.0,
                     attr_hdls[0], alpha, attr_hdls[2], attr_hdls[3]);
}

Interpolate_Linear::Interpolate_Linear(Agent *ag, Agent *bkag,
                                       const std::string &attr, int ord)
    : InterpolateBase(ag, bkag, attr, false, ord) {
  action_name = "Interpolate_Linear";
}

void Interpolate_Linear::run(double t, double dt, double alpha) {
  if (attr_hdls[2] <= 0)
    return;

  MAN_DEBUG(3,
            ("Interpolate_Linear::run (%s) called with t:%e dt:%e alpha:%e.\n",
             action_data[0].attr.c_str(), t, dt, alpha));

  COM_assertion_msg(alpha >= 0.0,
                    "ERROR: Interpolate_Linear called with invalid alpha!");

  COM_assertion_msg(alpha >= -1.e-6 && alpha <= 1 + 1.e-6, "ERROR: Abort!");

  if (order == 0 || agent->get_coupling()->new_start(t)) {
    COM_call_function(RocBlas::copy, &attr_hdls[0], &attr_hdls[2]);
    return;
  }

  double base = 0.0;
  // linear interpolation.
  extrapolate_Linear(dt, agent->get_old_dt(), base, attr_hdls[1], base + 1.0,
                     attr_hdls[0], alpha, attr_hdls[2], attr_hdls[3]);
}

Interpolate_Constant::Interpolate_Constant(Agent *ag, Agent *bkag,
                                           const std::string &attr, int ord)
    : InterpolateBase(ag, bkag, attr, false, ord) {
  action_name = "Interpolate_Constant";
}

void Interpolate_Constant::run(double t_dummy, double dt_dummy, double alpha) {
  if (attr_hdls[2] <= 0)
    return;

  MAN_DEBUG(3,
            ("Interpolate_Constant::run (bc) called with alpha:%e.\n", alpha));

  COM_call_function(RocBlas::copy, &attr_hdls[0], &attr_hdls[2]);
}

Interpolate_Central::Interpolate_Central(Agent *ag, Agent *bkag,
                                         const std::string &attr, int ord)
    : InterpolateBase(ag, bkag, attr, false, ord) {
  action_name = "Interpolate_Central";
}

void Interpolate_Central::run(double t, double dt, double alpha) {
  if (attr_hdls[2] <= 0)
    return;

  MAN_DEBUG(3,
            ("Interpolate_Central::run (%s) called with t:%e dt:%e alpha:%e.\n",
             action_data[0].attr.c_str(), t, dt, alpha));

  COM_assertion_msg(alpha >= 0.0,
                    "ERROR: Extrapolate_Central called with invalid alpha!");

  COM_assertion_msg(alpha >= -1.e-6 && alpha <= 1 + 1.e-6, "ERROR: Abort!");

  if (order == 0 || agent->get_coupling()->new_start(t)) {
    COM_call_function(RocBlas::copy, &attr_hdls[0], &attr_hdls[2]);
    return;
  }

  double base = -0.5;

  // linear interpolation.
  extrapolate_Linear(dt, agent->get_old_dt(), base, attr_hdls[1], base + 1.0,
                     attr_hdls[0], alpha, attr_hdls[2], attr_hdls[3]);
}
