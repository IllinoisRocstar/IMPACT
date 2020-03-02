//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** A test program that reads in given input CGNS files using Rocin and
 *  write out  all the contents into one CGNS file using Rocout with
 *  a given file prefix.
 **/
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include "COM_base.hpp"
#include "Rocin.h"
#include "com.h"
#include "com_devel.hpp"
#include "gtest/gtest.h"

COM_EXTERN_MODULE(SimIN)
COM_EXTERN_MODULE(SimOUT)

// Global variables used to pass arguments to the tests
char **ARGV;
int ARGC;

using namespace std;

TEST(Intest, ParallelRead1FromControlFile) {
  COM_init(&ARGC, &ARGV);

  COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");

  COM_set_verbose(11);
  COM_set_profiling(1);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  stringstream ss;
  ss << rank;
  string fileName(ARGV[3]);
  std::size_t pos = fileName.find_last_of('.');
  fileName = fileName.substr(0, pos);
  fileName = fileName + ss.str() + ".cgns";

  const char *file_in = ARGV[1];
  const char *file_out = fileName.c_str();

  int IN_read;
  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  EXPECT_NE(-1, IN_obtain) << "function: obtain_dataitem not found!\n";

  const char *lastdot = std::strrchr(file_in, '.');
  if (lastdot && std::strcmp(lastdot, ".txt") == 0) {
    IN_read = COM_get_function_handle("IN.read_by_control_file");
    EXPECT_NE(-1, IN_read) << "Function: IN.read_by_control_file not found!\n";
  } else {
    FAIL() << "Non-Control file provided for a control file read test!\n";
    // IN_read = COM_get_function_handle( "IN.read_window");
  }

  const char *win_in = "rocin_win";
  const char *win_out = "user_win";

  std::string win_in_pre(win_in);
  win_in_pre.append(".");
  std::string win_out_pre(win_out);
  win_out_pre.append(".");

  ASSERT_NO_THROW(COM_call_function(IN_read, file_in, win_in))
      << "An error occurred when reading from a control file" << std::endl;

  // Obtain the list of panes
  int np = 0, *pane_ids = nullptr;
  ASSERT_NO_THROW(COM_get_panes(win_in, &np, &pane_ids))
      << "An error occured when "
      << "getting the list of panes" << std::endl;

  // Create a new window and register the dataitems
  ASSERT_NO_THROW(COM_new_window(win_out))
      << "An error occured when "
      << "getting creating a window and registering the dataitem" << std::endl;

  // Loop through the panes to register the meshes
  for (int i = 0; i < np; ++i) {
    int nconn = 0;           // Number of connectivity tables
    char *cnames = nullptr;  // Names of connectivity tables separated by space

    // Obtain the connectivity tables
    ASSERT_NO_THROW(
        COM_get_connectivities(win_in, pane_ids[i], &nconn, &cnames))
        << "An error occurred when getting the connectivities" << std::endl;

    if (nconn == 1 && strncmp(cnames, ":st", 3) == 0) {  // Structured mesh
      int ndims;                                         // Number of elements
      int nglayers;  // Number of ghost layers.
      COM_get_size((win_in_pre + cnames).c_str(), pane_ids[i], &ndims,
                   &nglayers);
      COM_set_size((win_out_pre + cnames).c_str(), pane_ids[i], ndims,
                   nglayers);

      // Obtain the dimensions (must be a const array) of the pane and set them
      const int *dims;
      COM_get_array_const((win_in_pre + cnames).c_str(), pane_ids[i], &dims);
      COM_set_array_const((win_out_pre + cnames).c_str(), pane_ids[i], dims);

      std::cout << "Structured information" << endl;
      cout << "  ndims = " << ndims << endl;
      cout << "  nglayers = " << nglayers << endl;
      cout << "  dims[0] = " << dims[0] << endl;
      if (ndims > 1) cout << "  dims[1] = " << dims[1] << endl;
      if (ndims > 2) cout << "  dims[2] = " << dims[2] << endl;

    } else {        // Unstructured mesh
      int nnodes;   // total number of nodes
      int ngnodes;  // Number of ghost nodes

      // Obtain the size of nodes
      COM_get_size((win_in_pre + "nc").c_str(), pane_ids[i], &nnodes, &ngnodes);
      COM_set_size((win_out_pre + "nc").c_str(), pane_ids[i], nnodes, ngnodes);
      std::cout << "# nodes in dest set to " << nnodes
                << " & # gnodes in dest set to " << ngnodes << std::endl;

      // Obtain the sizes of connectivity tables
      if (nconn > 0) {
        std::istringstream is(cnames);
        for (int k = 0; k < nconn; ++k) {
          std::string cname;
          is >> cname;
          int nelems, ng;
          COM_get_size((win_in_pre + cname).c_str(), pane_ids[i], &nelems, &ng);
          COM_set_size((win_out_pre + cname).c_str(), pane_ids[i], nelems, ng);
          std::cout << "Connectivity table " << i << " has " << nelems
                    << " elements and " << ng << " ghost nodes" << std::endl;
          COM_resize_array((win_out_pre + cname).c_str(), pane_ids[i]);
        }
      }

      // free the buffer of cnames
      COM_free_buffer(&cnames);
    }

    COM_resize_array((win_out_pre + "nc").c_str(), pane_ids[i]);
  }

  // Obtain the list of dataitems
  int na;      // Number of dataitems
  char *atts;  // names of dataitems separated by spaces
  COM_get_dataitems(win_in, &na, &atts);

  std::istringstream is(atts);
  for (int i = 0; i < na; ++i) {
    // Obtain the dataitem name
    std::string aname;
    is >> aname;
    char loc;
    int type, ncomp;
    std::string unit;

    COM_get_dataitem(win_in_pre + aname, &loc, &type, &ncomp, &unit);
    std::cout << (win_in_pre + aname).c_str() << " has type " << type << endl;
    std::cout << (win_in_pre + aname).c_str() << " has " << ncomp
              << " components" << endl;
    std::string waname = win_out_pre + aname;
    COM_new_dataitem(waname.c_str(), loc, type, ncomp, unit.c_str());

    if (loc == 'w') {
      std::cout << "Windowed dataitem " << endl;

      // Obtain the size for a window dataitem.
      int nitems, ng;
      COM_get_size((win_in_pre + aname).c_str(), 0, &nitems, &ng);

      COM_set_size(waname.c_str(), 0, nitems, ng);

      COM_resize_array(waname.c_str(), 0, nullptr, ncomp);
    }
    // Loop through the panes to allocate memory
    else {
      std::cout << "Panel dataitem" << endl;
      // This is to demonstrate the loop over panes.
      // Could be replaced by a single call with paneID=0.
      for (int i2 = 0; i2 < np; ++i2) {
        if (loc == 'p') {
          // Obtain the size for a pane dataitem.
          int nitems, ng;
          COM_get_size((win_in_pre + aname).c_str(), pane_ids[i2], &nitems, &ng);
          COM_set_size(waname.c_str(), pane_ids[i2], nitems, ng);
        }
        COM_resize_array(waname.c_str(), pane_ids[i2], nullptr, ncomp);
      }
    }
  }

  // Free buffers for pane ids and dataitem names
  COM_free_buffer(&pane_ids);
  COM_free_buffer(&atts);

  // Mark the end of initialization
  COM_window_init_done(win_out);

  //  Finally, copy data from in to out.

  int OUT_all = COM_get_dataitem_handle((win_out_pre + "all").c_str());
  int IN_all = COM_get_dataitem_handle((win_in_pre + "all").c_str());
  EXPECT_NE(-1, IN_all) << "Data item: IN_all not found!\n";
  EXPECT_NE(-1, OUT_all) << "Data item: OUT_all not found!\n";

  ASSERT_NO_THROW(COM_call_function(IN_obtain, &IN_all, &OUT_all))
      << "An error occurred when "
      << "transferring data from SimIN module to SimOUT module" << std::endl;

  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  EXPECT_NE(-1, OUT_write) << "Function: OUT.write_dataitem not found!\n";
  ASSERT_NO_THROW(
      COM_call_function(OUT_write, file_out, &OUT_all, win_out, "000"))
      << "An error occurred when writing out the mesh data" << std::endl;

  COM_print_profile("", "");
  COM_finalize();
}

TEST(Intest, ParallelRead2FromControlFile) {
  COM_init(&ARGC, &ARGV);

  const char *CGNS_in = ARGV[1];
  const string winname(ARGV[2]);

  COM_set_profiling(1);

  COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");

  //===== Read in using Rocin
  int IN_read;
  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  EXPECT_NE(-1, IN_obtain) << "Function In.obtain_dataitem was not found!\n";

  const char *lastdot = std::strrchr(CGNS_in, '.');
  if (lastdot && std::strcmp(lastdot, ".txt") == 0) {
    IN_read = COM_get_function_handle("IN.read_by_control_file");
    EXPECT_NE(-1, IN_read) << "Function: IN.read_by_control_file not found!\n";
  } else {
    FAIL() << "Data file provided to a control file read test!\n";
    // IN_read = COM_get_function_handle( "IN.read_windows");
  }

  COM_call_function(IN_read, CGNS_in, winname.c_str());

  int NEW_all;
  NEW_all = COM_get_dataitem_handle((winname + ".all").c_str());
  EXPECT_NE(-1, NEW_all) << "Dataitem: " << winname << ".all was not found!\n";

  ASSERT_NO_THROW(COM_call_function(IN_obtain, &NEW_all, &NEW_all))
      << "An error occurred when obtaining all data items using SimIN"
      << std::endl;

  //===== Write out using Rocout
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  EXPECT_NE(-1, OUT_write) << "Function OUT.write_dataitem was not found!\n";
  ASSERT_NO_THROW(COM_call_function(OUT_write, winname.c_str(), &NEW_all,
                                    winname.c_str(), "000"))
      << "An error occurred when writing out all data items using SimOUT"
      << std::endl;

  COM_print_profile("", "");
  COM_finalize();
}

int main(int argc, char *argv[]) {
  // Ensure corrent number of input parameters
  // the first two are for test1 and parameters 1
  // and 3 for test2
  /* if ( argc < 4 || argc > 4 ) {
    std::cout << "Usage: To test in parallel: \n\t" << argv[0]
              << " <input CGNS file|Rocin control file> <winname>"
        << "<outputfile>\n" << std::endl;
    // Note: -com-mpi option will be eaten away by COM_init.
    exit(-1);
  } */
  // Tests one and two should output the same files, with slightyly
  // different names

  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  MPI_Init(&ARGC, &ARGV);
  int ret = RUN_ALL_TESTS();
  MPI_Finalize();
  return ret;
}
