//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

/** \file com_c.C
 *  This file contains the wrapper routines for the C binding
 *  of the COM implementation.
 *  @see com_c.h
 */

#include "COM_base.hpp"
#include "com_assertion.h"
#include <cstdarg>

// Include C++ implementation
#define C_ONLY
#include "com_c.h"
#include "com_c++.hpp" 

USE_COM_NAME_SPACE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Blocking function calls
void COM_call_function( const int wf, int argc, ...) {
  COM_assertion_msg( argc <= Function::MAX_NUMARG, "Too many arguments") ;

  int i;
  void *args[Function::MAX_NUMARG];
  va_list   ap;
  
  va_start( ap, argc);
  for ( i=0; i<argc; ++i) {
    args[i] = va_arg( ap, void*);
  }
  va_end( ap);
  COM_get_com()->call_function( wf, argc, args);
}

// Non-blocking function calls
void COM_icall_function( const int wf, int argc, ...) {
  COM_assertion_msg( argc <= Function::MAX_NUMARG, "Too many arguments") ;

  int i;
  void *args[Function::MAX_NUMARG];
  va_list   ap;

  va_start( ap, argc);
  for ( i=0; i<argc-1; ++i) {
    args[i] = va_arg( ap, void*);
  }
  int *status = va_arg( ap, int *);
  va_end( ap);
  COM_get_com()->icall_function( wf, argc, args, status); 
}

void COM_get_dataitem( const char *wa_str, char *loc, 
			int *type, int  *size, char *u_str,  int u_len) 
{
  std::string unit;
  COM_get_com()->get_dataitem( wa_str, loc, type, size, &unit);
  if ( u_str && u_len) {
    int len=unit.size(), n=std::min(len, int(u_len));
    std::copy( unit.c_str(), unit.c_str()+n, u_str);
    std::fill_n( u_str+1+n, std::max(0,int(u_len-len)), 0);
  }
}


#ifdef __cplusplus
}
#endif



