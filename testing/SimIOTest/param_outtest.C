//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#define WINDOW_EXISTS 0

#include "COM_base.h"
#include "Rocin.h"
#include "Rocout.h"
#include "com.h"
#include "com_devel.h"
#include "gtest/gtest.h"

#include <cstring>
#include <iostream>
#include <string>

COM_EXTERN_MODULE(SimIN)
COM_EXTERN_MODULE(SimOUT)

using namespace std;

// Global variables used to pass command line arguments to the test
int ARGC;
char** ARGV;

TEST(paramOutTest, SimIOTest) { outTest(ARGC, ARGV); }
// argv[2] == parameter file out is not used??
void outTest(int argc, char** argv) {
  COM_init(&argc, &argv);

  const char* FILE_in = argv[1];
  const char* FILE_out = argv[2];
  const string winname("parameters");
  // const string winname(argv[2]);

  COM_set_profiling(1);
  // COM_set_verbose(10);

  COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");
  int IN_param = COM_get_function_handle("IN.read_parameter_file");
  int OUT_param = COM_get_function_handle("OUT.write_parameter_file");

  //===== Add some non parameter formatted dataitems to the window
  // a parameter formatted dataitem is a windowed COM_CHAR dataitem
  // with 1 component
  void* addr;

  ASSERT_EQ(WINDOW_EXISTS, 0) << "Window does not exist!";

  if (WINDOW_EXISTS) {
    COM_new_window(winname.c_str());
    COM_new_dataitem((winname + "." + "burnrate").c_str(), 'w', COM_CHAR, 1,
                     "");
    COM_set_size((winname + "." + "burnrate").c_str(), 0, 6, 0);
    COM_resize_array((winname + "." + "burnrate").c_str(), 0, &addr);
    ((char*&)addr) = "three";

    COM_new_dataitem((winname + "." + "notwindowed").c_str(), 'p', COM_CHAR, 1,
                     "");
    COM_set_size((winname + "." + "notwindowed").c_str(), 0, 12, 0);
    COM_resize_array((winname + "." + "notwindowed").c_str(), 0, &addr);
    ((char*&)addr) = "notwindowed";

    COM_new_dataitem((winname + "." + "wrongtype").c_str(), 'w', COM_INT, 1,
                     "");
    COM_set_size((winname + "." + "wrongtype").c_str(), 0, 1, 0);
    COM_resize_array((winname + "." + "wrongtype").c_str(), 0, &addr);
    ((int*&)addr)[1] = 1;

    COM_new_dataitem((winname + "." + "toomanycomps").c_str(), 'w', COM_CHAR, 2,
                     "");
    COM_set_size((winname + "." + "toomanycomps").c_str(), 0, 13, 0);
    COM_resize_array((winname + "." + "toomanycomps").c_str(), 0, &addr);
    ((char*&)addr) = "toomanycomps";

    COM_window_init_done(winname.c_str());
  }

  //===== Read in parameter file using Rocin
  COM_call_function(IN_param, FILE_in, (winname).c_str());

  //===== Write out using Rocout
  COM_call_function(OUT_param, FILE_out, (winname).c_str());

  COM_print_profile("", "");
  COM_finalize();
}

// Gtest main method
int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  EXPECT_EQ(argc, 3) << "Usage: To test in serial: \n\t" << argv[0]
                     << " <parameter file in> <parameter file out>\n"
                     << std::endl;
  if (argc < 3 || argc > 3) return -1;
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}
