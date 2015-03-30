//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef _FLUIDAGENT_H_
#define _FLUIDAGENT_H_

#include "Agent.h"

class FluidAgent: public Agent {
public:

  FluidAgent(Coupling *coup, std::string mod, std::string obj, MPI_Comm com, int withSolid=0);

  virtual void input( double t);

  virtual void load_module();
  virtual void unload_module();
  virtual void init_module(double t, double dt);
  virtual void finalize();

  virtual void read_restart_data();
  virtual void output_restart_files( double t);
  virtual void output_visualization_files( double t);

  virtual void create_buffer_all();

  virtual void init_convergence( int iPredCorr);
  virtual int check_convergence( double tolerMass, double tolerTract, double tolerVelo);

  virtual int compute_integrals();
protected:
  bool   with_plag;
  string plag_window;
  int    with_solid;
  
  // Window name.
//  static const char *window_name;  

public:
  std::string ifluid;
  std::string fluid;
  std::string fluid_plag;

  std::string fluidSurfIn;
  std::string fluidVolIn;
  std::string fluidPlagIn;
  std::string fluidVPIn;

  std::string ifluid_all;
  std::string ifluid_i;			// FluidBuf

    // Surface windows for SimOUT, Surface window buffers
  std::string ifluid_b;
  std::string ifluid_nb;
  std::string ifluid_ni;

  std::string propBufAll;
  std::string fluidBufNG;
  std::string propBuf;
  std::string fluidBufB;
  std::string fluidBufNB;

  std::string fluidBufPC;
  std::string fluidBufBak;
  std::string fluidBufPRE;

  std::string fluidVolBak;
  std::string fluidPlagBak;

  int f_mdot_hdl, f_mdot_pre_hdl, f_ts_hdl, f_ts_pre_hdl, f_vm_hdl, f_vm_pre_hdl;
    // for update distances
  int nc_hdl, nc_tmp_hdl, sq_dist_hdl;
};


#endif



