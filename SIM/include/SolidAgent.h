//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef _SOLIDAGENT_H_
#define _SOLIDAGENT_H_

#include "Agent.h"

class InterpolateBase;

const int NO_SHEER=1;
const int WITH_SHEER=2;

class SolidAgent: public Agent {
public:

  SolidAgent(Coupling *coup, std::string mod, std::string obj, MPI_Comm com, int withFluid=0);

    // read control files
  virtual void input( double t);

  virtual void load_module();
  virtual void unload_module();
  virtual void init_module(double t, double dt);
  virtual void finalize();

  virtual void create_buffer_all();

  virtual void read_restart_data();
  virtual void output_restart_files( double t);

  virtual int compute_integrals();

    // not implemented
  virtual void output_visualization_files( double t);
protected:
  // Window name.
//  static const char *window_name;  
public:
  std::string isolid_i;

  std::string isolid_all;		// for registering dataitems
  std::string isolid_b;			// buring
  std::string isolid_nb;		// non-buring
  std::string isolid_ni;		// noninteracting

  std::string solidBufBase;		// a window for intermediate buffers for fluid-solid interface
  std::string solidBuf;			// a window for solid/fluid interaction
  std::string propBufAll;		// for surface propagation (containing all panes)
  std::string propBuf;			// for surface propagation

    // for input
  std::string isolid;
  std::string solid;

   // SimIN windows
  std::string solidSurfIN;
  std::string solidVolIN;

  std::string solidBufBak;
  std::string solidVolBak;

  int with_fluid;			// flag: coupled
  int withALE;				// TODO init it
  int rhos_mode;
  int size_ts;
  int traction_mode;

  int y_hdl;				// for compute distances
};



#endif



