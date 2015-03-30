///
/// @file
/// @ingroup impact_group
/// @brief Main function of the test driver (in C/C++) for the test modules
/// @author Jessica Kress (jkress@ilrocstar.com)
/// @date May 1, 2014
/// 
/// This file serves as both a test and a simple example for implementing
/// a driver in C/C++. This driver loads and unloads two test modules: one
/// written in C/C++ and one written in Fortran.
///
#include "com.h"
#include "com_devel.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>

COM_EXTERN_MODULE( TestModule);

using namespace std;

int main(int argc, char *argv[]){
  COM_init( &argc, &argv);

  COM_LOAD_MODULE_STATIC_DYNAMIC( TestModule, "TestWin1");

  COM_UNLOAD_MODULE_STATIC_DYNAMIC( TestModule, "TestWin1");

  COM_LOAD_MODULE_STATIC_DYNAMIC( TestFortranModule, "TestFortranCWin1");

  COM_UNLOAD_MODULE_STATIC_DYNAMIC( TestFortranModule, "TestFortranCWin1");

  COM_finalize();

}
