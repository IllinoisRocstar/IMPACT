//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "com.h"
#include "mapbasic.h"
#include <iostream>

using namespace std;


int get_comm_rank( MPI_Comm comm) {
  int rank;
  int ierr = MPI_Comm_rank( comm, &rank); assert( ierr == 0);
  return rank;
}
int get_comm_size( MPI_Comm comm) {
  int size;
  int ierr = MPI_Comm_size( comm, &size); assert( ierr == 0);
  return size;
}

int main(int argc, char *argv[]) {
  MPI_Init( &argc, &argv);

  MPI_Comm comm = MPI_COMM_WORLD;
  int size_of_p = get_comm_size(comm);
  int myrank = get_comm_rank(comm);
  if(myrank == 0)
    std::cout << "myrank = " << myrank <<" and size_of_p = " << size_of_p << std::endl;
  
  COM_init( &argc, &argv);
  assert(size_of_p==2 && myrank <2);

  if(myrank == 0)
    cout << "Reading mesh file \"" << argv[1] << '"' << endl;

  std::string wname("surf");
  if(myrank == 0)
    cout << "Creating window \"" << wname << '"' << endl;

  // Read in HDF format
  if(myrank == 0)
    cout << "Loading Rocin" << endl;
  COM_load_module( "SimIN", "IN");
 
  std::string fname("surf000");
  if(myrank==0)
    fname += "0.hdf";
  else
    fname += "1.hdf";
  if (myrank ==0)
    cout << "Reading file " << fname << endl;
  int IN_read = COM_get_function_handle( "IN.read_window");
  COM_call_function( IN_read, 
  		     ("test/"+fname).c_str(), 
  		     wname.c_str(),
		     &comm);

  if (myrank ==0)
    std::cout << "Obtaining the mesh " << endl;
  int IN_obtain = COM_get_function_handle( "IN.obtain_dataitem");
  int mesh_hdl = COM_get_dataitem_handle((wname+".mesh").c_str());
  COM_call_function( IN_obtain, &mesh_hdl, &mesh_hdl);

  // Change the memory layout to contiguous.
  if (myrank ==0)
    std::cout << "Resizing the dataitem arrays " << endl;
  COM_resize_array( (wname+".all").c_str(), 0, NULL, 0);

  int npanes; 
  int* pane_ids;
  COM_get_panes( wname.c_str(), &npanes, &pane_ids);
  COM_assertion( npanes>=0);
  if(myrank ==0)
    std::cout << npanes << " panes found" << endl;
  
  if(myrank==0)
    std::cout<< "Labeling nodes with pane ids" << endl;
  COM_new_dataitem("surf.pane_ids",'n', COM_FLOAT, 1, "empty");
  COM_resize_array("surf.pane_ids");
 
  int nitems;
  float *ptr;
  for(int j = 0; j <npanes; ++j){
    COM_get_size("surf.pane_ids", pane_ids[j], &nitems);
    COM_get_array("surf.pane_ids", pane_ids[j], &ptr);
    for(int k =0; k<nitems; ++k){
      ptr[k] = pane_ids[j];
    }
  }
  int pid_hdl = COM_get_dataitem_handle("surf.pane_ids");

  if(myrank == 0)
    cout << "Loading Rocmap" << endl;
  COM_load_module( "SurfMap", "MAP");

  if(myrank == 0)
    std::cout << "Performing an average-reduction on shared nodes." << endl;
  int MAP_average_shared = COM_get_function_handle( "MAP.reduce_average_on_shared_nodes");
  COM_call_function( MAP_average_shared, &pid_hdl);
  
  if(myrank == 0)
    std::cout << "Updating ghost nodes." << endl;
  int MAP_update_ghost = COM_get_function_handle( "MAP.update_ghosts");
  COM_call_function( MAP_update_ghost, &pid_hdl);

  if(myrank == 0)
    std::cout << "finishing up window initialization" << endl;
  COM_window_init_done( wname.c_str());

  if(myrank == 0)
    std::cout << "loading Rocout" << endl;
  COM_load_module("SimOUT", "OUT");

  const string pconn = wname+".pconn";

  // Output smoothed mesh
  string newfile = "smoothed" + fname;
  if(myrank == 0)
    std::cout << "Outputting window into file " << newfile << endl;
  int OUT_set = COM_get_function_handle( "OUT.set_option");
  int OUT_write = COM_get_function_handle( "OUT.write_dataitem");
  COM_call_function( OUT_set, "mode", "w");
  int all_hdl = COM_get_dataitem_handle( (wname+".all").c_str());
  COM_call_function( OUT_write, newfile.c_str(), &all_hdl, 
		     (char*)wname.c_str(), "0000");
  
  COM_finalize();
}



