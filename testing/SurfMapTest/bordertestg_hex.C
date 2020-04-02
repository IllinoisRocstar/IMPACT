//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <iostream>
#include <sstream>
#include <cstring>
#include "com.h"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

COM_EXTERN_MODULE(SurfMap)
COM_EXTERN_MODULE(SimOUT)

// ==== Routines for creating mesh information
void init_unstructure_mesh(double coors[24][3], int elmts[6][8]) {
  coors[0][0] = 0.0;
  coors[0][1] = 0.0;
  coors[0][2] = 0.0;
  coors[1][0] = 0.5;
  coors[1][1] = 0.0;
  coors[1][2] = 0.0;
  coors[2][0] = 1.0;
  coors[2][1] = 0.0;
  coors[2][2] = 0.0;
  coors[3][0] = 1.0;
  coors[3][1] = 0.5;
  coors[3][2] = 0.0;
  coors[4][0] = 0.5;
  coors[4][1] = 0.5;
  coors[4][2] = 0.0;
  coors[5][0] = 0.0;
  coors[5][1] = 0.5;
  coors[5][2] = 0.0;
  coors[6][0] = 0.0;
  coors[6][1] = 1.0;
  coors[6][2] = 0.0;
  coors[7][0] = 0.5;
  coors[7][1] = 1.0;
  coors[7][2] = 0.0;
  coors[8][0] = 1.0;
  coors[8][1] = 1.0;
  coors[8][2] = 0.0;
  coors[9][0] = 0.0;
  coors[9][1] = 0.0;
  coors[9][2] = 1.0;
  coors[10][0] = 0.5;
  coors[10][1] = 0.0;
  coors[10][2] = 1.0;
  coors[11][0] = 1.0;
  coors[11][1] = 0.0;
  coors[11][2] = 1.0;
  coors[12][0] = 1.0;
  coors[12][1] = 0.5;
  coors[12][2] = 1.0;
  coors[13][0] = 0.5;
  coors[13][1] = 0.5;
  coors[13][2] = 1.0;
  coors[14][0] = 0.0;
  coors[14][1] = 0.5;
  coors[14][2] = 1.0;
  coors[15][0] = 0.0;
  coors[15][1] = 1.0;
  coors[15][2] = 1.0;
  coors[16][0] = 0.5;
  coors[16][1] = 1.0;
  coors[16][2] = 1.0;
  coors[17][0] = 1.0;
  coors[17][1] = 1.0;
  coors[17][2] = 1.0;
  coors[18][0] = 1.5;
  coors[18][1] = 0.0;
  coors[18][2] = 0.0;
  coors[19][0] = 1.5;
  coors[19][1] = 0.5;
  coors[19][2] = 0.0;
  coors[20][0] = 1.5;
  coors[20][1] = 1.0;
  coors[20][2] = 0.0;
  coors[21][0] = 1.5;
  coors[21][1] = 0.0;
  coors[21][2] = 1.0;
  coors[22][0] = 1.5;
  coors[22][1] = 0.5;
  coors[22][2] = 1.0;
  coors[23][0] = 1.5;
  coors[23][1] = 1.0;
  coors[23][2] = 1.0;

  elmts[0][0] = 1;
  elmts[0][1] = 2;
  elmts[0][2] = 5;
  elmts[0][3] = 6;
  elmts[0][4] = 10;
  elmts[0][5] = 11;
  elmts[0][6] = 14;
  elmts[0][7] = 15;

  elmts[1][0] = 2;
  elmts[1][1] = 3;
  elmts[1][2] = 4;
  elmts[1][3] = 5;
  elmts[1][4] = 11;
  elmts[1][5] = 12;
  elmts[1][6] = 13;
  elmts[1][7] = 14;

  elmts[2][0] = 5;
  elmts[2][1] = 4;
  elmts[2][2] = 9;
  elmts[2][3] = 8;
  elmts[2][4] = 14;
  elmts[2][5] = 13;
  elmts[2][6] = 18;
  elmts[2][7] = 17;

  elmts[3][0] = 6;
  elmts[3][1] = 5;
  elmts[3][2] = 8;
  elmts[3][3] = 7;
  elmts[3][4] = 15;
  elmts[3][5] = 14;
  elmts[3][6] = 17;
  elmts[3][7] = 16;

  elmts[4][0] = 3;
  elmts[4][1] = 19;
  elmts[4][2] = 20;
  elmts[4][3] = 4;
  elmts[4][4] = 12;
  elmts[4][5] = 22;
  elmts[4][6] = 23;
  elmts[4][7] = 13;

  elmts[5][0] = 4;
  elmts[5][1] = 20;
  elmts[5][2] = 21;
  elmts[5][3] = 9;
  elmts[5][4] = 13;
  elmts[5][5] = 23;
  elmts[5][6] = 24;
  elmts[5][7] = 18;
}

TEST(SurfMap, HexBorderGhostTest) {
  const int num_nodes = 24, num_elmts = 6;
  const int ghost_nodes = 6, ghost_elements = 2;

  double coors_s[num_nodes][3];
  int elmts[num_elmts][8];

  COM_init(&ARGC, &ARGV);
  //const char* format = ARGV[1];

  std::cout << "Creating window \"unstr\"" << std::endl;
  COM_new_window("unstr");
  COM_new_dataitem("unstr.normals", 'e', COM_DOUBLE, 3, "m");
  COM_new_dataitem("unstr.centers", 'e', COM_DOUBLE, 3, "m");

  init_unstructure_mesh(coors_s, elmts);
  EXPECT_NO_THROW(COM_set_size("unstr.nc", 1, num_nodes, ghost_nodes))
      << "An error occurred when setting the size of the nodal coordinates!\n";
  EXPECT_NO_THROW(COM_set_array("unstr.nc", 1, &coors_s[0][0]))
      << "An error occurred when setting the nodal cooridate array!\n";
  EXPECT_NO_THROW(COM_set_size("unstr.:H8:", 1, num_elmts, ghost_elements))
      << "An error occurred when setting the size of the Ghost Element "
         "connectivity!\n";
  EXPECT_NO_THROW(COM_set_array("unstr.:H8:", 1, &elmts[0][0]))
      << "An error occurred when setting the Ghost Elements!\n";

  COM_resize_array("unstr.data");

  for (int i = 0; i < num_nodes; ++i) {
    std::cout << "Coors_s[0][" << i << "] = [" << coors_s[i][0] << " , "
              << coors_s[i][1] << " , " << coors_s[i][2] << "]" << std::endl;
  }
  for (int i = 0; i < num_elmts; ++i) {
    std::cout << "Element " << i << ": " << elmts[i][0] << " " << elmts[i][1]
              << " " << elmts[i][2] << " " << elmts[i][3] << " " << elmts[i][4]
              << " " << elmts[i][5] << " " << elmts[i][6] << " " << elmts[i][7]
              << std::endl;
  }

  // Allocate memory for displacements
  const std::string disps("unstr.disps");
  char loc;
  int size, type;
  std::string unit;
  COM_get_dataitem("unstr.nc", &loc, &type, &size, &unit);
  COM_new_dataitem(disps.c_str(), 'n', COM_DOUBLE, 3, unit.c_str());
  COM_resize_array(disps.c_str());
  COM_window_init_done("unstr");
  int mesh_hdl = COM_get_dataitem_handle("unstr.mesh");
  EXPECT_NE(-1, mesh_hdl)
      << "An error occurred when getting the mesh handle!\n";

  COM_LOAD_MODULE_STATIC_DYNAMIC(SurfMap, "MAP");
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");

  int MAP_compute_pconn = COM_get_function_handle("MAP.compute_pconn");
  const std::string pconn = "unstr.pconn";
  int pconn_hdl = COM_get_dataitem_handle(pconn.c_str());
  EXPECT_NE(-1, pconn_hdl)
      << "An error occurred when getting the Pane connectivity"
      << " handle!\n";
  ASSERT_NO_THROW(COM_call_function(MAP_compute_pconn, &mesh_hdl, &pconn_hdl))
      << "An error occurred when computing the pane connectivity!" << std::endl;

  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  EXPECT_NE(-1, OUT_write)
      << "An error occurred when getting the OUT.write_dataitem!\n";
  if(!std::strcmp(ARGV[1], "HDF")){
    ASSERT_NO_THROW(COM_call_function(OUT_write, "GhostHexMesh", &pconn_hdl,
                                    "GhostHexMesh", "000", "HDF"))
      << "An error occurred when writing out the mesh!\n";
  } else {
  ASSERT_NO_THROW(COM_call_function(OUT_write, "GhostHexMesh", &pconn_hdl,
                                    "GhostHexMesh", "000"))
      << "An error occurred when writing out the mesh!\n";
  }

  COM_finalize();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}