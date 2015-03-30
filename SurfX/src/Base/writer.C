//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "writer.h"

using namespace std;

static int start_vertexID=0;            /* whether the vertex ids start from 0 or 1*/
static int start_faceID=0;              /* whether the subface ids start from 0 or 1*/
static int start_subvertexID=0;         /* whether the subvertex ids start from 0 or 1*/
static int start_subfaceID=0;           /* whether the subface ids start from 0 or 1*/

/* This function is passed information about subvertices and subfaces to be written to .off file
   Returns 0  : if operation successful
   Returns 1  : if operation unsuccessful */

void SD_write_off_comref( FILE *fhandle, int pid, int n_sv,int n_sf,
			  const float coor[],const int sv_parent[],
			  const float sv_nc[],const int sv_co_id[],
			  const int sf_svID[],const int sf_parent[],
			  const float sf_nc[],const int sf_co_id[],
			  const int sv_parent_vID[],
			  const int *sv_pnID, const int *sf_pnID) {
  
  int i;

  fprintf(fhandle, "OFF\n");
  fprintf(fhandle, "%d   %d   %d  #! %d\n",n_sv,n_sf, 0, pid);
  fprintf(fhandle, "# SubVertex Information\n");
  fprintf(fhandle, "# Coor_x  Coor_y    Coor_z    #!  ParentFaceId NatCoor_xi NatCoor_eta  CorrespSubvertId  PartitionId  VertexId\n");

  for(i=0;i<n_sv;i++) {
    fprintf(fhandle,"%f  %f  %f  #!",coor[3*i], coor[3*i+1], coor[3*i+2]);
    fprintf(fhandle,"   %7d  ", sv_parent[i]-start_faceID);
    fprintf(fhandle,"   %f  %f", sv_nc[2*i], sv_nc[2*i+1]);
    fprintf(fhandle,"   %7d  ", sv_co_id[i]-start_vertexID);

    if(sv_pnID==NULL)
      fprintf(fhandle,"  1  ");
    else
      fprintf(fhandle,"%7d  ",sv_pnID[i]);
    fprintf(fhandle,"%d  \n",sv_parent_vID[i]);
  }

  fprintf(fhandle,"# SubFace Information\n");
  fprintf(fhandle,"# sv_id_1  sv_id_2  sv_id_3  #! ParentFaceId sv1_nc_xi sv1_nc_eta sv2_nc_xi sv2_nc_eta sv3_nc_xi sv3_nc_eta CorrespSubfaceId PartitionId \n");

  int numEdges = 3;
  for(i=0;i<n_sf;i++) {
    fprintf(fhandle,"%d  ",numEdges);
    fprintf(fhandle,"%d  ",sf_svID[3*i]-start_subvertexID);  
    fprintf(fhandle,"%d  ",sf_svID[3*i+1]-start_subvertexID);  
    fprintf(fhandle,"%d  ",sf_svID[3*i+2]-start_subvertexID);  
    fprintf(fhandle,"  #!   ");    
    fprintf(fhandle,"%7d  ",sf_parent[i]-start_faceID);
    fprintf(fhandle,"%f  ",sf_nc[6*i]);
    fprintf(fhandle,"%f  ",sf_nc[6*i+1]);
    fprintf(fhandle,"%f  ",sf_nc[6*i+2]);
    fprintf(fhandle,"%f  ",sf_nc[6*i+3]);
    fprintf(fhandle,"%f  ",sf_nc[6*i+4]);
    fprintf(fhandle,"%f  ",sf_nc[6*i+5]);
    fprintf(fhandle,"%7d  ",sf_co_id[i]-start_subfaceID);
    if(sf_pnID==NULL)
      fprintf(fhandle," 1");
    else
      fprintf(fhandle,"%d  \n",sf_pnID[i]);
  }

  start_vertexID    =0;
  start_faceID      =0;
  start_subvertexID =0;
  start_subfaceID   =0;

  return ;
}

/* This function is used to set the starting values od the vertex ,face,subvertex, and subface */
void SD_set_option(const char *option,int value) {
 
  if(strcmp(option,"start_vertexID")==0)
    start_vertexID=value;
  else if(strcmp(option,"start_faceID")==0)
    start_faceID=value;
  else if(strcmp(option,"start_subvertexID")==0)
    start_subvertexID=value;
  else if(strcmp(option,"start_subfaceID")==0)
    start_subfaceID=value;
  else
    exit(-1);
  return;
}


