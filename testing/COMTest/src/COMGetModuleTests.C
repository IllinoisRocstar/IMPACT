#include <iostream>
#include "COM_base.hpp"
#include "com_basic.h"
#include "com_c++.hpp"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

TEST(COMGetModuleTest, GetModule) {
  COM_init(&ARGC, &ARGV);
  std::vector<std::string> module_names;
  std::vector<std::string> window_names;
  std::vector<std::string> imods;
  std::vector<std::string> iwins;
  std::vector<std::string> smods;
  std::vector<std::string> swins;

  // check that the module and window lists are initially empty
  ASSERT_NO_THROW(COM_get_modules(module_names))
      << "An error occurred when obtaining the module names\n";
  ASSERT_NO_THROW(COM_get_windows(window_names))
      << "An error occurred when obtaining the window names\n";
  EXPECT_TRUE(module_names.empty())
      << "Module list has initial values when it should not" << std::endl;
  EXPECT_TRUE(window_names.empty())
      << "Window list has initial values when it should not" << std::endl;
  //bool module_names_works = false;
  //bool window_names_works = false;
  std::vector<std::string>::iterator ni = module_names.begin();

  // push string names of windows and modules into vectors
  smods.push_back("COMTESTMOD");
  smods.push_back("COMFTESTMOD");
  swins.push_back("win1");
  swins.push_back("win2");
  swins.push_back("win3");
  swins.push_back("win4");
  module_names.resize(0);
  window_names.resize(0);

  // load each module into two different windows each
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "win1"))
      << "An error occurred when loading the"
      << " dummy C module\n";
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "win2"))
      << "An error occurred when loading the "
      << "dummy C module\n";
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "win3"))
      << "An error occurred when loading the "
      << "dummy Fortran module\n";
  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "win4"))
      << "An error occurred when loading the "
      << "dummy Fortran module\n";
  COM_get_modules(module_names);
  COM_get_windows(window_names);

  EXPECT_EQ(2, module_names.size()) << "Module names were not correctly loaded "
                                       "into vector using COM_get_modules"
                                    << std::endl;
  EXPECT_EQ(4, window_names.size()) << "Window names were not correctly loaded "
                                       "into vector using COM_get_windows"
                                    << std::endl;

  // check that module names are found in the vector
  ni = smods.begin();
  while (ni != smods.end()) {
    std::vector<std::string>::iterator mni =
        std::find(module_names.begin(), module_names.end(), *ni);
    EXPECT_NE(mni, module_names.end()) << "Couldn't find expected module: "
                                       << "\"" << *ni << "\"" << std::endl;
    ni++;
  }
  // check that each window is found in the vector
  ni = swins.begin();
  while (ni != swins.end()) {
    std::vector<std::string>::iterator mni =
        std::find(window_names.begin(), window_names.end(), *ni);
    EXPECT_NE(mni, window_names.end()) << "Couldn't find expected window: "
                                       << "\"" << *ni << "\"" << std::endl;
    ni++;
  }
  // print out the module list
  ni = module_names.begin();
  std::cout << "Module list:" << std::endl;
  while (ni != module_names.end())
    std::cout << "\"" << *ni++ << "\"" << std::endl;
  // print out the window list
  ni = window_names.begin();
  std::cout << "Window list:" << std::endl;
  while (ni != window_names.end())
    std::cout << "\"" << *ni++ << "\"" << std::endl;
  module_names.resize(0);
  window_names.resize(0);
  // unload the modules
  ASSERT_NO_THROW(COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "win1"))
      << "An error occurred"
      << " when unloading the dummy C module ";
  ASSERT_NO_THROW(COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD, "win2"))
      << "An error occurred"
      << " when unloading the dummy C module ";
  ASSERT_NO_THROW(COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "win3"))
      << "An error occurred"
      << " when unloading the dummy F module ";
  ASSERT_NO_THROW(COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "win4"))
      << "An error occurred"
      << " when unloading the dummy F module ";
  COM_get_modules(module_names);
  COM_get_windows(window_names);

  EXPECT_EQ(0, module_names.size())
      << "Module names were not correctly updated during unload" << std::endl;
  EXPECT_EQ(0, window_names.size())
      << "Window names were not correctly updated during unload" << std::endl;

  // in the case where modules and windows were not properly unloaded, print out
  // the remaining windows and modules
  if (!module_names.empty() && !(module_names == imods)) {
    //module_names_works = false;
    ni = module_names.begin();
    std::cout << "Final module list:" << std::endl;
    while (ni != module_names.end()) std::cout << *ni++ << std::endl;
  }
  if (!window_names.empty() && !(window_names == iwins)) {
    //window_names_works = false;
    ni = window_names.begin();
    std::cout << "Final window list:" << std::endl;
    while (ni != window_names.end()) std::cout << *ni++ << std::endl;
  }

  COM_finalize();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}
