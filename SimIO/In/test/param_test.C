//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

/** A test program that reads in a given parameter file and writes 
 *  out the resulting parameter window into one HDF file using 
 *  Rocout with a given file prefix. */

#include "Rocin.h"
#include "COM_base.hpp"
#include "com.h"
#include "com_devel.hpp"

#include <iostream>
#include <cstring>
#include <string>

COM_EXTERN_MODULE( SimIN);
COM_EXTERN_MODULE( SimOUT);

using namespace std;

int main(int argc, char *argv[]) {
  COM_init( &argc, &argv);

  if ( argc < 3 || argc > 3 ) {
    std::cout << "Usage: To test in serial: \n\t" << argv[0] 
	      << " <parameter file> <winname>\n"
	      << std::endl;
    exit(-1);
  }

  const char *FILE_in  = argv[1];
  const string winname(argv[2]);

  COM_set_profiling(1);
  COM_set_verbose(10);

  COM_LOAD_MODULE_STATIC_DYNAMIC( SimIN, "IN");
  COM_LOAD_MODULE_STATIC_DYNAMIC( SimOUT, "OUT");

  //===== Read in parameter file using Rocin
  int IN_param = COM_get_function_handle( "IN.read_parameter_file");

  COM_call_function(IN_param, FILE_in, winname.c_str());
  int PARAM_all = COM_get_dataitem_handle ((winname+".all").c_str());

  COM_window_init_done(winname.c_str());

  //===== Write out using Rocout
  int OUT_write = COM_get_function_handle( "OUT.write_dataitem");
  COM_call_function( OUT_write, winname.c_str(), 
 		     &PARAM_all,winname.c_str(), "000");

  COM_print_profile("", "");
  COM_finalize();
}



