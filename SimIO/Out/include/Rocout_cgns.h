//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//


/** \file Rocout_cgns.h
 *  Declaration of Rocout CGNS routines.
 */
#if !defined(_ROCOUT_CGNS_H)

#include "com.h"
#include <string>

/**
 ** Write the data for the given attribute to file.
 **
 ** Write the given attribute to file using the CGNS format.  The attribute
 ** may be a "mesh", "all" or some other predefined attribute.
 **
 ** \param fname The name of the main datafile. (Input)
 ** \param mname The name of the optional mesh datafile. (Input)
 ** \param attr The attribute to write out. (Input)
 ** \param material The name of the material. (Input)
 ** \param timelevel The simulation time for this data. (Input)
 ** \param pane_id The id for the local pane. (Input)
 ** \param ghosthandle "ignore" or "write" on ghost data.
 ** \param errorhandle "ignore", "warn", or "abort" on errors.
 ** \param mode Write == 0, append == 1. (Input)
 **/
void write_dataitem_CGNS(const std::string& fname, const std::string& mfile, 
                         const COM::DataItem* attr, const char* material, 
                         const char* timelevel, int pane_id,
                         const std::string& ghosthandle,
                         const std::string& errorhandle, int mode);
#endif // !defined(_ROCOUT_CGNS_H)



