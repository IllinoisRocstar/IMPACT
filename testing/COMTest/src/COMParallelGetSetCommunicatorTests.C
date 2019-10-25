#include <iostream>
#include <sstream>
#include "COM_base.hpp"
#include "com_basic.h"
#include "com_c++.hpp"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

// Testing Fixture class for Com Get & Set access methods
class COMGetSetCommunicator : public ::testing::Test {
 public:
  // Per-test-suite set-up.
  // Called before the first test in this test suite.
  // Can be omitted if not needed.
  static void SetUpTestCase() { COM_init(&ARGC, &ARGV); }
  // Per-test-suite tear-down.
  // Called after the last test in this test suite.
  // Can be omitted if not needed.
  static void TearDownTestCase() {
    COM_finalize();
    // MPI_Finalize();
  }

 protected:
  COMGetSetCommunicator() {}
  virtual ~COMGetSetCommunicator() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
  // shared resources for the test fixture
};

// needed for shared resources to be recognized properly with GTest

///
/// Test for get and set communicator functions
///
/// @param result COM::TestResults object to store test results.
///
/// This function tests calling the functions
/// COM_get_communicator, COM_get_default_communicator,
/// COM_set_communicator, and COM_set_default_communicator
/// with different contexts and window settings.
/// It is only run when ENABLE_MPI is true

TEST_F(COMGetSetCommunicator, GetDefaultComm) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm default_comm = COM_get_default_communicator();
  ASSERT_EQ(MPI_COMM_WORLD, default_comm)
      << "COM_get_default fails to return the correct value when no default "
         "was set\n";
}

TEST_F(COMGetSetCommunicator, WithUninitializedWindow) {
  MPI_Comm comm;
  MPI_Comm default_comm = COM_get_default_communicator();
  COM_new_window("testparallelwindow1");
  COM_get_communicator("testparallelwindow1", &comm);
  EXPECT_EQ(default_comm, comm)
      << "COM_get_communicator fails to return the default communicator, "
         "window initialized with no communicator\n";
  COM_delete_window("testparallelwindow1");
}

TEST_F(COMGetSetCommunicator, WithInitializedWindow) {
  MPI_Comm comm;
  COM_new_window("testparallelwindow2", MPI_COMM_SELF);
  COM_get_communicator("testparallelwindow2", &comm);
  EXPECT_EQ(MPI_COMM_SELF, comm)
      << "COM_get_communicator does not return communicator used in window "
         "initialization\n";
  int errorSend = 0;
  int errorReceive = 0;
  if (comm != MPI_COMM_SELF) errorSend = 1;
  MPI_Allreduce(&errorSend, &errorReceive, 1, MPI_INTEGER, MPI_SUM, comm);
  EXPECT_EQ(0, errorReceive)
      << "An error occured in: " << errorReceive << " threads\n";
  COM_delete_window("testparallelwindow2");
}

TEST_F(COMGetSetCommunicator, GetSetDefault) {
  COM_set_default_communicator(MPI_COMM_SELF);
  MPI_Comm default_comm = COM_get_default_communicator();
  MPI_Comm comm;
  EXPECT_EQ(MPI_COMM_SELF, default_comm)
      << "COM_get_default_communicator does not return the communicator used "
         "by set_default _communicator\n";
  COM_new_window("testparallelwindow3");
  COM_get_communicator("testparallelwindow3", &comm);
  EXPECT_EQ(comm, default_comm)
      << "COM_get_communicator does not return the communicator"
      << " used by set_default_communicator! (a window intialized"
      << " with no communicator should have the default)\n";
  int errorSend = 0;
  int errorReceive = 0;
  if (comm != MPI_COMM_SELF) errorSend = 1;
  MPI_Allreduce(&errorSend, &errorReceive, 1, MPI_INTEGER, MPI_SUM, comm);
  EXPECT_EQ(0, errorReceive)
      << "An error occured in: " << errorReceive << " threads\n";
  COM_delete_window("testparallelwindow3");
}

TEST_F(COMGetSetCommunicator, WindowWithNoComm) {
  COM::Window* param_window = nullptr;
  COM::COM_base* rbase = COM_get_com();
  MPI_Comm comm;
  MPI_Comm default_comm = COM_get_default_communicator();
  COM_new_window("testparallelwindow");

  int h = COM_get_window_handle("testparallelwindow");
  EXPECT_GT(h, 0) << "window handle not found!\n";
  if (h > 0) param_window = rbase->get_window_object(h);

  comm = param_window->get_communicator();

  EXPECT_EQ(comm, default_comm)
      << "window::get_communicator does not return default communicator!"
      << " (window initialized with no communicator)" << std::endl;

  COM_delete_window("testparallelwindow");
}

TEST_F(COMGetSetCommunicator, GetCommWindowInitialized) {
  COM::Window* param_window = nullptr;
  COM::COM_base* rbase = COM_get_com();
  MPI_Comm comm;
  MPI_Comm default_comm = COM_get_default_communicator();
  COM_new_window("testparallelwindow6", MPI_COMM_SELF);

  int h = COM_get_window_handle("testparallelwindow6");
  EXPECT_GT(h, 0) << "window handle not found!\n";
  if (h > 0) param_window = rbase->get_window_object(h);

  comm = param_window->get_communicator();

  EXPECT_EQ(comm, MPI_COMM_SELF)
      << "window::get_communicator does not return communicator used"
      << " in window initialization!" << std::endl;

  COM_delete_window("testparallelwindow6");
}

TEST_F(COMGetSetCommunicator, InitializedWithMPI_COM_SELF) {
  COM::Window* param_window = nullptr;
  COM::COM_base* rbase = COM_get_com();
  COM_set_default_communicator(MPI_COMM_SELF);

  MPI_Comm default_comm = COM_get_default_communicator();

  EXPECT_EQ(default_comm, MPI_COMM_SELF)
      << "COM_get_default_communicator does not return the communicator"
      << " used by set_default_communicator!" << std::endl;

  COM_new_window("testparallelwindow");
  int h = COM_get_window_handle("testparallelwindow");
  EXPECT_GT(h, 0) << "window not found!\n";
  if (h > 0) param_window = rbase->get_window_object(h);

  MPI_Comm comm = param_window->get_communicator();
  EXPECT_EQ(default_comm, comm)
      << "window::get_communicator does not return the communicator"
      << " used by set_default_communicator! (a window intialized"
      << " with no communicator should have the default)" << std::endl;

  COM_delete_window("testparallelwindow");
}

TEST_F(COMGetSetCommunicator, SplitCommunicator) {
  MPI_Comm comm = MPI_COMM_WORLD;
  int size, rank;
  MPI_Comm_size(comm, &size);
  MPI_Comm_rank(comm, &rank);
  MPI_Comm newComm;

  COM::COM_base* rbase = COM_get_com();
  COM::Window* param_window = NULL;

  int color;
  if (rank < size / 2)
    color = 0;
  else
    color = 1;
  MPI_Comm_split(comm, color, rank, &newComm);
  COM_set_default_communicator(newComm);

  if (rank == 0 || rank == 1) {
    COM_new_window("testparallelwindow8");
    int h = COM_get_window_handle("testparallelwindow8");
    EXPECT_GT(h, 0) << "window handle not found\n";
    if (h > 0) {
      param_window = rbase->get_window_object(h);
      comm = param_window->get_communicator();
      EXPECT_EQ(comm, newComm)
          << "window::get_communicator does not return the newly"
          << " split communicator that was set as the default!" << std::endl;
    }
    COM_delete_window("testparallelwindow8");
  }
  COM_set_default_communicator(MPI_COMM_WORLD);
}

TEST_F(COMGetSetCommunicator, ModuleGetSetComm) {
  int commworksnew = 1;
  MPI_Comm comm;
  int rank, size;
  int funchand, h;
  const char* name = "TestParallelWin1";

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // JK 4/14/16: The set_default is needed now that we removed the forcing
  // of windows to load onto only MPI_COMM_SELF
  COM_set_default_communicator(MPI_COMM_SELF);
  COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestParallelWin1");
  COM_set_default_communicator(MPI_COMM_WORLD);
  // JK 4/14/16: End

  h = COM_get_window_handle("TestParallelWin1");
  EXPECT_GT(h, 0) << "Com window not found!\n";

  if (h > 0)
    funchand =
        COM_get_function_handle("TestParallelWin1.get_communicator_module");

  if (funchand > 0) COM_call_function(funchand, &commworksnew);
  EXPECT_NE(0, commworksnew)
      << "Get communicator module does not work as expected\n";

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
