//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef RFC_BASIC_H
#define RFC_BASIC_H

#define RFC_BEGIN_NAME_SPACE  namespace RFC {
#define RFC_END_NAME_SPACE    }

#define USE_RFC_NAME_SPACE using namespace RFC;

#include "surfbasic.h"
#include "Generic_element_2.h"
#include <vector>

RFC_BEGIN_NAME_SPACE

typedef double                                   Real;
typedef SURF::Point_3<Real>                      Point_3;
typedef SURF::Vector_3<Real>                     Vector_3;
typedef SURF::Vector_2<Real>                     Point_2;
typedef SURF::Vector_2<Real>                     Vector_2;
typedef SURF::Point_2<float>                     Point_2S;
typedef SURF::Generic_element_2                  Generic_element;

enum Color       { NONE=-1, BLUE=0, GREEN=1, OVERLAY=2 };
enum Parent_type { PARENT_VERTEX=0, PARENT_EDGE=1, 
		   PARENT_FACE=2, PARENT_NONE=3};

template <class _TT> 
inline void
free_vector( std::vector< _TT> &v) { std::vector<_TT> t; v.swap(t); }

void assertion_fail      ( const char*, const char*, int, const char*);
void precondition_fail   ( const char*, const char*, int, const char*);

#if defined(NDEBUG)
#  define RFC_precondition(EX) ((void)0)
#  define RFC_assertion(EX) ((void)0)
#  define RFC_assertion_msg(EX,MSG) ((void)0)
#  define RFC_assertion_code(CODE)
#else
#  define RFC_precondition(EX) \
   ((EX)?((void)0): precondition_fail( # EX , __FILE__, __LINE__, 0))
#  define RFC_assertion(EX) \
   ((EX)?((void)0): assertion_fail( # EX , __FILE__, __LINE__, 0))
#  define RFC_assertion_msg(EX,MSG) \
   ((EX)?((void)0): assertion_fail( # EX , __FILE__, __LINE__, MSG))
#  define RFC_assertion_code(CODE) CODE
#endif

RFC_END_NAME_SPACE

#endif // RFC_BASIC_H


