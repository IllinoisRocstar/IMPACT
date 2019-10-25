!>
!> @file
!> @ingroup impact_group
!> @brief F90 Test Module
!> @author JEK
!> @date May 1, 2014
!> 
!> This F90 test module serves as both a test and a simple example for implementing
!> a COM module in Fortran.
!>
SUBROUTINE TESTROUTINE0(global,outstring)
 
  USE TESTOBJECT
  
  IMPLICIT NONE

  INCLUDE '../include/comf90.h'

  CHARACTER(80), INTENT(OUT) :: outstring
  TYPE(t_global), POINTER :: global
  CHARACTER(80) :: message
  
  message = 'Hello COM from '//TRIM(global%window_name)//'!'

  WRITE(*,'(A)') TRIM(message)
  WRITE(outstring,'(A)') message
  
END SUBROUTINE TESTROUTINE0

SUBROUTINE TESTROUTINE1(global,name)
 
  USE TESTOBJECT
  
  IMPLICIT NONE

  INCLUDE 'comf90.h'

  CHARACTER(*), INTENT(IN) :: name
  TYPE(t_global), POINTER :: global
  INTEGER :: handle

  WRITE(*,'(A)') 'Fortran module '//TRIM(global%window_name)//' loading Fortran module '//TRIM(name)//'.'

  global%other_window_name = TRIM(name) 
  CALL COM_LOAD_MODULE("COMFTESTMOD",TRIM(name))
  global%other_window_handle = COM_GET_WINDOW_HANDLE(TRIM(name))

END SUBROUTINE TESTROUTINE1

SUBROUTINE TESTROUTINE2(global,name)
 
  USE TESTOBJECT
  
  IMPLICIT NONE

  INCLUDE 'comf90.h'

  CHARACTER(80), INTENT(OUT) :: name
  TYPE(t_global), POINTER :: global
  INTEGER :: winhandle, funchandle
 
  
  WRITE(*,'(A)') 'Fortran module '//TRIM(global%window_name)//' calling Fortran module '//&
       TRIM(global%other_window_name)//'.Function0.'
  winhandle = COM_GET_WINDOW_HANDLE(TRIM(global%other_window_name))

  IF(winhandle .gt. 0) THEN
     funchandle = COM_get_function_handle(TRIM(global%other_window_name)//'.Function0')
     IF(funchandle .gt. 0) THEN
        CALL COM_CALL_FUNCTION(funchandle,1,name)
     ENDIF
  ENDIF

END SUBROUTINE TESTROUTINE2

SUBROUTINE TESTROUTINE3(global,name)
 
  USE TESTOBJECT
  
  IMPLICIT NONE

  INCLUDE 'comf90.h'

  CHARACTER(*), INTENT(IN) :: name
  TYPE(t_global), POINTER :: global
  INTEGER :: handle

  WRITE(*,'(A)') 'Fortran module '//TRIM(global%window_name)//' loading C module '//TRIM(name)//'.'

  global%c_window_name = TRIM(name) 
  CALL COM_LOAD_MODULE("COMTESTMOD",TRIM(name))
  global%c_window_handle = COM_GET_WINDOW_HANDLE(TRIM(name))

END SUBROUTINE TESTROUTINE3

SUBROUTINE TESTROUTINE4(global,outstring)
 
  USE TESTOBJECT
  
  IMPLICIT NONE

  INCLUDE 'comf90.h'

  CHARACTER(80), INTENT(OUT) :: outstring
  TYPE(t_global), POINTER :: global
  INTEGER :: winhandle, funchandle
 
  IF(global%c_window_handle .gt. 0) THEN
     WRITE(*,'(A)') 'Fortran module '//TRIM(global%window_name)//&
          ' calling C module '//TRIM(global%c_window_name)//'.Function0F.'
     funchandle = COM_get_function_handle(TRIM(global%c_window_name)//'.Function0F')
     IF(funchandle .gt. 0) THEN
!        WRITE(*,*) 'Calling now.'
        CALL COM_CALL_FUNCTION(funchandle,2,outstring)
        WRITE(*,*) 'Call complete.'
     ENDIF
  ENDIF
END SUBROUTINE TESTROUTINE4

SUBROUTINE INTFUNCTION(io_data)
 
  USE TESTOBJECT
  
  IMPLICIT NONE

  INCLUDE 'comf90.h'

  INTEGER, INTENT(INOUT) :: io_data
 
  io_data = 1 
  
END SUBROUTINE INTFUNCTION

SUBROUTINE CONSTINTFUNCTION(idata, string_data)
 
  USE TESTOBJECT
  
  IMPLICIT NONE

  INCLUDE 'comf90.h'

  INTEGER, INTENT(INOUT) :: idata
  CHARACTER*(*), INTENT(INOUT) :: string_data
 
  WRITE(string_data, '(I2)') idata

END SUBROUTINE CONSTINTFUNCTION

SUBROUTINE COMFTESTMOD_LOAD_MODULE(name)

  USE TESTOBJECT

  IMPLICIT NONE

  INCLUDE "comf90.h"

  INTERFACE
    SUBROUTINE COM_set_pointer(attr,ptr,asso)
      USE TESTOBJECT
      CHARACTER(*), INTENT(IN) :: attr
      TYPE(t_global), POINTER  :: ptr
      EXTERNAL asso
    END SUBROUTINE COM_set_pointer

    SUBROUTINE TESTROUTINE0(global,outstring)
      USE TESTOBJECT
      CHARACTER(*), INTENT(OUT) :: outstring
      TYPE(t_global), POINTER :: global
    END SUBROUTINE TESTROUTINE0

    SUBROUTINE TESTROUTINE1(global,name)
      USE TESTOBJECT
      CHARACTER(*), INTENT(IN) :: name
      TYPE(t_global), POINTER :: global
    END SUBROUTINE TESTROUTINE1

    SUBROUTINE TESTROUTINE2(global,outstring)
      USE TESTOBJECT
      CHARACTER(*), INTENT(OUT) :: outstring
      TYPE(t_global), POINTER :: global
    END SUBROUTINE TESTROUTINE2

    SUBROUTINE TESTROUTINE3(global,name)
      USE TESTOBJECT
      CHARACTER(*), INTENT(IN) :: name
      TYPE(t_global), POINTER :: global
    END SUBROUTINE TESTROUTINE3

    SUBROUTINE TESTROUTINE4(global,outstring)
      USE TESTOBJECT
      CHARACTER(*), INTENT(OUT) :: outstring
      TYPE(t_global), POINTER :: global
    END SUBROUTINE TESTROUTINE4

    SUBROUTINE INTFUNCTION(io_data)
      USE TESTOBJECT
      INTEGER, INTENT(INOUT) :: io_data
    END SUBROUTINE INTFUNCTION

    SUBROUTINE CONSTINTFUNCTION(idata, string_data)
      USE TESTOBJECT
      INTEGER, INTENT(INOUT) :: idata
      CHARACTER(*), INTENT(INOUT) :: string_data
    END SUBROUTINE CONSTINTFUNCTION

  END INTERFACE

  CHARACTER(*),intent(in) :: name
  INTEGER :: types(7)
  TYPE(t_global), POINTER :: glb
  

  WRITE(*,'(A)') "Loading TestFortranModule: "//TRIM(name)
  

  ALLOCATE(glb)
  glb%window_name = TRIM(name)
  glb%other_window_handle = -1
  glb%c_window_handle = -1
  CALL COM_NEW_WINDOW(TRIM(name))

  CALL COM_new_dataitem(TRIM(name)//'.global','w',COM_F90POINTER,1,'')
  CALL COM_allocate_array(TRIM(name)//'.global')

  types(1) = COM_F90POINTER
  types(2) = COM_STRING
  CALL COM_set_member_function(TRIM(name)//'.Function0',TESTROUTINE0, &
                               TRIM(name)//'.global','bb',types)

  CALL COM_set_member_function(TRIM(name)//'.Function1',TESTROUTINE1, &
                               TRIM(name)//'.global','bi',types)

  CALL COM_set_member_function(name//'.Function2',TESTROUTINE2, &
                               name//'.global','bb',types)
 
  CALL COM_set_member_function(TRIM(name)//'.Function3',TESTROUTINE3, &
                               TRIM(name)//'.global','bi',types)
 
  CALL COM_set_member_function(name//'.Function4',TESTROUTINE4, &
                               name//'.global','bb',types)
  types(1) = COM_INTEGER
  CALL COM_set_function(TRIM(name)//'.IntFunction',INTFUNCTION, &
                       'b',types)

  CALL COM_set_function(TRIM(name)//'.ConstIntFunction',CONSTINTFUNCTION, &
                       'bb',types)

  CALL COM_WINDOW_INIT_DONE(name)

  CALL COM_set_pointer(name//'.global',glb,associate_pointer )

END SUBROUTINE COMFTESTMOD_LOAD_MODULE


SUBROUTINE COMFTESTMOD_UNLOAD_MODULE(name)
  USE TESTOBJECT
  IMPLICIT NONE
  INCLUDE "comf90.h"
  INTERFACE 
    SUBROUTINE COM_get_pointer(attr,ptr,asso)
      USE TESTOBJECT
      CHARACTER(*), INTENT(IN) :: attr
      TYPE(t_global), POINTER :: ptr
      EXTERNAL asso
    END SUBROUTINE COM_get_pointer
  END INTERFACE
  character(*),intent(in) :: name
  TYPE(t_global), POINTER :: glb
  INTEGER :: window_handle,other_window_handle,c_window_handle, owlen

  WRITE(*,'(A)') "Unloading TestFortranModule: "//TRIM(name)
  NULLIFY(glb)
  window_handle = COM_GET_WINDOW_HANDLE(TRIM(name))
  if(window_handle .gt. 0) then
     CALL COM_get_pointer(TRIM(name)//'.global',glb,associate_pointer)
     IF(ASSOCIATED(glb).eqv..true.) THEN       
        WRITE(*,'(A)') 'Fortran module '//TRIM(glb%window_name)//' unloading name '//TRIM(name)
        if(glb%other_window_handle .gt. 0) then
           WRITE(*,*) 'Fortran module '//TRIM(glb%window_name)//&
                ' unloading external Fortran module '//TRIM(glb%other_window_name)//'.'
           other_window_handle = COM_GET_WINDOW_HANDLE(TRIM(glb%other_window_name))
           IF(other_window_handle .gt. 0) THEN
              CALL COM_UNLOAD_MODULE("COMFTESTMOD",TRIM(glb%other_window_name))
           ENDIF
        endif
        if(glb%c_window_handle .gt. 0) then
           WRITE(*,*) 'Fortran module '//TRIM(glb%window_name)//&
                ' unloading external C module '//TRIM(glb%c_window_name)//'.'
           c_window_handle = COM_GET_WINDOW_HANDLE(TRIM(glb%c_window_name))
           IF(c_window_handle .gt. 0) THEN
              CALL COM_UNLOAD_MODULE("COMTESTMOD",TRIM(glb%c_window_name))
           ENDIF
        endif
!        DEALLOCATE(glb)
     ENDIF
     CALL COM_DELETE_WINDOW(TRIM(name))
  endif
END SUBROUTINE COMFTESTMOD_UNLOAD_MODULE
