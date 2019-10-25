//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef RFC_TIMING_H
#define RFC_TIMING_H

#include <sys/time.h>
#include "rfc_basic.h"

RFC_BEGIN_NAME_SPACE

inline double get_wtime() {
  ::timeval tv;
  gettimeofday(&tv, NULL);

  return tv.tv_sec + tv.tv_usec * 1.e-6;
}

RFC_END_NAME_SPACE

#endif
