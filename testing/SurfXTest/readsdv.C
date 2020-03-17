//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

// This file demonstrates how to read in a sdv file.
#include <cstdlib>
#include <iostream>
#include <vector>
#include "com.h"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char **ARGV;
int ARGC;

COM_EXTERN_MODULE(SimIN)

struct Subdiv {
  std::vector<int> subfaces;  // Connectivity of subfaces

  std::vector<int> subnode_parents;       // Parent face ID
  std::vector<float> subnode_ncs;         // Natural coordinates in parent face
  std::vector<int> subnode_subIDs;        // Sub-node ID of input nodes
  std::vector<int> subnode_counterparts;  // Sub-node ID in other window

  std::vector<int> subface_parents;  // Parent face ID
  std::vector<float>
      subface_nat_coors;             // Natural coors. of subnodes of susface
  std::vector<int> subface_offsets;  // Smallest sub-face ID in input face
  std::vector<int> subface_counterparts;  // Sub-face ID in other window
};

// A sample program for reading an output file of Rocface for a single pane.
// Arguments: fname: the file name of the CGNS file
//            prefix: is the window name (also the base material name) in CGNS
//            files pid: the pane ID of the window. sd:  the output structure
//            contained in an Subdiv object.
void read_pane_sp(std::string &fname, const std::string &prefix, int pid,
                  Subdiv &sd) {
  // Load Rocin into Roccom
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "SDV_IN"));

  // Obtain function handles to Rocin functions
  int hdl_read = COM_get_function_handle("SDV_IN.read_windows");
  int hdl_obtain = COM_get_function_handle("SDV_IN.obtain_dataitem");
  EXPECT_NE(-1, hdl_read)
      << "SDV_IN.read_windows function handle was not found!\n";
  EXPECT_NE(-1, hdl_obtain) << "SDV_IN.obtain_dataitem handle was not found!\n";
  // Define the base-window and sdv-window names
  std::string bufprefix = "__BUF";
  std::string sdv_material = prefix + "_sdv";
  std::string sdv_wname = bufprefix + sdv_material;

  // Read the pane from the given file. Note that the file contains both
  // the parent and the subdivided windows. Read only the subdivided one.
  MPI_Comm comm_self = MPI_COMM_SELF;
  ASSERT_NO_THROW(COM_call_function(hdl_read, fname.c_str(), bufprefix.c_str(),
                                    sdv_material.c_str(), &comm_self))
      << "An error occurred when reading from the window!" << std::endl;
  int hdl_all = COM_get_dataitem_handle((sdv_wname + ".all").c_str());
  EXPECT_NE(-1, hdl_all) << sdv_wname
                         << ".all dataitem handle was not found!\n";
  ASSERT_NO_THROW(COM_call_function(hdl_obtain, &hdl_all, &hdl_all, &pid))
      << "An error occurred when obtaining all dataitems from the window!"
      << std::endl;

  // Obtain number of sub-nodes, sub-faces, nodes, and faces
  int nsubn, nsubf, nn, nf;
  EXPECT_NO_THROW(
      COM_get_size((sdv_wname + ".sn_parent_fcID").c_str(), pid, &nsubn))
      << "An error occurred when reading the number of subnodes!\n";
  EXPECT_NO_THROW(COM_get_size((sdv_wname + ".:t3").c_str(), pid, &nsubf))
      << "An error occurred when reading the number of subfaces!\n";
  EXPECT_NO_THROW(COM_get_size((sdv_wname + ".sn_subID").c_str(), pid, &nn))
      << "An error occurred when reading the number of nodes!\n";
  EXPECT_NO_THROW(COM_get_size((sdv_wname + ".sf_offset").c_str(), pid, &nf))
      << "An error occurred when reading the number of faces!\n";

  // Obtain the connectivity
  sd.subfaces.resize(3 * nsubf);
  EXPECT_NO_THROW(
      COM_copy_array((sdv_wname + ".:t3").c_str(), pid, &sd.subfaces[0], 3))
      << "An error occurred when copying the connectivity information!\n";
  // NOTE: The last argument (3) indicates that three sub-node IDs are stored
  // consecutively for each sub-face. Use the number one (1) if the first
  // sub-nodes of all sub-faces are stored together followed by the second
  // sub-nodes and then third.

  // Obtain subnode_parents, subnode_ncs, and subnode_counterparts
  sd.subnode_parents.resize(nsubn);
  EXPECT_NO_THROW(COM_copy_array((sdv_wname + ".sn_parent_fcID").c_str(), pid,
                                 &sd.subnode_parents[0]))
      << "An error occurred when copying the subnode parents!\n";

  sd.subnode_ncs.resize(2 * nsubn);
  EXPECT_NO_THROW(COM_copy_array((sdv_wname + ".sn_parent_ncs").c_str(), pid,
                                 &sd.subnode_ncs[0], 2))
      << "An error occurred when copying the subnode parent nodal "
         "connectivity!\n";
  // NOTE: The last argument (2) indicates that the two local coordinates are
  // stored consecutively. Use the number one (1) if the first local parameter
  // (xi) of all sub-nodes are stored together followed by the second one (eta).

  sd.subnode_subIDs.resize(nn);
  EXPECT_NO_THROW(COM_copy_array((sdv_wname + ".sn_subID").c_str(), pid,
                                 &sd.subnode_subIDs[0]))
      << "An error occurred when copying the subnode ID's!\n";

  sd.subnode_counterparts.resize(nsubn);
  EXPECT_NO_THROW(COM_copy_array((sdv_wname + ".sn_cntpt_ndID").c_str(), pid,
                                 &sd.subnode_counterparts[0]))
      << "An error occurred when copying the subnode counterparts!\n";

  sd.subface_parents.resize(nsubf);
  EXPECT_NO_THROW(COM_copy_array((sdv_wname + ".sf_parent").c_str(), pid,
                                 &sd.subface_parents[0]))
      << "An error occurred when copying the subface parents!\n";

  sd.subface_nat_coors.resize(nsubf * 6);
  EXPECT_NO_THROW(COM_copy_array((sdv_wname + ".sf_ncs").c_str(), pid,
                                 &sd.subface_nat_coors[0], 6))
      << "An error occurred when copying the subface coordinates!\n";
  // NOTE: The last argument (6) indicates that the local coordinates are
  // stored consecutively (xi1, eta1, xi2, eta2, xi3, eta3). Use the number
  // one (1) if the xi1 for all nodes are stored together and then xi2, etc..

  // Obtain subface_offsets
  sd.subface_offsets.resize(nf);
  EXPECT_NO_THROW(COM_copy_array((sdv_wname + ".sf_offset").c_str(), pid,
                                 &sd.subface_offsets[0]))
      << "An error occurred when copying the subface offsets!\n";

  sd.subface_counterparts.resize(nsubf);
  EXPECT_NO_THROW(COM_copy_array((sdv_wname + ".sf_cntpt_fcID").c_str(), pid,
                                 &sd.subface_counterparts[0]))
      << "An error occurred when copying the subface counterparts!\n";

  // Delete the window created by Rocin.
  EXPECT_NO_THROW(COM_delete_window(sdv_wname.c_str()))
      << "An error occurred when deleting the window!\n";

  // Unload Rocin from Roccom.
  EXPECT_NO_THROW(COM_UNLOAD_MODULE_STATIC_DYNAMIC(SimIN, "SDV_IN"))
      << "An error occurred when unloading SimIN\n";
}

TEST(SurfXTest, readSDV) {
  COM_init(&ARGC, &ARGV);

  if (ARGC < 3) {
    std::cerr << "Usage: " << ARGV[0] << " <file-name> <prefix> <pane_id>\n"
              << "Example: " << ARGV[0] << " A_101_sdv.cgns A 101" << std::endl;
    exit(-1);
  }

  std::string fname = ARGV[1], prefix = ARGV[2];
  int pid = std::atoi(ARGV[3]);

  // Reading the given pane from the file into the structure.
  Subdiv sd;
  ASSERT_NO_THROW(read_pane_sp(fname, prefix, pid, sd));

  COM_finalize();
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}