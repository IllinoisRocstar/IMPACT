//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//
#include <time.h>
#include <iostream>
#include "COM_base.hpp"
#include "com_basic.h"
#include "com_c++.hpp"
#include "gtest/gtest.h"

///
/// @brief Parallel Module management tests.
///
/// This function implements several parallel module management tests designed
/// to put module loading, unloading, and management through their paces.  It
/// uses the testing module COMTESTMOD which is written in C++.

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

// Testing Fixture class for testing Linear Data Transfer between
class COMParallelModuleLoading : public ::testing::Test {
 public:
  // Per-test-suite set-up.
  // Called before the first test in this test suite.
  // Can be omitted if not needed.
  static void SetUpTestCase() { COM_init(&ARGC, &ARGV); }
  // Per-test-suite tear-down.
  // Called after the last test in this test suite.
  // Can be omitted if not needed.
  static void TearDownTestCase() { COM_finalize(); }

 protected:
  COMParallelModuleLoading() {}
  virtual ~COMParallelModuleLoading() {}
  virtual void SetUp() {
    // MPI_Init(&ARGC,&ARGV);
  }
  virtual void TearDown() {
    // MPI_Finalize();
  }
  // shared resources for the test fixture
};

// needed for shared resources to be recognized properly with GTest

///
/// Test for COM_LOAD_MODULE_STATIC_DYNAMIC and
/// COM_UNLOAD_MODULE_STATIC_DYNAMIC.
///
/// @param result COM::TestResults object to store test results.
///
/// This function implements several module management tests designed to
/// test dataitem management.  It uses the two testing modules COMTESTMOD and
/// COMFTESTMOD which are C++ and F90 respectively.

TEST_F(COMParallelModuleLoading, LoadModuleInSpecificComm) {
  int world_rank, color, world_size;
  int errorSend = 0;
  int errorReceive = 0;
  MPI_Comm comm_check, groupComm, worldComm = MPI_COMM_WORLD;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  // Split the communicator so we can test which communicator the
  // module is loaded on
  if (world_rank < world_size / 2)
    color = 0;
  else
    color = 1;
  MPI_Comm_split(worldComm, color, world_rank, &groupComm);
  int h = -1000;

  COM_set_default_communicator(groupComm);

  if (color == 0) {
    // Load the module
    COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestParallelWin1");

    // See if the module window exists
    h = COM_get_window_handle("TestParallelWin1");
    EXPECT_NE(-1, h) << "COM_get_window_handle(\"TestParallelWin1\") returns "
                     << h << std::endl;
  }
  if (h == -1) errorSend = 1;

  if (color == 0 && h != -1) {
    COM_get_communicator("TestParallelWin1", &comm_check);
    EXPECT_EQ(comm_check, groupComm)
        << "COM_get_communicator does not return the newly"
        << " split communicator that was set as the default!" << std::endl;
    if (comm_check != groupComm) errorSend = 1;
  }

  if (color == 0) {
    // Unload the module
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestParallelWin1");

    // Check that the window no longer exists
    h = COM_get_window_handle("TestParallelWin1");
    EXPECT_EQ(-1, h) << "COM_get_window_handle(\"TestParallelWin1\") returns "
                     << h << std::endl;
    if (h != -1) errorSend = 1;
  }

  MPI_Allreduce(&errorSend, &errorReceive, 1, MPI_INTEGER, MPI_SUM, worldComm);
  EXPECT_EQ(0, errorReceive)
      << "An error occured in: " << errorReceive << " threads\n";
}

TEST_F(COMParallelModuleLoading, LoadModuleOnAllProcesses) {
  int rank/*, color*/, size;
  int errorSend = 0;
  int errorReceive = 0;
  MPI_Comm worldComm = MPI_COMM_WORLD;
  MPI_Comm_rank(worldComm, &rank);
  MPI_Comm_size(worldComm, &size);
  int number = 0, IncrementHandle = 0, h = -1000;

  // load the module on all, but only  increment on the root process
  COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestParallelWin1");
  h = COM_get_window_handle("TestParallelWin1");
  EXPECT_NE(-1, h) << "Window not found!\n";
  IncrementHandle = COM_get_function_handle("TestParallelWin1.Increment");
  if (rank == 1 && IncrementHandle > 0) {
    COM_call_function(IncrementHandle, &number, &rank);
  }

  MPI_Bcast(&number, 1, MPI_INT, 1, worldComm);

  EXPECT_EQ(1, number)
      << "Incrementing number and then broadcasting did not work!\n";

  if (IncrementHandle > 0) COM_call_function(IncrementHandle, &number, &rank);

  // int numbers[size];
  int numbers[4];
  // Change the size of the numbers array accordingly with the number of threads

  //int commstat =
  MPI_Allgather(&number, 1, MPI_INT, &numbers, 1, MPI_INT, worldComm);

  for (int i = 0; i < size; i++) {
    EXPECT_EQ(i + 1, numbers[i])
        << "AllGather of number into numbers did not get the correct values!"
        << std::endl;
  }

  MPI_Allreduce(&errorSend, &errorReceive, 1, MPI_INTEGER, MPI_SUM, worldComm);
  EXPECT_EQ(0, errorReceive)
      << "An error occured in: " << errorReceive << " threads\n";

  COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestParallelWin1");
}

///
/// @brief Parallel function call with barrier test.
///
/// This function implements a test designed to call a function
/// in parallel and verify that the barrier after the function call
/// worked. It uses the testing module COMTESTMOD which is written in C++.

TEST_F(COMParallelModuleLoading, BarrierTest) {
  MPI_Comm comm = MPI_COMM_WORLD;
  ;
  int rank, size;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);
  int h, sleep_handle/*, b*/;
  time_t t1, t2;

  ASSERT_NO_THROW(COM_set_default_communicator(MPI_COMM_SELF))
      << "An error occurred when setting"
      << " the default communicator in parallel\n";
  // Load the module
  ASSERT_NO_THROW(
      COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestParallelWin1"))
      << "An error occurred when "
      << "loading the C Module in parallel \n";
  ASSERT_NO_THROW(COM_set_default_communicator(MPI_COMM_WORLD))
      << "An error occurred when setting the"
      << " default communicator\n";

  // See if the module window exists
  h = COM_get_window_handle("TestParallelWin1");
  EXPECT_NE(-1, h) << "Window handle TestParallelWin1 was not found\n";
  // If the window exists see if the function exists
  if (h > 0)
    sleep_handle = COM_get_function_handle("TestParallelWin1.SleepMultiply");
  EXPECT_NE(-1, sleep_handle)
      << "Function SleepMultiply handle was not found\n";
  // get the current time
  t1 = time(NULL);

  // if the function exists, call it
  if (sleep_handle > 0)
    ASSERT_NO_THROW(COM_call_function(sleep_handle, &rank))
        << "An error occurred";

  // get current time
  t2 = time(NULL);

  // barrier function to force all processes to wait
  /*b = */MPI_Barrier(comm);

  // get current time
  t2 = time(NULL);

  // check that it took all the procceses the same amount of time
  double difft = static_cast<double>(t2 - t1);
  EXPECT_PRED_FORMAT2(::testing::DoubleLE, static_cast<double>(size - 1) - difft,
                      1.0e-3)
      << "Somehow, processes did not take the same"
      << "amount of time to a tolerance of 1.0e-3 sec\n";

  // Unload the module
  COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestParallelWin1");
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  MPI_Init(&ARGC, &ARGV);
  int ret = RUN_ALL_TESTS();
  MPI_Finalize();
  return ret;
}