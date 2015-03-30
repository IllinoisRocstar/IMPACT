//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#if !defined(_ROCOUT_HDF4_H)

#include "com.h"
#include <string>

void write_dataitem_HDF4(const std::string& fname, const std::string& mfile, 
                         const COM::DataItem* attr, const char* material, 
                         const char* timelevel, int pane_id,
                         const std::string& errorhandle, int mode);

#endif // !defined(_ROCOUT_HDF4_H)



