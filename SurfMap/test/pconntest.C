//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "com.h"
#include "mapbasic.h"
#include <iostream>

COM_EXTERN_MODULE( SimIN);
COM_EXTERN_MODULE( SimOUT);
COM_EXTERN_MODULE( SurfMap);

using namespace std;

int main(int argc, char *argv[]) {
  COM_init( &argc, &argv);

  if ( argc < 3) {
    std::cout << "Usage: " << argv[0] 
	      << " <in_hdf_file> <out_hdf_file> " << endl;
    exit(-1);
  }

  std::cout << "Reading mesh file \"" << argv[1] << '"' << endl;

  std::string fname(argv[1]), wname;
  string::size_type n0 = fname.find_last_of( "/");

  if ( n0 != std::string::npos) 
    fname = fname.substr( n0+1, fname.size());

  string::size_type ni;
  ni = fname.find_first_of( ".:-*[]?\\\"\'0123456789");
  COM_assertion_msg(ni, "File name must start with a letter");

  if ( ni == std::string::npos) {
    wname = fname;
  }
  else {
    while (fname[ni-1]=='_') --ni; // Remove the '_' at the end.
    wname = fname.substr( 0, ni);
  }

  std::cout << "Creating window \"" << wname << '"' << endl;

  // Read in HDF format
  COM_LOAD_MODULE_STATIC_DYNAMIC( SimIN, "IN");
  std::cout << "Reading window " << endl;
  int IN_read = COM_get_function_handle( "IN.read_window");
  COM_call_function( IN_read, argv[1], wname.c_str());
  int IN_obtain = COM_get_function_handle( "IN.obtain_dataitem");
  int mesh_hdl = COM_get_dataitem_handle((wname+".mesh").c_str());
  std::cout << "Obtaining the mesh " << endl;
  COM_call_function( IN_obtain, &mesh_hdl, &mesh_hdl);

  // Change the memory layout to contiguous.
  std::cout << "Resizing the array " << endl;
  COM_resize_array( (wname+".mesh").c_str(), 0, NULL, 0);
  // Delete all dataitems and leave alone the mesh.
  std::cout << "deleting the dataitem" << endl;
  COM_delete_dataitem( (wname+".atts").c_str());

  int npanes; COM_get_panes( wname.c_str(), &npanes, NULL);

  COM_assertion( npanes>=0);

  std::cout << "finishing up window initialization" << endl;
  COM_window_init_done( wname.c_str());

  std::cout << "loading Rocout" << endl;
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");

  std::cout << "Computing connectivity map... " << endl;

  COM_LOAD_MODULE_STATIC_DYNAMIC(SurfMap, "MAP");

  // Invoke compute_pconn to obtain pane connectivity.
  int MAP_compute_pconn = COM_get_function_handle( "MAP.compute_pconn");
  const string pconn = wname+".pconn";
  int pconn_hdl = COM_get_dataitem_handle( pconn.c_str());
  COM_call_function( MAP_compute_pconn, &mesh_hdl, &pconn_hdl);

  std::cout << "Output window into file..." << endl;

  // Output pconn
  int OUT_set = COM_get_function_handle( "OUT.set_option");
  int OUT_write = COM_get_function_handle( "OUT.write_dataitem");

  COM_call_function( OUT_set, "mode", "w");
  int all_hdl = COM_get_dataitem_handle( (wname+".all").c_str());
  COM_call_function( OUT_write, argv[2], &all_hdl, 
		     (char*)wname.c_str(), "000");
  
  COM_finalize();
}



