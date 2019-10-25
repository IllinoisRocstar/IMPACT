//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef OFF_WRITER_H
#define OFF_WRITER_H

void SD_write_off_comref(FILE *fhandle, int pid, int n_sv, int n_sf,
                         const float coor[], const int sv_parent[],
                         const float sv_nc[], const int sv_co_id[],
                         const int sf_svID[], const int sf_parent[],
                         const float sf_nc[], const int sf_co_id[],
                         const int sv_parentvID[], const int *sv_pnID,
                         const int *sf_pnID = NULL);

void SD_set_option(const char *option, int value);

#endif
