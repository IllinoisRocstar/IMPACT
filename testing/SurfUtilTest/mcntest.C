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

using namespace std;

COM_EXTERN_MODULE(SimOUT)
COM_EXTERN_MODULE(SurfUtil)
COM_EXTERN_MODULE(Simpal)

// Global variables used to pass command line arguments to the test
char** ARGV;
int ARGC;

// function which runs the heart of the test
void meanCurvatureNormals(int argc, char** argv) {
  COM_init(&argc, &argv);

  std::cout << "Reading surface mesh file \"" << argv[1] << '"' << endl;

  std::string fname(argv[1]), wname;
  string::size_type n0 = fname.find_last_of("/");

  if (n0 != std::string::npos) fname = fname.substr(n0 + 1, fname.size());

  string::size_type ni;
  ni = fname.find_first_of(".:-*[]?\\\"\'0123456789");
  COM_assertion_msg(ni, "File name must start with a letter");

  if (ni == std::string::npos) {
    wname = fname;
  } else {
    while (fname[ni - 1] == '_') --ni;  // Remove the '_' at the end.
    wname = fname.substr(0, ni);
  }

  std::cout << "Creating window \"" << wname << '"' << endl;

  // Read in IM/HDF format
  int err = IM_Reader().read_winmesh(argv[1], wname);
  // std::cout << "err is equal to: " << err << std::endl;
  COM_assertion(err >= 0);

  // Allocate memory for normals
  const string mcn = wname + ".mcn";
  COM_new_dataitem(mcn.c_str(), 'n', COM_DOUBLE, 3, "");
  COM_resize_array(mcn.c_str());
  // std::cout << "mcn is equal to: " << mcn << std::endl;

  const string lbmcn = wname + ".lbmcn";
  COM_new_dataitem(lbmcn.c_str(), 'n', COM_DOUBLE, 3, "");
  COM_resize_array(lbmcn.c_str());
  // std::cout << "lbmcn is equal to: " << lbmcn << std::endl;

  COM_window_init_done(wname.c_str());
  int pmesh_hdl = COM_get_dataitem_handle((wname + ".pmesh").c_str());
  // std::cout << "pmesh_hdl is equal to: " << pmesh_hdl << std::endl;

  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");
  COM_LOAD_MODULE_STATIC_DYNAMIC(SurfUtil, "SURF");

  int SURF_init = COM_get_function_handle("SURF.initialize");
  COM_call_function(SURF_init, &pmesh_hdl);
  // std::cout << "SURF_init is equal to: " << SURF_init << std::endl;

  std::cout << "Computing mean-curvature normals..." << endl;

  int SURF_mcn = COM_get_function_handle("SURF.compute_mcn");
  int mcn_hdl = COM_get_dataitem_handle(mcn.c_str());
  int lbmcn_hdl = COM_get_dataitem_handle(lbmcn.c_str());
  // std::cout << "SURF_mcn is equal to: " << SURF_mcn << std::endl;
  // std::cout << "mcn_hdl is equal to: " << mcn_hdl << std::endl;
  // std::cout << "lbmcn_hdl is equal to: " << lbmcn_hdl << std::endl;

  COM_call_function(SURF_mcn, &mcn_hdl, &lbmcn_hdl);

  std::cout << "Output window into file..." << argv[2] << endl;

  // Output normals
  int OUT_set = COM_get_function_handle("OUT.set_option");
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  // std::cout << "OUT_set is equal to: " << OUT_set << std::endl;
  // std::cout << "OUT_write is equal to: " << OUT_write << std::endl;

  COM_call_function(OUT_set, "mode", "w");
  int all_hdl = COM_get_dataitem_handle((wname + ".all").c_str());
  // std::cout << "all_hdl is set to: " << all_hdl << std::endl;
  COM_call_function(OUT_write, argv[2], &all_hdl, (char*)wname.c_str(), "000");

  COM_finalize();
}

TEST(MCNTest, GeneralMCNTest) { meanCurvatureNormals(ARGC, ARGV); }

// GTest main method
int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  /*/if ( argc != 3) {
    EXPECT_EQ(argc,3)<< "Usage:\n\tTo run in serial: " << argv[0] \
              << " <surffile> <hdffile> \n" << "\n\tTo run in parallel:
  <mpirun-command> " << argv[0] \
              << " -com-mpi <Rocin control file> <hdfoutput-prefix> " << endl;
    return -1;
  }*/
  ARGC = argc;
  ARGV = argv;
  cout << "Reading surface mesh file \"" << argv[1] << '"' << endl;
  return RUN_ALL_TESTS();
}
