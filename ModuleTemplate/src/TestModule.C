///
/// @file
/// @ingroup impact_group
/// @brief Test Module in C which contains load_module and unload_module.
/// @author Jessica Kress (jkress@ilrocstar.com)
/// @date May 1, 2014
/// 
/// This file serves as both a test and a simple example for implementing
/// a module in C/C++. This module provides only load and unload functions
/// at this point.
///
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include "FC.h"
#include "com.h"
#include "com_devel.hpp"


// C/C++ Binding
extern "C" void TestModule_load_module( const char *name) {
  std::cout << "Loading TestModule" << std::endl;
  COM_new_window(std::string(name));
  COM_window_init_done(std::string(name));
}
extern "C" void TestModule_unload_module( const char *name){
  std::cout << "Unloading TestModule" << std::endl;
  COM_delete_window(std::string(name));
}
