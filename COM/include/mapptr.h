//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

/** \file mapptr.h
 *  This file contains the prototypes of the Fortran 90 subroutines for
 *  mapping C routines into Fortran pointers. They should be called from C.
 *  @see com_f.C
 */

#include "com_basic.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
extern "C" {
#endif

void COM_F_FUNC2(com_mapptr_chr1d,COM_MAPPTR_CHR1D)(void);

void COM_F_FUNC2(com_mapptr_int0d,COM_MAPPTR_INT0D)(void);
void COM_F_FUNC2(com_mapptr_int1d,COM_MAPPTR_INT1D)(void);
void COM_F_FUNC2(com_mapptr_int2d,COM_MAPPTR_INT2D)(void);
void COM_F_FUNC2(com_mapptr_int3d,COM_MAPPTR_INT3D)(void);

void COM_F_FUNC2(com_mapptr_flt0d,COM_MAPPTR_FLT0D)(void);
void COM_F_FUNC2(com_mapptr_flt1d,COM_MAPPTR_FLT1D)(void);
void COM_F_FUNC2(com_mapptr_flt2d,COM_MAPPTR_FLT2D)(void);
void COM_F_FUNC2(com_mapptr_flt3d,COM_MAPPTR_FLT3D)(void);

void COM_F_FUNC2(com_mapptr_dbl0d,COM_MAPPTR_DBL0D)(void);
void COM_F_FUNC2(com_mapptr_dbl1d,COM_MAPPTR_DBL1D)(void);
void COM_F_FUNC2(com_mapptr_dbl2d,COM_MAPPTR_DBL2D)(void);
void COM_F_FUNC2(com_mapptr_dbl3d,COM_MAPPTR_DBL3D)(void);

void COM_F_FUNC2(com_getptr_chr1d,COM_GETPTR_CHR1D)(void);

void COM_F_FUNC2(com_getptr_int0d,COM_GETPTR_INT0D)(void);
void COM_F_FUNC2(com_getptr_int1d,COM_GETPTR_INT1D)(void);
void COM_F_FUNC2(com_getptr_int2d,COM_GETPTR_INT2D)(void);
void COM_F_FUNC2(com_getptr_int3d,COM_GETPTR_INT3D)(void);

void COM_F_FUNC2(com_getptr_flt0d,COM_GETPTR_FLT0D)(void);
void COM_F_FUNC2(com_getptr_flt1d,COM_GETPTR_FLT1D)(void);
void COM_F_FUNC2(com_getptr_flt2d,COM_GETPTR_FLT2D)(void);
void COM_F_FUNC2(com_getptr_flt3d,COM_GETPTR_FLT3D)(void);

void COM_F_FUNC2(com_getptr_dbl0d,COM_GETPTR_DBL0D)(void);
void COM_F_FUNC2(com_getptr_dbl1d,COM_GETPTR_DBL1D)(void);
void COM_F_FUNC2(com_getptr_dbl2d,COM_GETPTR_DBL2D)(void);
void COM_F_FUNC2(com_getptr_dbl3d,COM_GETPTR_DBL3D)(void);

void COM_F_FUNC2(com_set_true, COM_SET_TRUE)( int *);
void COM_F_FUNC2(com_set_false, COM_SET_FALSE)( int *);

void COM_F_FUNC2( com_copy_string, COM_COPY_STRING) 
       ( const char *str_from, const int *len_from, 
	 char *str_to, const int *len_to, long int len1, long int len2);

#ifdef __cplusplus
}
#endif

#endif /* DOXYGEN_SHOULD_SKIP_THIS */



