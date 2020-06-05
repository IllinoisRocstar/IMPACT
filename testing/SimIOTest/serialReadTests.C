//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** A test program that reads in given input CGNS files using Rocin and
 *  write out all the contents into one CGNS file using Rocout with
 *  a given file prefix.
 **/

#include <cstring>
#include <iostream>
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

TEST(Intest, SerialRead1FromWindow) {
  int c = ARGC;
  char **v = new char *[c];
  for (int i = 0; i < c; ++i) {
    size_t len = strlen(ARGV[i]) + 1;
    v[i] = reinterpret_cast<char *>(malloc(len));
    memcpy(v[i], ARGV[i], len);
  }
  ASSERT_NO_THROW(COM_init(&c, &v));

  const char *CGNS_in = v[1];
  const std::string winname(v[2]);

  COM_set_profiling(1);

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN"));
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));

  //===== Read in using Rocin
  int IN_read;
  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  EXPECT_NE(-1, IN_obtain) << "Function: IN.obtain_dataitem not found!\n";

  const char *lastdot = std::strrchr(CGNS_in, '.');
  if (lastdot && std::strcmp(lastdot, ".cgns") == 0) {
    IN_read = COM_get_function_handle("IN.read_windows");
    EXPECT_NE(-1, IN_read) << "Function: IN.read_windows not found!"
                           << std::endl;
  } else {
    FAIL() << "Control file provided to a data input read test!\n";
    // IN_read = COM_get_function_handle( "IN.read_by_control_file");
  }

  ASSERT_NO_THROW(COM_call_function(IN_read, CGNS_in, winname.c_str()));

  //int NEW_all;
  //NEW_all = COM_get_dataitem_handle((winname + ".all").c_str());
  //EXPECT_NE(-1, NEW_all) << "Data item: " << winname
  //                       << ".all handle not found!\n";

  //ASSERT_NO_THROW(COM_call_function(IN_obtain, &NEW_all, &NEW_all));
  //std::cout << __FILE__ << __LINE__ << std::endl;

  ////===== Write out using Rocout
  //int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  //EXPECT_NE(-1, OUT_write) << "Function: OUT.write_dataitem was not found!\n";
  //ASSERT_NO_THROW(COM_call_function(OUT_write, winname.c_str(), &NEW_all,
  //                                  winname.c_str(), "000"));
  //std::cout << __FILE__ << __LINE__ << std::endl;

  //// this should be changed in multiples of two depending on the number of com
  //// arguments
  //COM_print_profile("", "");
  //for (int i = 0; i < ARGC - 2; ++i) {
  //  delete v[i];
  //}
  delete[] v;
  COM_finalize();
}

TEST(Intest, SerialRead2FromWindow) {
  std::cout << __FILE__ << __LINE__ << std::endl;
  int c = ARGC;
  char **v = new char *[c];
  for (int i = 0; i < c; ++i) {
    size_t len = strlen(ARGV[i]) + 1;
    v[i] = reinterpret_cast<char *>(malloc(len));
    memcpy(v[i], ARGV[i], len);
  }
  COM_init(&c, &v);

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN"));
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));

  COM_set_verbose(11);
  COM_set_profiling(1);

  const char *file_in = v[1];
  const char *file_out = v[3];

  int IN_read;
  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  EXPECT_NE(-1, IN_obtain) << "Function: IN.obtain_dataitem not found!\n";

  // this should only be read in using "read_window" in this file -MAP
  const char *lastdot = std::strrchr(file_in, '.');
  if (lastdot && std::strcmp(lastdot, ".cgns") == 0) {
    IN_read = COM_get_function_handle("IN.read_window");
  } else {
    FAIL() << "Wrong file format is being passed as a parameter.\n"
           << "This test is intended to read cgns files!\n";
    // IN_read = COM_get_function_handle( "IN.read_by_control_file");
  }

  const char *win_in = "rocin_win";
  const char *win_out = "user_win";

  std::string win_in_pre(win_in);
  win_in_pre.append(".");
  std::string win_out_pre(win_out);
  win_out_pre.append(".");

  ASSERT_NO_THROW(COM_call_function(IN_read, file_in, win_in));

  // Obtain the list of panes
  int np, *pane_ids;
  ASSERT_NO_THROW(COM_get_panes(win_in, &np, &pane_ids));

  // Create a new window and register the dataitems
  ASSERT_NO_THROW(COM_new_window(win_out));

  // Loop through the panes to register the meshes
  for (int i = 0; i < np; ++i) {
    int nconn;     // Number of connectivity tables
    char *cnames;  // Names of connectivity tables separated by space

    // Obtain the connectivity tables
    ASSERT_NO_THROW(
        COM_get_connectivities(win_in, pane_ids[i], &nconn, &cnames));

    if (nconn == 1 && strncmp(cnames, ":st", 3) == 0) {  // Structured mesh
      int ndims;                                         // Number of elements
      int nglayers;  // Number of ghost layers.
      EXPECT_NO_THROW(COM_get_size((win_in_pre + cnames).c_str(), pane_ids[i],
                                   &ndims, &nglayers));
      EXPECT_NO_THROW(COM_set_size((win_out_pre + cnames).c_str(), pane_ids[i],
                                   ndims, nglayers));

      // Obtain the dimensions (must be a const array) of the pane and set them
      const int *dims;
      EXPECT_NO_THROW(COM_get_array_const((win_in_pre + cnames).c_str(),
                                          pane_ids[i], &dims));
      EXPECT_NO_THROW(COM_set_array_const((win_out_pre + cnames).c_str(),
                                          pane_ids[i], dims));

      std::cout << "Structured information" << std::endl;
      std::cout << "  ndims = " << ndims << std::endl;
      std::cout << "  nglayers = " << nglayers << std::endl;
      std::cout << "  dims[0] = " << dims[0] << std::endl;
      if (ndims > 1) std::cout << "  dims[1] = " << dims[1] << std::endl;
      if (ndims > 2) std::cout << "  dims[2] = " << dims[2] << std::endl;

    } else {        // Unstructured mesh
      int nnodes;   // total number of nodes
      int ngnodes;  // Number of ghost nodes

      // Obtain the size of nodes
      EXPECT_NO_THROW(COM_get_size((win_in_pre + "nc").c_str(), pane_ids[i],
                                   &nnodes, &ngnodes));
      EXPECT_NO_THROW(COM_set_size((win_out_pre + "nc").c_str(), pane_ids[i],
                                   nnodes, ngnodes));
      std::cout << "# nodes in dest set to " << nnodes
                << " & # gnodes in dest set to " << ngnodes << std::endl;

      // Obtain the sizes of connectivity tables
      if (nconn > 0) {
        std::istringstream is(cnames);
        for (int k = 0; k < nconn; ++k) {
          std::string cname;
          is >> cname;
          int nelems, ng;
          EXPECT_NO_THROW(COM_get_size((win_in_pre + cname).c_str(),
                                       pane_ids[i], &nelems, &ng));
          EXPECT_NO_THROW(COM_set_size((win_out_pre + cname).c_str(),
                                       pane_ids[i], nelems, ng));
          std::cout << "Connectivity table " << i << " has " << nelems
                    << " elements and " << ng << " ghost nodes" << std::endl;
          EXPECT_NO_THROW(
              COM_resize_array((win_out_pre + cname).c_str(), pane_ids[i]));
        }
      }

      // free the buffer of cnames
      ASSERT_NO_THROW(COM_free_buffer(&cnames));
    }

    ASSERT_NO_THROW(
        COM_resize_array((win_out_pre + "nc").c_str(), pane_ids[i]));
  }

  // Obtain the list of dataitems
  int na;      // Number of dataitems
  char *atts;  // names of dataitems separated by spaces
  ASSERT_NO_THROW(COM_get_dataitems(win_in, &na, &atts));

  std::istringstream is(atts);
  for (int i = 0; i < na; ++i) {
    // Obtain the dataitem name
    std::string aname;
    is >> aname;
    char loc;
    int type, ncomp;
    std::string unit;

    ASSERT_NO_THROW(COM_get_dataitem((win_in_pre + aname).c_str(), &loc, &type,
                                     &ncomp, &unit));
    std::cout << (win_in_pre + aname).c_str() << " has type " << type
              << std::endl;
    std::cout << (win_in_pre + aname).c_str() << " has " << ncomp
              << " components" << std::endl;
    std::string waname = win_out_pre + aname;
    ASSERT_NO_THROW(
        COM_new_dataitem(waname.c_str(), loc, type, ncomp, unit.c_str()));

    if (loc == 'w') {
      std::cout << "Windowed dataitem " << std::endl;

      // Obtain the size for a window dataitem.
      int nitems, ng;
      EXPECT_NO_THROW(
          COM_get_size((win_in_pre + aname).c_str(), 0, &nitems, &ng));

      EXPECT_NO_THROW(COM_set_size(waname.c_str(), 0, nitems, ng));

      EXPECT_NO_THROW(COM_resize_array(waname.c_str(), 0, NULL, ncomp));
    } else {  // Loop through the panes to allocate memory
      std::cout << "Panel dataitem" << std::endl;
      // This is to demonstrate the loop over panes.
      // Could be replaced by a single call with paneID=0.
      for (int i2 = 0; i2 < np; ++i2) {
        if (loc == 'p') {
          // Obtain the size for a pane dataitem.
          int nitems, ng;
          EXPECT_NO_THROW(COM_get_size((win_in_pre + aname).c_str(),
                                       pane_ids[i2], &nitems, &ng));
          EXPECT_NO_THROW(
              COM_set_size(waname.c_str(), pane_ids[i2], nitems, ng));
        }
        EXPECT_NO_THROW(
            COM_resize_array(waname.c_str(), pane_ids[i2], NULL, ncomp));
      }
    }
  }

  // Free buffers for pane ids and dataitem names
  EXPECT_NO_THROW(COM_free_buffer(&pane_ids));
  EXPECT_NO_THROW(COM_free_buffer(&atts));

  // Mark the end of initialization
  EXPECT_NO_THROW(COM_window_init_done(win_out));

  //  Finally, copy data from in to out.

  int OUT_all = COM_get_dataitem_handle((win_out_pre + "all").c_str());
  int IN_all = COM_get_dataitem_handle((win_in_pre + "all").c_str());
  EXPECT_NE(-1, OUT_all) << "Dataitem: " << win_out_pre << "all"
                         << " was not found!\n";
  EXPECT_NE(-1, IN_all) << "Dataitem: " << win_in_pre << "all"
                        << " was not found!\n";

  ASSERT_NO_THROW(COM_call_function(IN_obtain, &IN_all, &OUT_all));

  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  ASSERT_NO_THROW(
      COM_call_function(OUT_write, file_out, &OUT_all, win_out, "000"));

  COM_print_profile("", "");
  // this should be changed in multiples of two depending on the number of com
  // arguments
  for (int i = 0; i < ARGC - 2; ++i) {
    delete v[i];
  }
  delete[] v;
  COM_finalize();
}

TEST(Intest, SerialReadFromControlFile) {
  std::cout << __FILE__ << __LINE__ << std::endl;
  int c = ARGC;
  char **v = new char *[c];
  for (int i = 0; i < c; ++i) {
    size_t len = strlen(ARGV[i]) + 1;
    v[i] = reinterpret_cast<char *>(malloc(len));
    memcpy(v[i], ARGV[i], len);
  }
  ASSERT_NO_THROW(COM_init(&c, &v));

  const char *CGNS_in = v[5];
  const std::string winname(v[4]);

  COM_set_profiling(1);

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN"));
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));

  //===== Read in using Rocin
  int IN_read;
  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  EXPECT_NE(-1, IN_obtain) << "Function IN.obtain_dataitem was not found"
                           << std::endl;

  const char *lastdot = std::strrchr(CGNS_in, '.');
  if (lastdot && std::strcmp(lastdot, ".txt") == 0) {
    IN_read = COM_get_function_handle("IN.read_by_control_file");
    EXPECT_NE(-1, IN_read) << "Function IN.read_by_control_file was not found"
                           << std::endl;
  } else {
    FAIL() << "Control file not provided to a control file input read test!\n";
    // IN_read = COM_get_function_handle( "IN.read_windows");
  }

  ASSERT_NO_THROW(COM_call_function(IN_read, CGNS_in, winname.c_str()));

  int NEW_all;
  NEW_all = COM_get_dataitem_handle((winname + ".all").c_str());
  EXPECT_NE(-1, NEW_all) << "Dataitem: " << winname << ".all not found!\n";

  ASSERT_NO_THROW(COM_call_function(IN_obtain, &NEW_all, &NEW_all));

  //===== Write out using Rocout
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  EXPECT_NE(-1, OUT_write)
      << "Function OUT.write_dataitem function was not found!\n";
  ASSERT_NO_THROW(COM_call_function(OUT_write, winname.c_str(), &NEW_all,
                                    winname.c_str(), "000"));

  COM_print_profile("", "");
  // this should be changed in multiples of two depending on the number of com
  // arguments
  for (int i = 0; i < ARGC - 2; ++i) {
    delete v[i];
  }
  delete[] v;
  ASSERT_NO_THROW(COM_finalize());
}

int main(int argc, char *argv[]) {
  // Ensure corrent number of input parameters
  // the first two are for test1 and parameters 1
  // and 3 for test2
  /* if ( argc < 4 || argc > 4 ) {
    std::cout << "Usage: To test serially: \n\t" << argv[0]
              << " <input CGNS file|Rocin control file> <winname>"
        << "<outputfile>\n" << std::endl;
    exit(-1);
  } */

  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  std::cout << __FILE__ << __LINE__ << std::endl;
  return RUN_ALL_TESTS();
}
