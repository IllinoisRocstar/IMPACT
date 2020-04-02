//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "COM_base.hpp"
#include "com.h"
#include "com_devel.hpp"
#include "gtest/gtest.h"

using namespace std;

// Global variables used to pass arguments to the tests
char **ARGV;
int ARGC;

COM_EXTERN_MODULE(SimIN)
COM_EXTERN_MODULE(SimOUT)

TEST(SerialWrite, WriteCGNS) {
  map<int, string> typeAsString;
  typeAsString[COM_CHAR] = "COM_CHAR";
  typeAsString[COM_UNSIGNED_CHAR] = "COM_UNSIGNED_CHAR";
  typeAsString[COM_BYTE] = "COM_BYTE";
  typeAsString[COM_SHORT] = "COM_SHORT";
  typeAsString[COM_UNSIGNED_SHORT] = "COM_UNSIGNED_SHORT";
  typeAsString[COM_INT] = "COM_INT";
  typeAsString[COM_UNSIGNED] = "COM_UNSIGNED";
  typeAsString[COM_LONG] = "COM_LONG";
  typeAsString[COM_UNSIGNED_LONG] = "COM_UNSIGNED_LONG";
  typeAsString[COM_FLOAT] = "COM_FLOAT";
  typeAsString[COM_DOUBLE] = "COM_DOUBLE";
  typeAsString[COM_LONG_DOUBLE] = "COM_LONG_DOUBLE";
  typeAsString[COM_BOOL] = "COM_BOOL";
  typeAsString[COM_CHARACTER] = "COM_CHARACTER";
  typeAsString[COM_LOGICAL] = "COM_LOGICAL";
  typeAsString[COM_INTEGER] = "COM_INTEGER";
  typeAsString[COM_REAL] = "COM_REAL";
  typeAsString[COM_DOUBLE_PRECISION] = "COM_DOUBLE_PRECISION";
  typeAsString[COM_COMPLEX] = "COM_COMPLEX";
  typeAsString[COM_DOUBLE_COMPLEX] = "COM_DOUBLE_COMPLEX";
  typeAsString[COM_MPI_COMMC] = "COM_MPI_COMMC";
  typeAsString[COM_MPI_COMMF] = "COM_MPI_COMMF";
  typeAsString[COM_STRING] = "COM_STRING";
  typeAsString[COM_RAWDATA] = "COM_RAWDATA";
  typeAsString[COM_METADATA] = "COM_METADATA";
  typeAsString[COM_VOID] = "COM_VOID";
  typeAsString[COM_F90POINTER] = "COM_F90POINTER";
  typeAsString[COM_OBJECT] = "COM_OBJECT";

  COM_init(&ARGC, &ARGV);

  COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");

  int IN_read = COM_get_function_handle("IN.read_by_control_file");
  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  int OUT_set = COM_get_function_handle("OUT.set_option");
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  EXPECT_NE(-1, IN_read) << "Function: IN.read_windows not found!\n";
  EXPECT_NE(-1, IN_obtain) << "Function: IN.obtain_dataitem not found!\n";
  EXPECT_NE(-1, OUT_set) << "Function: OUT.set_option not found!\n";
  EXPECT_NE(-1, OUT_write) << "Function: OUT.write_dataitem not found!\n";

  COM_set_verbose(11);
  COM_set_profiling(1);

  /* if ( argc < 3) {
    cout << "Usage: " << argv[0] << " <inputfile> <outputfile> " << endl;
    exit(-1);
  } */

  const char *file_in = ARGV[1];
  const char *file_out = ARGV[2];

  const char *win_in = "rocin_win";
  const char *win_out = "user_win";

  string win_in_pre(win_in);
  win_in_pre.append(".");
  string win_out_pre(win_out);
  win_out_pre.append(".");

  COM_call_function(IN_read, file_in, win_in);

  // Obtain the list of panes
  int np, *pane_ids;
  COM_get_panes(win_in, &np, &pane_ids);

  // Create a new window and register the dataitems
  COM_new_window(win_out);

  // Loop through the panes to register the meshes
  for (int i = 0; i < np; ++i) {
    int nconn;     // Number of connectivity tables
    char *cnames;  // Names of connectivity tables separated by space

    // Obtain the connectivity tables
    COM_get_connectivities(win_in, pane_ids[i], &nconn, &cnames);

    if (nconn == 1 && strncmp(cnames, ":st", 3) == 0) {  // Structured mesh
      int nelems;                                        // Number of elements
      int nglayers;  // Number of ghost layers.
      COM_get_size((win_in_pre + cnames).c_str(), pane_ids[i], &nelems,
                   &nglayers);
      COM_set_size((win_out_pre + cnames).c_str(), pane_ids[i], nelems,
                   nglayers);

      // Obtain the dimensions (must be a const array) of the pane and set them
      const int *dims;
      COM_get_array_const((win_in_pre + cnames).c_str(), pane_ids[i], &dims);
      COM_set_array_const((win_out_pre + cnames).c_str(), pane_ids[i], dims);

      cout << "Structured information" << endl;
      cout << "  nelems = " << nelems << endl;
      cout << "  nglayers = " << nglayers << endl;
      cout << "  dims[0] = " << dims[0] << endl;
      cout << "  dims[1] = " << dims[1] << endl;

    } else {        // Unstructured mesh
      int nnodes;   // total number of nodes
      int ngnodes;  // Number of ghost nodes

      // Obtain the size of nodes
      COM_get_size((win_in_pre + "nc").c_str(), pane_ids[i], &nnodes, &ngnodes);
      COM_set_size((win_out_pre + "nc").c_str(), pane_ids[i], nnodes, ngnodes);
      EXPECT_GT(nnodes, 0) << "Number of nodes should be greater than 0\n";
      EXPECT_EQ(0, ngnodes)
          << "Number of ghost nodes should be 0 in a serial application\n";
      cout << "# nodes in dest set to " << nnodes
           << " & # gnodes in dest set to " << ngnodes << endl;

      // Obtain the sizes of connectivity tables
      if (nconn > 0) {
        istringstream is(cnames);
        for (int k = 0; k < nconn; ++k) {
          string cname;
          is >> cname;
          int nelems, ng;
          COM_get_size((win_in_pre + cname).c_str(), pane_ids[i], &nelems, &ng);
          COM_set_size((win_out_pre + cname).c_str(), pane_ids[i], nelems, ng);
          cout << "Connectivity table " << i << " has " << nelems
               << " elements and " << ng << " ghost nodes" << endl;
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
  EXPECT_GT(na, 0) << "Number of dataitems should be greater than 0\n";

  // Add in the special cases
  string allatts("pconn ridges ");
  allatts += atts;

  istringstream is(allatts.c_str());
  for (int i = 0; i < na; ++i) {
    // Obtain the dataitem name
    string aname;
    is >> aname;
    char loc;
    int type, ncomp;
    string unit;

    COM_get_dataitem((win_in_pre + aname).c_str(), &loc, &type, &ncomp, &unit);
    cout << (win_in_pre + aname).c_str() << " has type " << typeAsString[type]
         << " (" << type << ')' << endl;
    cout << (win_in_pre + aname).c_str() << " has location '" << loc << '\''
         << endl;
    cout << (win_in_pre + aname).c_str() << " has " << ncomp << " components"
         << endl;
    string waname = win_out_pre + aname;
    COM_new_dataitem(waname.c_str(), loc, type, ncomp, unit.c_str());

    if (loc == 'w') {
      cout << "Windowed dataitem " << endl;

      // Obtain the size for a window dataitem.
      int nitems, ng;
      COM_get_size((win_in_pre + aname).c_str(), 0, &nitems, &ng);
      cout << (win_in_pre + aname).c_str() << " has " << nitems << " items and "
           << ng << " ghost items." << endl;

      COM_set_size(waname.c_str(), 0, nitems, ng);

      COM_resize_array(waname.c_str(), 0, NULL, ncomp);
    } else {  // Loop through the panes to allocate memory
      cout << "Panel dataitem" << endl;
      // This is to demonstrate the loop over panes.
      // Could be replaced by a single call with paneID=0.
      for (int i2 = 0; i2 < np; ++i2) {
        if (loc == 'p') {
          // Obtain the size for a pane dataitem.
          int nitems, ng;
          COM_get_size((win_in_pre + aname).c_str(), pane_ids[i2], &nitems, &ng);
          cout << (win_in_pre + aname).c_str() << " has " << nitems
               << " items and " << ng << " ghost items." << endl;
          COM_set_size(waname.c_str(), pane_ids[i2], nitems, ng);
        }

        COM_resize_array(waname.c_str(), pane_ids[i2], NULL, ncomp);
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
  EXPECT_NE(-1, OUT_all) << "Dataitem handle: " << win_out_pre
                         << "all was not found\n";
  EXPECT_NE(-1, IN_all) << "Dataitem handle: " << win_in_pre
                        << "all was not found\n";

  COM_call_function(IN_obtain, &IN_all, &OUT_all);

  string fo(file_out);

  // COM_call_function( OUT_set, "format", "HDF4");

  // COM_call_function( OUT_write, (fo + ".hdf").c_str(), &OUT_all, win_out,
  //                   "0000");

  COM_call_function(OUT_set, "format", "CGNS");

  COM_call_function(OUT_write, (fo + ".cgns").c_str(), &OUT_all, win_out,
                    "0000");

  COM_print_profile("", "");

  COM_UNLOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");
  COM_UNLOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");

  COM_finalize();
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}
