//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <iostream>
#include "IM_Reader.h"
#include "com.h"
#include "gtest/gtest.h"
#include "surfbasic.h"

COM_EXTERN_MODULE(SimOUT)
COM_EXTERN_MODULE(SurfUtil)
COM_EXTERN_MODULE(Simpal)

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}

TEST(SurfUtilTests, SerializeMesh) {
  COM_init(&ARGC, &ARGV);

  /* if ( ARGC != 3) {
    std::cout << "Usage:\n\tTo run in serial: " << ARGV[0]
              << " <surffile> <hdffile> " << std::endl;
    std::cout << "\n\tTo run in parallel: <mpirun-command> " << ARGV[0]
              << " -com-mpi <Rocin control file> <hdfoutput-prefix> " <<
  std::endl; exit(-1);
  } */

  std::cout << "Reading surface mesh file \"" << ARGV[1] << '"' << std::endl;
  std::cout << ARGV[2] << std::endl;

  std::string fname(ARGV[1]), wname;
  std::string::size_type n0 = fname.find_last_of('/');

  if (n0 != std::string::npos) fname = fname.substr(n0 + 1, fname.size());

  std::string::size_type ni;
  ni = fname.find_first_of(".:-*[]?\\\"\'0123456789");
  COM_assertion_msg(ni, "File name must start with a letter");

  if (ni == std::string::npos) {
    wname = fname;
  } else {
    while (fname[ni - 1] == '_') --ni;  // Remove the '_' at the end.
    wname = fname.substr(0, ni);
  }

  std::cout << "Creating window \"" << wname << '"' << std::endl;

  // Read in IM/HDF format
#ifndef NDEBUG
  int err =
#endif
      IM_Reader().read_winmesh(ARGV[1], wname);
  COM_assertion(err >= 0);

  COM_window_init_done(wname.c_str());

  std::string newwin("newwin");
  COM_new_window(newwin.c_str());
  COM_window_init_done(newwin.c_str());

  int pmesh_hdl = COM_get_dataitem_handle((wname + ".pmesh").c_str());

  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");
  COM_LOAD_MODULE_STATIC_DYNAMIC(SurfUtil, "SURF");

  int SURF_init = COM_get_function_handle("SURF.initialize");
  COM_call_function(SURF_init, &pmesh_hdl);

  int newmesh_hdl = COM_get_dataitem_handle((newwin + ".mesh").c_str());

  std::cout << "Serialize mesh..." << std::endl;
  int SURF_serialize = COM_get_function_handle("SURF.serialize_mesh");
  COM_call_function(SURF_serialize, &pmesh_hdl, &newmesh_hdl);

  std::cout << "Output window into file..." << std::endl;

  // Output normals
  int OUT_set = COM_get_function_handle("OUT.set_option");
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");

  COM_call_function(OUT_set, "mode", "w");
  int all_hdl = COM_get_dataitem_handle((newwin + ".all").c_str());
  COM_call_function(OUT_write, ARGV[2], &all_hdl,
                    reinterpret_cast<const char*>(wname.c_str()), "000");

  COM_finalize();
}
