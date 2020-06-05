#include <iostream>
#include "COM_base.hpp"
#include "com_basic.h"
#include "com_c++.hpp"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

/// Test for Module function registration and access
///
/// This function implements a simple test of the ability to
/// access functions which have been registered by modules.
/// Once the test module is loaded this unit test calls the test modules
/// functions to ensure that they have (1) been successfully registered
/// and (2) that they can successfully be called.

// helper function which trims the trailing whitespace at the end of a string
std::string& rtrim(std::string& str) {
  if (str[str.length() - 1] != ' ')
    return str;  // base case where the last char is a non-space
  for (int i = str.length() - 1; i >= 0; i--) {
    if (str[i] == ' ')
      continue;
    else {
      str.erase(i + 1, str.length() - 1);
      break;
    }
  }
  return str;
}
// helper function which trims the leading whitespace at the beginning of a
// string
std::string& ltrim(std::string& str) {
  if (str[0] != ' ')
    return str;  // base case where the first char is a non-space
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == ' ')
      continue;
    else {
      str.erase(0, i - 1);
      break;
    }
  }
  return str;
}

std::string& trim(std::string& str) { return ltrim(rtrim(str)); }

// C Module Testing Fixture class
class COMCModuleFuctionTest : public ::testing::Test {
 protected:
  COMCModuleFuctionTest() {}
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
    delete[] v;
    COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestCWin");

    // Get function handle and execute functions 0-4
    funcH0 = COM_get_function_handle("TestCWin.Function0");
    funcH1 = COM_get_function_handle("TestCWin.Function1");
    funcH2 = COM_get_function_handle("TestCWin.Function2");
    funcH3 = COM_get_function_handle("TestCWin.Function3");
    funcH4 = COM_get_function_handle("TestCWin.Function4");
    funcHInt = COM_get_function_handle("TestCWin.IntFunction");
    funcHConstInt = COM_get_function_handle("TestCWin.ConstIntFunction");
    funcHOpArgs = COM_get_function_handle("TestCWin.OptionalArgsFunction");
  }
  void TearDown() {
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "TestCWin");
    COM_finalize();
    ASSERT_EQ(nullptr, COM::COM_base::get_com())
        << "COM was not destroyed correctly" << std::endl
        << "pointer to comm is not a nullptr" << std::endl;
    std::cout << "The pointer to Com is a nullptr" << std::endl
              << "Com has been finalized succesfully" << std::endl;
    // MPI_Finalize();
  }
  int funcH0;
  int funcH1;
  int funcH2;
  int funcH3;
  int funcH4;
  int funcHInt;
  int funcHConstInt;
  int funcHOpArgs;
};

// Fortran Module Testing Fixture Class
class COMFModuleFunctionTest : public ::testing::Test {
 protected:
  COMFModuleFunctionTest() {}
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
    delete[] v;
    COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin");

    // Get function handle and execute functions 0-4
    funcH0 = COM_get_function_handle("TestFWin.Function0");
    funcH1 = COM_get_function_handle("TestFWin.Function1");
    funcH2 = COM_get_function_handle("TestFWin.Function2");
    funcH3 = COM_get_function_handle("TestFWin.Function3");
    funcH4 = COM_get_function_handle("TestFWin.Function4");
    funcHInt = COM_get_function_handle("TestFWin.IntFunction");
    funcHConstInt = COM_get_function_handle("TestFWin.ConstIntFunction");
  }
  void TearDown() {
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin");
    COM_finalize();
    ASSERT_EQ(nullptr, COM::COM_base::get_com())
        << "COM was not destroyed correctly" << std::endl
        << "pointer to comm is not a nullptr" << std::endl;
    std::cout << "The pointer to Com is a nullptr" << std::endl
              << "Com has been finalized succesfully" << std::endl;
    // MPI_Finalize();
  }
  int funcH0;
  int funcH1;
  int funcH2;
  int funcH3;
  int funcH4;
  int funcHInt;
  int funcHConstInt;
  int funcHOpArgs;
};

TEST_F(COMCModuleFuctionTest, CFunctionRegistration) {
  EXPECT_NE(-1, funcH0) << "Registration of Function 0 failed" << std::endl;
  EXPECT_NE(-1, funcH1) << "Registration of Function 1 failed" << std::endl;
  EXPECT_NE(-1, funcH2) << "Registration of Function 2 failed" << std::endl;
  EXPECT_NE(-1, funcH3) << "Registration of Function 3 failed" << std::endl;
  EXPECT_NE(-1, funcH4) << "Registration of Function 4 failed" << std::endl;
  EXPECT_NE(-1, funcHInt) << "Registration of IntFunction failed" << std::endl;
  EXPECT_NE(-1, funcHConstInt)
      << "Registration of ConstIntFunction 0 failed" << std::endl;
  EXPECT_NE(-1, funcHOpArgs)
      << "Registration of OptionalArgsFunction 0 failed" << std::endl;
}

TEST_F(COMCModuleFuctionTest, SimpleCFunctionExecution) {
  std::string result0;
  if (funcH0 != -1) {
    COM_call_function(funcH0, &result0);
  }
  EXPECT_STREQ("Hello COM from TestCWin!", result0.c_str())
      << "Function0 was not executed properly" << std::endl;

  if (funcHInt != -1) {
    int result = 0;
    COM_call_function(funcHInt, &result);
    EXPECT_EQ(1, result) << "Int Function returns the wrong value" << std::endl;
  }

  if (funcHConstInt != -1) {
    int result = 10;
    std::string output;
    COM_call_function(funcHConstInt, &result, &output);
    EXPECT_STREQ("10", output.c_str())
        << "Const Int Function returns the wrong value" << std::endl;
  }
}

TEST_F(COMCModuleFuctionTest, ComplexCExecution) {
  std::string result0;
  // name of the second window
  std::string win2("TestCWin2");
  int win2_func_handle = 0;
  int win2_handle = 0;
  if (funcH1 != -1) {
    // call function 1, which creates a second window
    COM_call_function(funcH1, &win2);
    win2_handle = COM_get_window_handle(win2.c_str());
    EXPECT_NE(-1, win2_handle)
        << "Second window is not being recursively generated properly"
        << std::endl;
    if (win2_handle > 0) {
      win2_func_handle = COM_get_function_handle("TestCWin2.Function0");
      EXPECT_NE(-1, win2_func_handle)
          << "Function for the second window is not being found" << std::endl;
      if (win2_func_handle > 0) {
        // call function 0 from the second window,
        COM_call_function(win2_func_handle, &result0);
        EXPECT_STREQ("Hello COM from TestCWin2!", result0.c_str());
      }
      result0.erase();
      if (funcH2 != -1) {
        // call function 2 from the first window
        COM_call_function(funcH2, &result0);
        EXPECT_STREQ("Hello COM from TestCWin2!", result0.c_str())
            << "Function1 was not executed properly" << std::endl;
      }
    }
  }

  std::string fwin3("TestCFWin");
  int fwin3_func_handle = 0;
  int fwin3_handle = 0;

  if (funcH3 != -1) {
    // call function 3 which creates a fortran window from a C window
    COM_call_function(funcH3, &fwin3);
    fwin3_handle = COM_get_window_handle(fwin3.c_str());
    EXPECT_NE(-1, fwin3_handle)
        << "Fortran window handle not initiated properly from a C window"
        << std::endl;
    if (fwin3_handle > 0) {
      // get the function0 handle from the fortran window
      fwin3_func_handle = COM_get_function_handle("TestCFWin.Function0");
      if (fwin3_func_handle > 0) {
        std::vector<char> fresult(81, '\0');
        COM_call_function(fwin3_func_handle, &fresult[0]);
        std::string result_string(&fresult[0]);
        trim(result_string);
        EXPECT_STREQ("Hello COM from TestCFWin!", result_string.c_str())
            << "Test of Fortran (loaded by C Module) Hello COM World returns: "
            << result_string << std::endl;
      }
      if (funcH4 != -1) {
        std::string result_string;
        COM_call_function(funcH4, &result_string);
        trim(result_string);
        EXPECT_STREQ("Hello COM from TestCFWin!", result_string.c_str())
            << "Test of Fortran (loaded by C Module) Hello COM World returns: "
            << result_string << std::endl;
      }
    }
  }
}

TEST_F(COMCModuleFuctionTest, COptionalArgsExecution) {
  if (funcHOpArgs != -1) {
    std::vector<int> args1(12, 0);
    std::vector<int>::iterator ai = args1.begin();
    // initialize the various arguments
    while (ai != args1.end()) {
      *ai = (ai - args1.begin());
      ai++;
    }

    int nargs = 0;
    COM_call_function(funcHOpArgs, &nargs);
    EXPECT_EQ(0, nargs) << "Optional args with " << nargs
                        << "arguments does not work properly" << std::endl;

    nargs = 1;
    COM_call_function(funcHOpArgs, &nargs, &args1[0]);
    EXPECT_EQ(0, nargs) << "Optional args with " << nargs
                        << "arguments does not work properly" << std::endl;

    nargs = 2;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1]);
    EXPECT_EQ(1, nargs) << "Optional args with " << nargs
                        << "arguments does not work properly" << std::endl;

    nargs = 3;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2]);
    EXPECT_EQ(3, nargs) << "Optional args with " << nargs
                        << "arguments does not work properly" << std::endl;

    nargs = 4;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3]);
    EXPECT_EQ(6, nargs) << "Optional args with " << nargs
                        << "arguments does not work properly" << std::endl;

    nargs = 5;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3], &args1[4]);
    EXPECT_EQ(10, nargs) << "Optional args with " << nargs
                         << "arguments does not work properly" << std::endl;

    nargs = 6;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3], &args1[4], &args1[5]);
    EXPECT_EQ(15, nargs) << "Optional args with " << nargs
                         << "arguments does not work properly" << std::endl;

    nargs = 7;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3], &args1[4], &args1[5], &args1[6]);
    EXPECT_EQ(21, nargs) << "Optional args with " << nargs
                         << "arguments does not work properly" << std::endl;

    nargs = 8;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3], &args1[4], &args1[5], &args1[6], &args1[7]);
    EXPECT_EQ(28, nargs) << "Optional args with " << nargs
                         << "arguments does not work properly" << std::endl;

    nargs = 9;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3], &args1[4], &args1[5], &args1[6], &args1[7],
                      &args1[8]);
    EXPECT_EQ(36, nargs) << "Optional args with " << nargs
                         << "arguments does not work properly" << std::endl;

    nargs = 10;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3], &args1[4], &args1[5], &args1[6], &args1[7],
                      &args1[8], &args1[9]);
    EXPECT_EQ(45, nargs) << "Optional args with " << nargs
                         << "arguments does not work properly" << std::endl;

    nargs = 11;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3], &args1[4], &args1[5], &args1[6], &args1[7],
                      &args1[8], &args1[9], &args1[10]);
    EXPECT_EQ(55, nargs) << "Optional args with " << nargs
                         << "arguments does not work properly" << std::endl;

    nargs = 12;
    COM_call_function(funcHOpArgs, &nargs, &args1[0], &args1[1], &args1[2],
                      &args1[3], &args1[4], &args1[5], &args1[6], &args1[7],
                      &args1[8], &args1[9], &args1[10], &args1[11]);
    EXPECT_EQ(66, nargs) << "Optional args with " << nargs
                         << "arguments does not work properly" << std::endl;
  }
}

TEST_F(COMFModuleFunctionTest, FFunctionRegistration) {
  EXPECT_NE(-1, funcH0) << "Registration of Function 0 failed" << std::endl;
  EXPECT_NE(-1, funcH1) << "Registration of Function 1 failed" << std::endl;
  EXPECT_NE(-1, funcH2) << "Registration of Function 2 failed" << std::endl;
  EXPECT_NE(-1, funcH3) << "Registration of Function 3 failed" << std::endl;
  EXPECT_NE(-1, funcH4) << "Registration of Function 4 failed" << std::endl;
  EXPECT_NE(-1, funcHInt) << "Registration of IntFunction failed" << std::endl;
  EXPECT_NE(-1, funcHConstInt)
      << "Registration of ConstIntFunction failed" << std::endl;
}

TEST_F(COMFModuleFunctionTest, SimpleFFunctionExecution) {
  std::vector<char> fresult(81, '\0');
  COM_call_function(funcH0, &fresult[0]);
  std::string result_string(&fresult[0]);
  trim(result_string);
  EXPECT_STREQ("Hello COM from TestFWin!", result_string.c_str())
      << "Function0 was not executed properly" << std::endl;

  if (funcHInt != -1) {
    int result = 0;
    COM_call_function(funcHInt, &result);
    EXPECT_EQ(1, result) << "Int Function returns the wrong value" << std::endl;
  }

  if (funcHConstInt != -1) {
    int result = 10;
    std::string output = "original";
    const char* outputptr = output.c_str();
    COM_call_function(funcHConstInt, &result, outputptr);
    trim(output);
    EXPECT_STREQ("10", output.c_str())
        << "Const Int Function returns the wrong value" << std::endl;
  }
}

TEST_F(COMFModuleFunctionTest, ComplexFExecution) {
  std::string fwin2("TestFWin2");
  const char* fwin2ptr = fwin2.c_str();
  int fwin2_func_handle = 0;
  int fwin2_handle = 0;
  EXPECT_NE(-1, funcH1) << "Function handle 1 was not found" << std::endl;
  COM_call_function(funcH1, fwin2ptr);
  fwin2_handle = COM_get_window_handle(fwin2.c_str());
  EXPECT_NE(-1, fwin2_handle)
      << "Second Fortran window was not loaded correctly" << std::endl;
  fwin2_func_handle = COM_get_function_handle("TestFWin2.Function0");
  EXPECT_NE(fwin2_func_handle, -1)
      << "Function not found in the second fortran window" << std::endl;
  std::vector<char> fresult(81, '\0');
  COM_call_function(fwin2_func_handle, &fresult[0]);
  std::string result_string(&fresult[0]);
  trim(result_string);
  EXPECT_STREQ("Hello COM from TestFWin2!", result_string.c_str())
      << "Test of Fortran (loaded by C Module) Hello COM World returns: "
      << result_string << std::endl;
  EXPECT_NE(-1, funcH2) << "Function hand 2 was not found" << std::endl;
  std::vector<char> fresult1(81, '\0');
  COM_call_function(funcH2, &fresult1[0]);
  std::string result_string1(&fresult1[0]);
  trim(result_string1);
  EXPECT_STREQ("Hello COM from TestFWin2!", result_string1.c_str())
      << "Test of Fortran (loaded by C Module) Hello COM World returns: "
      << result_string << std::endl;
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}
