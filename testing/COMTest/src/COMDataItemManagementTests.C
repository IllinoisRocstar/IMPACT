#include <iostream>
#include <sstream>
#include <vector>
#include "COM_base.hpp"
#include "SolverAgent.H"
#include "com_basic.h"
#include "com_c++.hpp"
#include "gtest/gtest.h"

///
/// Test for COM_LOAD_MODULE_STATIC_DYNAMIC and
/// COM_UNLOAD_MODULE_STATIC_DYNAMIC.
///
/// @param result COM::TestResults object to store test results.
///
/// This function implements several module management tests designed to
/// test dataitem management.  It uses the two testing modules COMTESTMOD and
/// COMFTESTMOD which are C++ and F90 respectively.

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

// Testing Fixture class for testing Linear Data Transfer between
class COMDataItemManagement : public ::testing::Test {
 public:
  // Per-test-suite set-up.
  // Called before the first test in this test suite.
  // Can be omitted if not needed.
  static void SetUpTestCase() {
    COM_init(&ARGC, &ARGV);
    COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "Solver1");
    expected_dataitem_names = new std::vector<std::string>;
    expected_dataitem_names->push_back("global");
    expected_dataitem_names->push_back("displacement");
    expected_dataitem_names->push_back("temperature");
    expected_dataitem_names->push_back("nodeflags");
    expected_dataitem_names->push_back("cellflags");
    expected_dataitem_names->push_back("pressure");
    expected_dataitem_names->push_back("id");
    expected_dataitem_names->push_back("time");
    expected_dataitem_names->push_back("dt");
  }
  // Per-test-suite tear-down.
  // Called after the last test in this test suite.
  // Can be omitted if not needed.
  static void TearDownTestCase() {
    delete expected_dataitem_names;
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "Solver1");
    COM_finalize();
  }

 protected:
  COMDataItemManagement() {}
  virtual ~COMDataItemManagement() {}
  virtual void SetUp() {
    // MPI_Init(&ARGC,&ARGV);
  }
  virtual void
  TearDown() {  // transferObjects must be deleted before finalizing COM
    // MPI_Finalize();
  }
  // shared resources for the test fixture
  static std::vector<std::string>* expected_dataitem_names;
};

// needed for shared resources to be recognized properly with GTest
std::vector<std::string>* COMDataItemManagement::expected_dataitem_names =
    nullptr;

///
/// Test for COM_LOAD_MODULE_STATIC_DYNAMIC and
/// COM_UNLOAD_MODULE_STATIC_DYNAMIC.
///
/// @param result COM::TestResults object to store test results.
///
/// This function implements several module management tests designed to
/// test dataitem management.  It uses the two testing modules COMTESTMOD and
/// COMFTESTMOD which are C++ and F90 respectively.

TEST_F(COMDataItemManagement, GetNonLocalDataItem) {
  int InitHandle = COM_get_function_handle("Solver1.Init");
  int DumpHandle = COM_get_function_handle("Solver1.Dump");
  int ValidateHandle = COM_get_function_handle("Solver1.ValidateAddress");
  EXPECT_NE(-1, InitHandle) << "Function handle Solver1.Init was not found\n";
  EXPECT_NE(-1, DumpHandle) << "Function handle Solver1.Dump was not found\n";
  EXPECT_NE(-1, ValidateHandle)
      << "Function handle Solver1.ValidateAddress was not found\n";

  std::string meshname("testTriangle1.mesh");
  std::string filename("Solver1");
  int use_timestamp = 0;
  // This init function registers a bunch of DataItems
  ASSERT_NO_THROW(COM_call_function(InitHandle, &meshname))
      << "An error occurred when registering"
      << " the dataitems." << std::endl;
  ASSERT_NO_THROW(COM_call_function(DumpHandle, &filename, &use_timestamp))
      << "An error occurred when dumping dataitem information" << std::endl;
  std::string dataitem_names;
  int number_of_dataitems = 0;
  ASSERT_NO_THROW(
      COM_get_dataitems("Solver1", &number_of_dataitems, dataitem_names))
      << "An error occurred when retrieving the dataitem names" << std::endl;
  std::istringstream Istr(dataitem_names);
  std::cout << "COM::Test__DataItemManagement::DataItems:" << std::endl;
  for (int i = 0; i < number_of_dataitems; i++) {
    std::string dataitem_name;
    Istr >> dataitem_name;
    std::cout << dataitem_name << std::endl;
  }
  std::vector<std::string>::iterator edit = expected_dataitem_names->begin();
  while (edit != expected_dataitem_names->end()) {
    std::string::size_type x = dataitem_names.find(*edit);
    ASSERT_NE(std::string::npos, x) << *edit << " data item was not found\n";
    if (*edit != "global" && ValidateHandle) {
      int result = 0;
      int myaddress;
      std::string full_data_name("Solver1." + *edit);
      std::string data_name(*edit);
      char test_loc;
      COM_Type get_dataitem_type;
      int get_dataitem_size;
      std::string get_dataitem_units;
      ASSERT_NO_THROW(COM_get_dataitem(full_data_name, &test_loc,
                                       &get_dataitem_type, &get_dataitem_size,
                                       &get_dataitem_units))
          << "An error occurred when retrieving a dataitem type, name, size, "
             "and units"
          << std::endl;
      void* testarray_get;
      if (test_loc == 'w')
        ASSERT_NO_THROW(
            COM_get_array(full_data_name.c_str(), 0, &testarray_get))
            << "An error occurred when "
            << "retrieving a dataitem array" << std::endl;
      else
        ASSERT_NO_THROW(
            COM_get_array(full_data_name.c_str(), 101, &testarray_get))
            << "An error occurred when "
            << "retrieving a dataitem array" << std::endl;

      ASSERT_NO_THROW(
          COM_call_function(ValidateHandle, &data_name, testarray_get, &result))
          << "An error occurred "
          << "when validating the dataitem address" << std::endl;
      EXPECT_EQ(1, result) << data_name << " was not properly called\n";
    }
    *edit++;
  }
}

TEST_F(COMDataItemManagement, GetDataItemHandle) {
  std::vector<std::string>::iterator edit = expected_dataitem_names->begin();
  std::string wname = "Solver1.";
  while (edit != expected_dataitem_names->end()) {
    std::string datastring = *edit;
    std::string waname = wname + datastring;
    int dataitem_handle = COM_get_dataitem_handle(waname.c_str());
    EXPECT_NE(-1, dataitem_handle)
        << datastring << " dataitem handle was not found\n";
    edit++;
  }
}

TEST_F(COMDataItemManagement, DataItemManagementRuns) {
  int use_timestamp = 0;
  int StepHandle = COM_get_function_handle("Solver1.Step");
  int DumpHandle = COM_get_function_handle("Solver1.Dump");
  std::string meshname("testTriangle1.mesh");
  std::string filename("Solver1");
  for (int i = 0; i < 100; i++) {
    ASSERT_NO_THROW(COM_call_function(StepHandle))
        << "An error occurred after calling StepHandle " << i
        << " number of times" << std::endl;
    ASSERT_NO_THROW(COM_call_function(DumpHandle, &filename, &use_timestamp))
        << "An error occurred after calling DumpHandle " << i << " times"
        << std::endl;
  }
  SUCCEED();  // any issues will arise during the 200 function calls
}

// Test for COM_get_size, COM_get_array, and COM_get_dataitem
TEST_F(COMDataItemManagement, GetSizeArrayDataItem) {
  COM_new_window("testwindow");
  int testarray_size = 20;
  std::vector<int> testarray(testarray_size);
  for (int i = 0; i < testarray_size; i++) testarray[i] = i * i * 3 - 2 * i;
  COM_new_dataitem("testwindow.array", 'w', COM_DOUBLE, testarray_size, "");
  COM_set_size("testwindow.array", 0, testarray_size);
  COM_set_array("testwindow.array", 0, &testarray[0]);

  char test_loc = 'w';
  COM_Type get_dataitem_type;
  int get_dataitem_size;
  std::string get_dataitem_units;
  COM_get_dataitem("testwindow.array", &test_loc, &get_dataitem_type,
                   &get_dataitem_size, &get_dataitem_units);

  EXPECT_FALSE(get_dataitem_type != COM_DOUBLE)
      << "COM_get_dataitem returns the wrong data item type\n";
  EXPECT_FALSE(get_dataitem_size != testarray_size)
      << "COM_get_dataitem returns the wrong array size\n";
  EXPECT_FALSE(get_dataitem_units != "")
      << "COM_get_dataitem returns blank units when it should not\n";

  int testarray_get_size;
  COM_get_size("testwindow.array", 0, &testarray_get_size);
  EXPECT_TRUE(testarray_get_size == testarray_size)
      << "COM_get_size returns the wrong size\n";
  int* testarray_get;
  COM_get_array("testwindow.array", 0, &testarray_get);
  EXPECT_EQ(testarray_get, &testarray[0])
      << "COM_get_array returns a pointer to the wrong array location\n";
  for (int i = 0; i < testarray_get_size; i++) {
    EXPECT_EQ(i * i * 3 - 2 * i, testarray_get[i]) << "\n";
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}