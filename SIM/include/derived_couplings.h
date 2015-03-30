//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef _DERIVED_COUPLINGS_H_
#define _DERIVED_COUPLINGS_H_

#include "Coupling.h"

#define DECLARE_NEW_COUPLING_SCHEME( New_scheme)			\
  class New_scheme : public Coupling {					\
  public:								\
    New_scheme(MPI_Comm com, Control_parameters *p, const RocmanControl_parameters *mp);		\
    New_scheme(const char *, MPI_Comm com, Control_parameters *p, const RocmanControl_parameters *mp);\
    New_scheme(const char *, const char *, MPI_Comm com, Control_parameters *p, const RocmanControl_parameters *mp);		\
    New_scheme(const char *, const char *, const char*, MPI_Comm com, Control_parameters *p, const RocmanControl_parameters *mp);	\
  };

DECLARE_NEW_COUPLING_SCHEME( FluidSolidISS);

DECLARE_NEW_FULLY_COUPLING_SCHEME( SolidFluidBurnEnergySPC);

#endif



