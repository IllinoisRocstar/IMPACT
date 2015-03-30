//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "Vector_n.h"

RFC_BEGIN_NAME_SPACE

std::ostream &operator<<( std::ostream &os,  const Array_n_const& v) {
  const Real *p=v.begin(); 
  if ( p!=v.end()) for (;;) {
    os << *p;
    ++p;
    if ( p!=v.end())
      os << " ";
    else
      break;
  }
  return os;
}

RFC_END_NAME_SPACE


