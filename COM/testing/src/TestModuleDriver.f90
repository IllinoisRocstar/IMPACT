!>
!> @file
!> @ingroup impact_group
!> @brief Main function of the test driver (in Fortran) for the test modules
!> @author Jessica Kress (jkress@ilrocstar.com)
!> @date May 1, 2014
!> 
!> This file serves as both a test and a simple example for implementing
!> a driver in Fortran. This driver loads and unloads two test modules: one
!> written in C/C++ and one written in Fortran.
!>
PROGRAM TestModuleDriverF 
  INCLUDE "comf90.h"
  
  CALL COM_INIT()

  CALL COM_LOAD_MODULE( "TestModule", "TestFWin1")

  CALL COM_UNLOAD_MODULE( "TestModule", "TestFWin1")

  CALL COM_LOAD_MODULE( "TestFortranModule", "TestFortranWin1")

  CALL COM_UNLOAD_MODULE( "TestFortranModule", "TestFortranWin1")

  CALL COM_FINALIZE()

END
