#include <cstdlib>
#include <iostream>
#include "COM_base.hpp"
#include "com_basic.h"
#include "com_c++.hpp"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

/// This function implements several module management tests designed to
/// put module loading, unloading, and management through their paces.
/// Creating, obtaining and destroying modules is tested in both C++ and F90
/// languages including recursive tests wherein modules load modules.

class COMModuleLoadingTest : public ::testing::Test {
 protected:
  COMModuleLoadingTest() {}
  void SetUp() {
    // MPI_Init(&ARGC,&ARGV);
    int c = ARGC;
    char** v = new char*[c];
    for (int i = 0; i < c; ++i) {
      size_t len = strlen(ARGV[i]) + 1;
      v[i] = reinterpret_cast<char*>(malloc(len));
      memcpy(v[i], ARGV[i], len);
    }
    COM_init(&c, &v);
    for (int i = 0; i < ARGC; ++i) {
      delete v[i];
    }
    delete v;
  }
  void TearDown() {
    COM_finalize();
    ASSERT_EQ(nullptr, COM::COM_base::get_com())
        << "COM was not destroyed correctly" << std::endl
        << "pointer to comm is not a nullptr" << std::endl;
    std::cout << "The pointer to Com is a nullptr" << std::endl
              << "Com has been finalized succesfully" << std::endl;
    // MPI_Finalize();
  }
};

TEST_F(COMModuleLoadingTest, CMODULE) {
  bool c_window_exists = false;
  int h = COM_get_window_handle("TestWin1");
  EXPECT_EQ(-1, h) << "COM window already exists" << std::endl;

  // load the test module for the first time, the handle should
  // return -1 before loading
  COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
  h = COM_get_window_handle("TestWin1");
  EXPECT_EQ(1, h) << "COM window handle returns incorrect value" << std::endl;
  if (h > 0) c_window_exists = true;

  // unload the test module for the first time, the handle should
  // return -1 after unloading
  ASSERT_NE(nullptr, COM::COM_base::get_com())
      << "COM window was not propperly opened" << std::endl;
  if (c_window_exists) {
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
    h = COM_get_window_handle("TestWin1");
    c_window_exists = false;
  }
  ASSERT_EQ(-1, h) << "COM window was not unloaded properly" << std::endl;
}

TEST_F(COMModuleLoadingTest, FMODULE) {
  bool f_window_exists = false;
  int h = COM_get_window_handle("TestFWin1");
  EXPECT_EQ(-1, h) << "COMF window already exists!" << std::endl;

  // load the Ftest module for the first time, the handle should
  // return -1 before loading
  COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
  h = COM_get_window_handle("TestFWin1");
  EXPECT_EQ(1, h) << "COMF window handle returns incorrect value" << std::endl;
  if (h > 0) f_window_exists = true;

  // unload the Ftest module for the first time, the handle should
  // return -1 after unloading
  ASSERT_NE(nullptr, COM::COM_base::get_com())
      << "COMF window was not propperly opened" << std::endl;
  if (f_window_exists) {
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
    h = COM_get_window_handle("TestFWin1");
    f_window_exists = false;
  }
  ASSERT_EQ(-1, h) << "COMF window was not unloaded properly" << std::endl;
}

TEST_F(COMModuleLoadingTest, CandFMODULE) {
  bool c_window_exists = false;
  bool f_window_exists = false;
  int k = COM_get_window_handle("TestFWin1");
  int h = COM_get_window_handle("TestWin1");
  ASSERT_EQ(-1, k) << "COMF window was already initialized" << std::endl;
  ASSERT_EQ(-1, h) << "COM window was already intialized" << std::endl;

  // load the Ftest module for the first time, the handle should
  // return -1 before loading, 1 after loading
  COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
  k = COM_get_window_handle("TestFWin1");
  EXPECT_EQ(1, k) << "COMF window handle returns incorrect value" << std::endl;
  if (k > 0) f_window_exists = true;

  // load the Ctest module for the first time, the handle should
  // return -1 before loading, 2 after loading
  COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
  h = COM_get_window_handle("TestWin1");
  EXPECT_EQ(2, h) << "COM window handle update not working properly"
                  << std::endl;
  if (h > 0) c_window_exists = true;

  // unload the Ftest Module for the first time, the handle should
  // return -1 after unloading
  ASSERT_NE(nullptr, COM::COM_base::get_com())
      << "COMF window was not propperly opened" << std::endl;
  if (f_window_exists) {
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
    k = COM_get_window_handle("TestFWin1");
    f_window_exists = false;
  }
  ASSERT_EQ(-1, k) << "COMF window not properly unloaded" << std::endl;

  // load the Ftest Module for the second time, the handle should
  // return 2 after loading
  if (!f_window_exists) {
    COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
    k = COM_get_window_handle("TestFWin1");
    f_window_exists = true;
  }
  EXPECT_EQ(2, k);

  // unload the Ctest Module for the first time, the handle should
  // return -1 after unloading
  ASSERT_NE(nullptr, COM::COM_base::get_com())
      << "COM window was not propperly opened" << std::endl;
  if (c_window_exists) {
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
    h = COM_get_window_handle("TestWin1");
    c_window_exists = false;
  }
  ASSERT_EQ(-1, h) << "COM window not properly unloaded" << std::endl;

  // load the Ctest Module for the second time, the handle should
  // return 2 after loading
  if (!c_window_exists) {
    COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
    h = COM_get_window_handle("TestWin1");
    c_window_exists = true;
  }
  EXPECT_EQ(2, h);

  COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
  COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
  ASSERT_EQ(-1, COM_get_window_handle("TestWin1"))
      << "final C-unload did not work properly" << std::endl;
  ASSERT_EQ(-1, COM_get_window_handle("TestFWin1"))
      << "final F-unload did not work properly" << std::endl;
}

TEST_F(COMModuleLoadingTest, recursiveCModuleLoading) {
  // no windows exist initially, check that this is the case
  bool c_window_exists = false;
  bool c_func_exists = false;
  int h = COM_get_window_handle("TestWin1");
  EXPECT_EQ(h, -1) << "Window handle indicates a window already exists"
                   << std::endl;
  if (h > 0) c_window_exists = true;

  COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
  int funcH1 = COM_get_function_handle("TestWin1.Function1");
  EXPECT_EQ(funcH1, 1)
      << "Function handle indicates the test function does not exist"
      << std::endl;
  if (funcH1 > 0) c_func_exists = true;

  if (c_func_exists) {
    std::string win2("TestWin2");
    // CModule function1 recursively loads the CModule.
    COM_call_function(funcH1, &win2);
    int win2_handle = COM_get_window_handle(win2.c_str());
    ASSERT_EQ(win2_handle, 2)
        << "window2 was not recursively loaded" << std::endl;
    if (c_window_exists) {
      // unload the first window and check that it recursively
      // unloads the second window
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
      win2_handle = COM_get_window_handle(win2.c_str());
      EXPECT_EQ(-1, win2_handle)
          << "window2 was not recursively unloaded" << std::endl;
      h = COM_get_window_handle("TestWin1");
      EXPECT_EQ(-1, h) << "window1 was not unloaded" << std::endl;
    }
  }
}

TEST_F(COMModuleLoadingTest, recursiveFModuleLoading) {
  // no windows exist initially, check that this is the case
  bool f_window_exists = false;
  bool f_func_exists = false;
  int h = COM_get_window_handle("TestFWin1");
  EXPECT_EQ(h, -1) << "Window handle indicates a window already exists"
                   << std::endl;
  if (h > 0) f_window_exists = true;

  COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
  int funcH1 = COM_get_function_handle("TestFWin1.Function1");
  EXPECT_EQ(funcH1, 1)
      << "Function handle indicates the test function does not exist"
      << std::endl;
  if (funcH1 > 0) f_func_exists = true;

  if (f_func_exists) {
    std::string win2("TestFWin2");
    const char* win2ptr = win2.c_str();
    // CModule function1 recursively loads the FModule.
    COM_call_function(funcH1, win2ptr);
    int win2_handle = COM_get_window_handle(win2.c_str());
    // check that the second window was loaded
    ASSERT_EQ(win2_handle, 2)
        << "window2 was not recursively loaded" << std::endl;
    if (f_window_exists) {
      // unload the first window and check that it recursively
      // unloads the second window
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
      win2_handle = COM_get_window_handle(win2.c_str());
      EXPECT_EQ(-1, win2_handle)
          << "window2 was not recursively unloaded" << std::endl;
      h = COM_get_window_handle("TestFWin1");
      EXPECT_EQ(-1, h) << "window1 was not unloaded" << std::endl;
    }
  }
}

TEST_F(COMModuleLoadingTest, CModuleLoadsFModule) {
  // no windows exist initially, check that this is the case
  bool c_window_exists = false;
  bool c_func_exists = false;
  int h = COM_get_window_handle("TestFWin1");
  EXPECT_EQ(h, -1) << "Window handle indicates a window already exists"
                   << std::endl;
  if (h > 0) c_window_exists = true;

  COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
  int funcH3 = COM_get_function_handle("TestWin1.Function3");
  EXPECT_EQ(funcH3, 1)
      << "Function handle indicates the test function does not exist"
      << std::endl;
  if (funcH3 > 0) c_func_exists = true;

  if (c_func_exists) {
    std::string win3("TestFWin3");
    const char* win2ptr = win3.c_str();
    // CModule function1 recursively loads the FModule.
    COM_call_function(funcH3, &win3);
    int win3_handle = COM_get_window_handle(win3.c_str());
    // check that the second window was loaded
    ASSERT_EQ(win3_handle, 2)
        << "window3 was not recursively loaded" << std::endl;
    if (c_window_exists) {
      // unload the first window and check that it recursively
      // unloads the second window
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestWin1");
      win3_handle = COM_get_window_handle(win3.c_str());
      EXPECT_EQ(-1, win3_handle)
          << "window3 was not recursively unloaded" << std::endl;
      h = COM_get_window_handle("Test3Win1");
      EXPECT_EQ(-1, h) << "window1 was not unloaded" << std::endl;
    }
  }
}

TEST_F(COMModuleLoadingTest, FModuleLoadsCModule) {
  // no windows exist initially, check that this is the case
  bool f_window_exists = false;
  bool f_func_exists = false;
  int h = COM_get_window_handle("TestFWin1");
  EXPECT_EQ(h, -1) << "Window handle indicates a window already exists"
                   << std::endl;
  if (h > 0) f_window_exists = true;

  // load the FModule
  COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
  int funcH3 = COM_get_function_handle("TestFWin1.Function3");
  EXPECT_EQ(funcH3, 1)
      << "Function handle indicates the test function does not exist"
      << std::endl;
  if (funcH3 > 0) f_func_exists = true;

  if (f_func_exists) {
    std::string win3("TestFCWin");
    const char* win3ptr = win3.c_str();
    // CModule function1 recursively loads the FModule.
    COM_call_function(funcH3, win3ptr);
    int win3_handle = COM_get_window_handle(win3.c_str());
    // check that the second window was loaded
    ASSERT_EQ(win3_handle, 2)
        << "window3 was not recursively loaded" << std::endl;
    if (f_window_exists) {
      // unload the first window and check that it recursively
      // unloads the second window
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
      win3_handle = COM_get_window_handle(win3.c_str());
      EXPECT_EQ(-1, win3_handle)
          << "window3 was not recursively unloaded" << std::endl;
      h = COM_get_window_handle("Test3Win1");
      EXPECT_EQ(-1, h) << "window1 was not unloaded" << std::endl;
    }
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}