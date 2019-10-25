//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <iostream>
#include <sstream>
#include "com.h"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

COM_EXTERN_MODULE(SurfMap)
COM_EXTERN_MODULE(SimOUT)

// ==== Routines for creating mesh information
// void init_structure_mesh( double coors_s[3][10][6][5]) {
void init_structure_mesh(double coors_s[5][6][10][3]) {
  // void init_structure_mesh( double coors_s[3][5][6][10]) {

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 6; j++) {
      for (int k = 0; k < 5; k++) {
        coors_s[k][j][i][0] = i * .02;
        coors_s[k][j][i][1] = j * .02;
        coors_s[k][j][i][2] = k * .02;
      }
    }
  }
}

bool is_local(int pid, int comm_rank, int comm_size) {
  return (pid % comm_size == comm_rank);
}

TEST(SurfMap, StrucBorderTest) {
  const int ni = 10, nj = 6, nk = 5;
  int total = ni * nj * nk;
  double coors_s[nk][nj][ni][3];
  int dims[3];
  dims[0] = ni;
  dims[1] = nj;
  dims[2] = nk;
  // int     elmts[num_elmts][8];

  COM_init(&ARGC, &ARGV);
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfMap, "MAP"));
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));

  COM_new_window("str");
  COM_new_dataitem("str.borders", 'n', COM_INTEGER, 1, "");

  init_structure_mesh(coors_s);

  EXPECT_NO_THROW(COM_set_array_const("str.:st3:actual", 1, dims))
      << "An error occurred when setting the read only array property!\n";
  EXPECT_NO_THROW(COM_set_size("str.nc", 1, total))
      << "An error occurred when setting the coordinate size!\n";
  EXPECT_NO_THROW(COM_set_array("str.nc", 1, &coors_s[0][0][0][0], 3))
      << "An error occurred when setting the coordinates array!\n";

  COM_resize_array("str.borders");
  COM_window_init_done("str");

  int mesh_hdl = COM_get_dataitem_handle("str.mesh");
  EXPECT_NE(-1, mesh_hdl)
      << "An error occurred when getting the strc mesh handle!\n";
  std::cout << "Get border nodes... " << std::endl;

  int MAP_border_nodes = COM_get_function_handle("MAP.pane_border_nodes");
  EXPECT_NE(-1, MAP_border_nodes)
      << "An error occurred when getting the pane_border_nodes handle!\n";
  int borders_hdl = COM_get_dataitem_handle("str.borders");
  EXPECT_NE(-1, borders_hdl)
      << "An error occurred when getting the borders dataitem handle\n";
  ASSERT_NO_THROW(COM_call_function(MAP_border_nodes, &mesh_hdl, &borders_hdl))
      << "An error occurred when calling MAP.pane_border_nodes!\n";

  // Output solution into file.
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  EXPECT_NE(-1, OUT_write)
      << "An error occurred when getting the OUT.write_dataitem!\n";
  ASSERT_NO_THROW(COM_call_function(OUT_write, "strucmesh", &borders_hdl,
                                    "strucmesh", "000"))
      << "An error occurred when writing out the mesh!\n";
  // COM_call_function( OUT_write, "strucmesh1", &mesh_hdl,
  //"strucmesh1", "000");

  COM_finalize();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}