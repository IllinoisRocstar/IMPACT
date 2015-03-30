!>
!> @file
!> @ingroup impact_group
!> @brief Test Module in Fortran which contains load_module and unload_module.
!> @author Jessica Kress (jkress@ilrocstar.com)
!> @date May 1, 2014
!> 
!> This file serves as both a test and a simple example for implementing
!> a module in Fortran. This module provides only load and unload functions
!> at this point.
!>
SUBROUTINE TESTFORTRANMODULE_LOAD_MODULE(name)
  INCLUDE "comf90.h"
  character(*),intent(in) :: name
  write(*,*) "Loading TestFortranModule"
  CALL COM_NEW_WINDOW(name)
  CALL COM_WINDOW_INIT_DONE(name)
END 

SUBROUTINE TESTFORTRANMODULE_UNLOAD_MODULE(name)
  INCLUDE "comf90.h"
  character(*),intent(in) :: name
  write(*,*) "Unloading TestFortranModule"
  CALL COM_DELETE_WINDOW(name)
END 
