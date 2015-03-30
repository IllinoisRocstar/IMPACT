//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef _BURNAGENT_H_
#define _BURNAGENT_H_

#include "Agent.h"

class BurnAgent: public Agent {
public:

  BurnAgent(Coupling *coup, std::string mod, std::string obj, MPI_Comm com, const std::string parent);

  virtual void input( double t);

  virtual void load_module();
  virtual void unload_module();
  virtual void init_module(double t, double dt);
  virtual void finalize();

  virtual void read_restart_data();
  virtual void output_restart_files( double t);
  virtual void output_visualization_files( double t);

  virtual void create_buffer_all();
protected:
  // Window name.
  static const char *window_name;  
  int  tbl_flag;
public:
  std::string parentWin;  
//  std::string iburn_i;

  std::string iburn_all;		// for registering dataitems
  std::string iburn_ng;		

    // for input
  std::string iburn;
  std::string burn;

   // SimIN windows
  std::string burnSurfIN;
  std::string burnVolIN;

  std::string burnIntBak;

  std::string burnBufOUT;
  bool ignmodel;
};

#endif



