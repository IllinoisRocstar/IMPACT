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

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

COM_EXTERN_MODULE(SimIN)
COM_EXTERN_MODULE(SimOUT)
COM_EXTERN_MODULE(SurfMap)
COM_EXTERN_MODULE(SurfUtil)

TEST(SurfUtilTest, ComputeFaceNormals) {
  COM_init(&ARGC, &ARGV);

  if (ARGC < 3) {
    std::cout << "Usage:\n\tTo run in serial: " << ARGV[0]
              << " <surffile> <hdffile> " << std::endl;
    std::cout << "\n\tTo run in parallel: <mpirun-command> " << ARGV[0]
              << " -com-mpi <Rocin control file> <hdfoutput-prefix> "
              << std::endl;
    exit(-1);
  }

  std::cout << "Reading surface mesh file \"" << ARGV[1] << '"' << std::endl;

  std::string fname(ARGV[1]), wname;
  std::string::size_type n0 = fname.find_last_of("/");

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
  int err = IM_Reader().read_winmesh(ARGV[1], wname);
  COM_assertion(err >= 0);

  std::cout << "finishing up window initialization" << std::endl;
  EXPECT_NO_THROW(
      COM_new_dataitem((wname + ".normals").c_str(), 'e', COM_DOUBLE, 3, ""))
      << "An error occurred when declaring the face normals dataitem!\n";
  EXPECT_NO_THROW(COM_resize_array((wname + ".normals").c_str()))
      << "An error occurred when resizing the face normals array!\n";
  EXPECT_NO_THROW(COM_window_init_done(wname.c_str()))
      << "An error occurred when finishing initializing the window " << wname
      << "\n";

  std::cout << "loading Rocout" << std::endl;
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"))
      << "An error occurred when loading SimOUT!" << std::endl;

  std::cout << "Computing pane connectivity... " << std::endl;

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfMap, "MAP"))
      << "An error occurred when loading SurfMap!" << std::endl;

  // Invoke compute_pconn to obtain pane connectivity.
  int MAP_compute_pconn = COM_get_function_handle("MAP.compute_pconn");
  EXPECT_NE(-1, MAP_compute_pconn)
      << "MAP.compute_pconn handle was not found!\n";
  const std::string pconn = wname + ".pconn";

  int mesh_hdl = COM_get_dataitem_handle((wname + ".mesh").c_str());
  EXPECT_NE(-1, mesh_hdl) << wname << ".mesh dataitem handle was not found!\n";
  int pconn_hdl = COM_get_dataitem_handle(pconn.c_str());
  EXPECT_NE(-1, pconn_hdl) << pconn << " dataitem handle was not found!\n";
  ASSERT_NO_THROW(COM_call_function(MAP_compute_pconn, &mesh_hdl, &pconn_hdl))
      << "An error occurred when computing the pane connectivity!" << std::endl;

  std::cout << "Computing face normals... " << std::endl;
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfUtil, "SURF"))
      << "An error occurred when loading SurfUtil!" << std::endl;

  int SURF_normal = COM_get_function_handle("SURF.compute_element_normals");
  EXPECT_NE(-1, SURF_normal)
      << "SURF.compute_elements_normals function handle was not found!\n";
  int nrmls_hdl = COM_get_dataitem_handle((wname + ".normals").c_str());
  EXPECT_NE(-1, nrmls_hdl) << wname
                           << ".normals dataitem handle was not found!\n";
  ASSERT_NO_THROW(COM_call_function(SURF_normal, &nrmls_hdl))
      << "An error occurred"
      << " when computing the surface normals!" << std::endl;

  std::cout << "Output window into file..." << std::endl;

  // Output pconn
  int OUT_set = COM_get_function_handle("OUT.set_option");
  EXPECT_NE(-1, OUT_set) << "OUT.set_option function handle was not found!\n";
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  EXPECT_NE(-1, OUT_write)
      << "OUT.write_dataitem function handle was not found!\n";

  ASSERT_NO_THROW(COM_call_function(OUT_set, "mode", "w"))
      << "An error occured when attempting"
      << " to write out the results" << std::endl;
  int all_hdl = COM_get_dataitem_handle((wname + ".all").c_str());
  EXPECT_NE(-1, all_hdl) << wname << ".all dataitem handle was not found!\n";
  ASSERT_NO_THROW(
      COM_call_function(OUT_write, ARGV[2], &all_hdl,
                        reinterpret_cast<const char*>(wname.c_str()), "000"))
      << "An error occurred when writing out the results!" << std::endl;

  COM_finalize();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}