//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "com.h"
#include "commpi.h"
#include "gtest/gtest.h"
#include "meshio.C"

COM_EXTERN_MODULE(SurfX)
COM_EXTERN_MODULE(SimOUT)

// Global variables used to pass arguments to the tests
char **ARGV;
int ARGC;

using namespace std;

TEST(SurfXTests, TriToTriRefinement) {
  COM_init(&ARGC, &ARGV);

  /* if ( argc < 7) {
    std::cout << "Usage: " << argv[0]
              << " <t1_prefix> <t1_num_blocks> <t1_suffix> <t2_prefix>"
              << " <t2_num_blocks> <t2_suffix>" << std::endl;
    exit(-1);
  } */

  COM_set_verbose(1);
  COM_set_profiling(1);

  const char* format;
  if(!std::strcmp(ARGV[7], "HDF")){
    format = "HDF";
  } else {
    format = "CGNS";
  }

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfX, "RFC"));

  MPI_Comm comm = MPI_COMM_WORLD;
  const int comm_rank = 0;
  const int comm_size = 1;

  const char *t1_prefix = ARGV[1];
  const int t1_num_blocks = atoi(ARGV[2]);
  const char *t1_suffix = ARGV[3];
  assert(t1_num_blocks <= 5);

  std::vector<double> t1_mesh_coors[5];
  std::vector<int> t1_mesh_elems[5];

  for (int i = 0; i < t1_num_blocks; ++i)
    if (i % comm_size == comm_rank) {
      char fname[100];
      std::sprintf(fname, "%s%d%s", t1_prefix, i + 1, t1_suffix);
      std::ifstream is(fname);
      assert(is.is_open());
      read_obj(is, t1_mesh_coors[i], t1_mesh_elems[i]);
    }

  EXPECT_NO_THROW(COM_new_window("tri1"))
      << "An error occurred when reating window tri1\n";
  for (int i = 0; i < t1_num_blocks; ++i)
    if (i % comm_size == comm_rank) {
      EXPECT_NO_THROW(
          COM_set_size("tri1.nc", i + 1, t1_mesh_coors[i].size() / 3))
          << "An error occurred when setting the size of the coordinates "
             "array\n";
      EXPECT_NO_THROW(COM_set_array("tri1.nc", i + 1, &t1_mesh_coors[i][0]))
          << "An error occurred when setting the coordinates array\n";
      EXPECT_NO_THROW(
          COM_set_size("tri1.:t3:", i + 1, t1_mesh_elems[i].size() / 3))
          << "An error occurred when setting the size of the elements array\n";
      EXPECT_NO_THROW(COM_set_array("tri1.:t3:", i + 1, &t1_mesh_elems[i][0]))
          << "An error occurred when setting the elements array\n";
    }
  COM_window_init_done("tri1");

  const char *t2_prefix = ARGV[4];
  const int t2_num_blocks = atoi(ARGV[5]);
  const char *t2_suffix = ARGV[6];
  assert(t2_num_blocks <= 5);

  std::vector<double> t2_mesh_coors[5];
  std::vector<int> t2_mesh_elems[5];

  for (int i = 0; i < t2_num_blocks; ++i)
    if (i % comm_size == comm_rank) {
      char fname[100];
      std::sprintf(fname, "%s%d%s", t2_prefix, i + 1, t2_suffix);
      std::ifstream is(fname);
      assert(is.is_open());
      read_obj(is, t2_mesh_coors[i], t2_mesh_elems[i]);
    }
  COM_new_window("tri2");
  for (int i = 0; i < t2_num_blocks; ++i)
    if (i % comm_size == comm_rank) {
      COM_set_size("tri2.nc", i + 1, t2_mesh_coors[i].size() / 3);
      COM_set_array("tri2.nc", i + 1, &t2_mesh_coors[i][0]);
      COM_set_size("tri2.:t3:", i + 1, t2_mesh_elems[i].size() / 3);
      COM_set_array("tri2.:t3:", i + 1, &t2_mesh_elems[i][0]);
    }
  COM_window_init_done("tri2");

  int tri1_mesh = COM_get_dataitem_handle("tri1.mesh");
  int tri2_mesh = COM_get_dataitem_handle("tri2.mesh");
  int RFC_clear = COM_get_function_handle("RFC.clear_overlay");
  int RFC_write = COM_get_function_handle("RFC.write_overlay");

  if (comm_size == 1) {
    int RFC_overlay = COM_get_function_handle("RFC.overlay");

    EXPECT_NO_THROW(
        COM_call_function(RFC_overlay, &tri1_mesh, &tri2_mesh, &comm))
        << "An error occurred while performing the SurfX overlay\n";
    EXPECT_NO_THROW(COM_call_function(RFC_write, &tri1_mesh, &tri2_mesh,
                                      "TriToTriTest1", "TriToTriTest2", format))
        << "An error occurred while writing the overlay data\n";
    EXPECT_NO_THROW(COM_call_function(RFC_clear, "tri1", "tri2"))
        << "An error occurred "
        << "while clearing the overlay data\n";
  }

  int RFC_read = COM_get_function_handle("RFC.read_overlay");
  EXPECT_NO_THROW(COM_call_function(RFC_read, &tri1_mesh, &tri2_mesh, &comm,
                                    "TriToTriTest1", "TriToTriTest2", format))
      << "An error occurred while reading the overlay data\n";

  char prefix1[100], prefix2[100];
  std::sprintf(prefix1, "tri1_%d", comm_rank);
  std::sprintf(prefix2, "tri2_%d", comm_rank);

  EXPECT_NO_THROW(COM_call_function(RFC_clear, "tri1", "tri2"))
      << "An error occurred while"
      << "clearing the overlay data\n";

  COM_delete_window("tri1");
  COM_delete_window("tri2");

  COM_print_profile("", "");

  COM_finalize();
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}