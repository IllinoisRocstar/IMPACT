!!! Determine the sizes of F90 Pointer found at
!!! http://www.cca-forum.org/pipermail/cca-fortran/2003-February/000123.html
      MODULE m_pointers
        ! checking compilation of comf90.h
        INCLUDE 'comf90.h' 

        TYPE data
           INTEGER :: ibegin
           INTEGER, POINTER :: a(:)
           INTEGER :: iend
        END TYPE data
        
        TYPE data_wrapper
           INTEGER :: ibegin
           TYPE(data), POINTER :: p_data
           integer :: iend
        END TYPE data_wrapper
        
      END MODULE m_pointers


