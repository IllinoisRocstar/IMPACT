//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <iostream>
#include "com.h"
#include "gtest/gtest.h"
#include "mapbasic.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

using namespace std;

TEST(PCommTest, readFromWindow) {
  COM_init(&ARGC, &ARGV);

  /* if ( ARGC < 2) {
    cout << "Usage: " << argv[0]
              << "<in_cgns_file> <out_cgns_file> " << endl;
    exit(-1);
  } */

  cout << "Reading mesh file \"" << ARGV[1] << '"' << endl;

  string wname("surf");
  string fname(ARGV[1]);
  string::size_type n0 = fname.find_last_of("/");

  if (n0 != std::string::npos) fname = fname.substr(n0 + 1, fname.size());

  string::size_type ni;
  ni = fname.find_first_of(".:-*[]?\\\"\'0123456789");
  COM_assertion_msg(ni, "File name must start with a letter");

  /* if ( ni == std::string::npos) {
    wname = fname;
  }
  else {
    while (fname[ni-1]=='_') --ni; // Remove the '_' at the end.
    wname = fname.substr( 0, ni);
  } */

  cout << "Creating window \"" << wname << '"' << endl;

  // Read in CGNS format
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN"));
  cout << "Reading window " << endl;
  int IN_read = COM_get_function_handle("IN.read_window");
  EXPECT_NE(-1, IN_read) << "Function IN.read_window not found!\n";
  ASSERT_NO_THROW(COM_call_function(IN_read, ARGV[1], wname.c_str()));
  /* COM_call_function( IN_read,
                     "test/surf000[01].hdf",
                     wname.c_str()); */

  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  ASSERT_NE(-1, IN_obtain) << "Function IN.obtain_dataitem was not found!\n";
  int mesh_hdl = COM_get_dataitem_handle((wname + ".mesh").c_str());
  ASSERT_NE(-1, mesh_hdl) << "Dataitem " << wname << ".mesh was not found!\n";
  cout << "Obtaining the mesh " << endl;
  ASSERT_NO_THROW(COM_call_function(IN_obtain, &mesh_hdl, &mesh_hdl));

  // Change the memory layout to contiguous.
  cout << "Resizing the dataitem arrays " << endl;
  ASSERT_NO_THROW(COM_resize_array((wname + ".all").c_str(), 0, NULL, 0));

  // Delete all dataitems and leave alone the mesh.
  cout << "deleting the dataitem" << endl;
  ASSERT_NO_THROW(COM_delete_dataitem((wname + ".data").c_str()));

  int npanes;
  int* pane_ids;
  COM_get_panes(wname.c_str(), &npanes, &pane_ids);
  ASSERT_GT(npanes, 0) << "Number of panes should be greater than 0!\n";
  COM_assertion(npanes >= 0);

  cout << "Labeling nodes with pane ids" << endl;
  ASSERT_NO_THROW(
      COM_new_dataitem("surf.pane_ids", 'n', COM_FLOAT, 1, "empty"));
  ASSERT_NO_THROW(COM_resize_array("surf.pane_ids"));

  int nitems;
  float* ptr;
  for (int j = 0; j < npanes; ++j) {
    ASSERT_NO_THROW(COM_get_size("surf.pane_ids", pane_ids[j], &nitems));
    cout << "  pane[" << j << "] has " << nitems << " items " << endl;
    ASSERT_NO_THROW(COM_get_array("surf.pane_ids", pane_ids[j], &ptr));
    for (int k = 0; k < nitems; ++k) {
      ptr[k] = pane_ids[j];
    }
  }
  int pid_hdl = COM_get_dataitem_handle("surf.pane_ids");
  ASSERT_NE(-1, pid_hdl) << "Dataitem surf.pane_ids not found!\n";

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfMap, "MAP"));

  cout << "Performing an average-reduction on shared nodes." << endl;
  int MAP_average_shared =
      COM_get_function_handle("MAP.reduce_average_on_shared_nodes");
  ASSERT_NE(-1, MAP_average_shared)
      << "Function MAP.reduce_average_on_shared_nodes not found!\n";
  ASSERT_NO_THROW(COM_call_function(MAP_average_shared, &pid_hdl));

  cout << "Updating ghost nodes." << endl;
  int MAP_update_ghost = COM_get_function_handle("MAP.update_ghosts");
  ASSERT_NE(-1, MAP_update_ghost)
      << "Function MAP.update_ghosts was not found!\n";
  ASSERT_NO_THROW(COM_call_function(MAP_update_ghost, &pid_hdl));

  cout << "Finalizing the window" << endl;
  ASSERT_NO_THROW(COM_window_init_done(wname.c_str()));

  cout << "Loading Rocout" << endl;
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));

  cout << "Output window into file..." << endl;

  // Output new mesh
  int OUT_set = COM_get_function_handle("OUT.set_option");
  ASSERT_NE(-1, OUT_set) << "Function OUT.set_option was not found!\n";
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  ASSERT_NE(-1, OUT_write) << "Function OUT.write_dataitem\n";

  ASSERT_NO_THROW(COM_call_function(OUT_set, "mode", "w"));
  int all_hdl = COM_get_dataitem_handle((wname + ".all").c_str());
  ASSERT_NE(-1, all_hdl) << "Dataitem " << wname << ".all was not found!\n";
  COM_call_function(OUT_write, ARGV[1], &all_hdl, (char*)wname.c_str(), "0000");

  COM_finalize();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}