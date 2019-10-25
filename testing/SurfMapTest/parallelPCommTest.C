//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//
#include <iostream>
#include "Rocin.h"
#include "com.h"
#include "gtest/gtest.h"
#include "mapbasic.h"

COM_EXTERN_MODULE(SimIN)
COM_EXTERN_MODULE(SimOUT)

// Global variables used to pass arguments to the tests
char **ARGV;
int ARGC;

int get_comm_rank(MPI_Comm comm) {
  int rank;
  int ierr = MPI_Comm_rank(comm, &rank);
  assert(ierr == 0);
  return rank;
}
int get_comm_size(MPI_Comm comm) {
  int size;
  int ierr = MPI_Comm_size(comm, &size);
  assert(ierr == 0);
  return size;
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}

TEST(PCommTest, ParallelTest) {
  MPI_Init(&ARGC, &ARGV);

  MPI_Comm comm = MPI_COMM_WORLD;
  int size_of_p = get_comm_size(comm);
  int myrank = get_comm_rank(comm);
  EXPECT_EQ(4, size_of_p) << "Expected thread size is 4\n";
  EXPECT_LT(myrank, 4) << "My rank should be less than 4\n";
  if (myrank == 0)
    std::cout << "myrank = " << myrank << " and size_of_p = " << size_of_p
              << std::endl;

  COM_init(&ARGC, &ARGV);
  COM_set_verbose(11);
  COM_set_profiling(1);

  // Read in CGNS format
  // COM_load_module( "SimIN", "IN");
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN"));

  /* std::string fname("surf000");
  if(myrank==0)
    fname += "0.cgns";
  else
    fname += "1.cgns";
  if (myrank ==0)
    std::cout << "Reading file " << fname << std::endl;

  COM_call_function( IN_read, ("test/"+fname).c_str(), wname.c_str(), &comm);*/
  std::stringstream ss;
  ss << (myrank + 1);
  std::string finame(ARGV[1]);
  std::string foname(ARGV[2]);
  /* std::size_t pos = fname.find_last_of(".");
  fname = fname.substr(0,pos); */
  finame = finame + ss.str() + ".cgns";
  foname = foname + ss.str() + ".cgns";

  const char *file_in = finame.c_str();
  const char *file_out = foname.c_str();

  std::string wname("surf");
  if (myrank == 0) {
    std::cout << "Reading window file \"" << ARGV[1] << '"' << std::endl;
    std::cout << "Creating window \"" << wname << '"' << std::endl;
    std::cout << "Loading Rocin" << std::endl;
  }

  int IN_read = COM_get_function_handle("IN.read_window");
  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  ASSERT_NE(-1, IN_obtain) << "Function IN.obtain_dataitem not found!\n";
  ASSERT_NO_THROW(COM_call_function(IN_read, file_in, wname.c_str(), &comm));

  if (myrank == 0) std::cout << "Obtaining the mesh " << std::endl;

  int mesh_hdl = COM_get_dataitem_handle((wname + ".mesh").c_str());
  ASSERT_NE(-1, mesh_hdl) << "Dataitem " << wname << ".mesh was not found!\n";
  ASSERT_NO_THROW(COM_call_function(IN_obtain, &mesh_hdl, &mesh_hdl));

  // Change the memory layout to contiguous.
  // if (myrank ==0)
  //  std::cout << "Resizing the dataitem arrays " << std::endl;
  // ASSERT_NO_THROW( COM_resize_array( (wname+".all").c_str(), 0, NULL, 0) );

  int npanes = -1;
  int *pane_ids;
  COM_get_panes(wname.c_str(), &npanes, &pane_ids);
  ASSERT_GT(npanes, 0) << "Panes must be greater than 0!\n";
  COM_assertion(npanes >= 0);
  if (myrank == 0) {
    std::cout << npanes << " panes found" << std::endl;
    std::cout << "Labeling nodes with pane ids" << std::endl;
  }
  EXPECT_NO_THROW(
      COM_new_dataitem("surf.pane_ids", 'n', COM_FLOAT, 1, "empty"));
  EXPECT_NO_THROW(COM_resize_array("surf.pane_ids"));

  int nitems;
  float *ptr;
  for (int j = 0; j < npanes; ++j) {
    COM_get_size("surf.pane_ids", pane_ids[j], &nitems);
    COM_get_array("surf.pane_ids", pane_ids[j], &ptr);
    for (int k = 0; k < nitems; ++k) {
      ptr[k] = pane_ids[j];
    }
  }
  int pid_hdl = COM_get_dataitem_handle("surf.pane_ids");
  EXPECT_NE(-1, pid_hdl) << "Dataitem surf.pane_ids was not found!\n";

  if (myrank == 0) std::cout << "Loading Rocmap" << std::endl;
  // COM_load_module( "SurfMap", "MAP");
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfMap, "MAP"));

  if (myrank == 0)
    std::cout << "Performing an average-reduction on shared nodes."
              << std::endl;
  int MAP_average_shared =
      COM_get_function_handle("MAP.reduce_average_on_shared_nodes");
  EXPECT_NE(-1, MAP_average_shared)
      << "Function MAP.reduce_average_on_shared_nodes was not found!\n";
  ASSERT_NO_THROW(COM_call_function(MAP_average_shared, &pid_hdl));

  if (myrank == 0) std::cout << "Updating ghost nodes." << std::endl;
  int MAP_update_ghost = COM_get_function_handle("MAP.update_ghosts");
  EXPECT_NE(-1, MAP_update_ghost)
      << "Function MAP.update_ghosts was not found!\n";
  ASSERT_NO_THROW(COM_call_function(MAP_update_ghost, &pid_hdl));

  if (myrank == 0)
    std::cout << "finishing up window initialization" << std::endl;
  EXPECT_NO_THROW(COM_window_init_done(wname.c_str()));

  if (myrank == 0) std::cout << "loading Rocout" << std::endl;
  // COM_load_module("SimOUT", "OUT");
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));

  const std::string pconn = wname + ".pconn";

  // Output smoothed mesh
  // std::string newfile = "smoothed" + fname;
  if (myrank == 0)
    std::cout << "Outputting window into file " << foname << std::endl;
  int OUT_set = COM_get_function_handle("OUT.set_option");
  EXPECT_NE(-1, OUT_set) << "Function OUT.set_option was not found!\n";
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  EXPECT_NE(-1, OUT_write) << "Function OUT.write_dataitem was not found!\n";
  ASSERT_NO_THROW(COM_call_function(OUT_set, "mode", "w"));
  int all_hdl = COM_get_dataitem_handle((wname + ".all").c_str());
  EXPECT_NE(-1, all_hdl) << "Dataitem " << wname << ".all was not found!\n";
  ASSERT_NO_THROW(
      COM_call_function(OUT_write, file_out, &all_hdl,
                        reinterpret_cast<const char *>(wname.c_str()), ""));

  COM_finalize();
  MPI_Finalize();
}
