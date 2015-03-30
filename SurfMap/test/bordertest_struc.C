//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "com.h"
#include <iostream>
#include <sstream>

COM_EXTERN_MODULE(SurfMap);
COM_EXTERN_MODULE(SimOUT);

using namespace std;

// ==== Routines for creating mesh information
//void init_structure_mesh( double coors_s[3][10][6][5]) {
void init_structure_mesh( double coors_s[5][6][10][3]) {
//void init_structure_mesh( double coors_s[3][5][6][10]) {

for (int i=0; i<10; i++){
	for (int j=0; j<6; j++){
		for (int k=0; k<5; k++){
			coors_s[k][j][i][0] = i*.02;
			coors_s[k][j][i][1] = j*.02;
			coors_s[k][j][i][2] = k*.02;
		}
	}
}

}

bool is_local( int pid, int comm_rank, int comm_size) {
  return ( pid % comm_size == comm_rank);
}

int main(int argc, char *argv[]) {
  const int ni=10, nj=6, nk=5;
  int total = ni*nj*nk;
  double  coors_s[nk][nj][ni][3];
  int dims[3];
  dims[0] = ni;
  dims[1] = nj;
  dims[2] = nk;
  //int     elmts[num_elmts][8];

  COM_init( &argc, &argv);
  COM_LOAD_MODULE_STATIC_DYNAMIC(SurfMap, "MAP");
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");


  COM_new_window("str");
  COM_new_dataitem("str.borders", 'n', COM_INTEGER, 1, "");

  init_structure_mesh( coors_s);

  COM_set_array_const("str.:st3:actual", 1, dims);
  COM_set_size("str.nc", 1, total);
  COM_set_array("str.nc", 1, &coors_s[0][0][0][0], 3);
  
  COM_resize_array( "str.borders");
  COM_window_init_done("str");


  int mesh_hdl = COM_get_dataitem_handle("str.mesh");


    std::cout << "Get border nodes... " << endl;
  
  int MAP_border_nodes = COM_get_function_handle( "MAP.pane_border_nodes");

  int borders_hdl = COM_get_dataitem_handle( "str.borders");
  COM_call_function( MAP_border_nodes, &mesh_hdl, &borders_hdl);

  // Output solution into file.
  int OUT_write = COM_get_function_handle( "OUT.write_dataitem");

  COM_call_function( OUT_write, "strucmesh", &borders_hdl, 
		     "strucmesh", "000");
  //COM_call_function( OUT_write, "strucmesh1", &mesh_hdl, 
		     //"strucmesh1", "000");

  COM_finalize();
}



