//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "Rocmap.h"
#include "com.h"
#include "Pane_connectivity.h"
#include "Pane_communicator.h"
#include "Pane_boundary.h"

MAP_BEGIN_NAMESPACE

// Compute pane connectivity map between shared nodes.
void Rocmap::compute_pconn( const COM::DataItem *mesh,
			    COM::DataItem *pconn) {
  // Compute the pane connectivity from scratch
  Pane_connectivity pc( mesh, mesh->window()->get_communicator());

  pc.compute_pconn( pconn);
}

// Determine the nodes at pane boundaries of a given mesh
void Rocmap::pane_border_nodes( const COM::DataItem *mesh,
				COM::DataItem *isborder, 
				int *ghost_level) {
  Pane_boundary::determine_borders( mesh, isborder, ghost_level?*ghost_level:0);
}

// Get the number of communicating panes.
void Rocmap::size_of_cpanes( const COM::DataItem *pconn, const int *pane_id,
			     int *npanes_total, int *npanes_ghost) {

  Pane_connectivity::size_of_cpanes(pconn, pane_id, npanes_total, npanes_ghost);
}

// Perform an average-reduction on the shared nodes for the given dataitem.
void Rocmap::reduce_average_on_shared_nodes(COM::DataItem *att, 
					    COM::DataItem *pconn){
  Pane_communicator pc(att->window(), att->window()->get_communicator());
  pc.init(att,pconn);
  pc.begin_update_shared_nodes();
  pc.reduce_average_on_shared_nodes();
  pc.end_update_shared_nodes();
}

// Perform an average-reduction on the shared nodes for the given dataitem.
void Rocmap::reduce_minabs_on_shared_nodes(COM::DataItem *att, 
					   COM::DataItem *pconn){
  Pane_communicator pc(att->window(), att->window()->get_communicator());
  pc.init(att,pconn);
  pc.begin_update_shared_nodes();
  pc.reduce_minabs_on_shared_nodes();
  pc.end_update_shared_nodes();
}

// Perform a maxabs-reduction on the shared nodes for the given dataitem.
void Rocmap::reduce_maxabs_on_shared_nodes(COM::DataItem *att,
					   COM::DataItem *pconn){
  Pane_communicator pc(att->window(), att->window()->get_communicator());
  pc.init(att,pconn);
  pc.begin_update_shared_nodes();
  pc.reduce_maxabs_on_shared_nodes();
  pc.end_update_shared_nodes();
}

// Update ghost nodal or elemental values for the given dataitem.
void Rocmap::update_ghosts(COM::DataItem *att,
			   const COM::DataItem *pconn){
  std::cout << "Updating ghost nodes for dataitem " << att->name() << std::endl;   
  Pane_communicator pc(att->window(), att->window()->get_communicator());
  pc.init(att, pconn);

  // MS: following lines cause memory leak issue with rocburn
  if (att->is_elemental()){
    std::cout << __FILE__ << __LINE__ << std::endl;
    pc.begin_update_ghost_cells();
    pc.end_update_ghost_cells();
  }
  else{
    std::cout << __FILE__ << __LINE__ << std::endl;
    pc.begin_update_ghost_nodes();
    pc.end_update_ghost_nodes();
  }
}

void Rocmap::load( const std::string &mname) {
  COM_new_window( mname.c_str());

  COM_Type types[4];
  types[0] = COM_METADATA;
  types[1] = COM_METADATA;
  COM_set_function( (mname+".reduce_average_on_shared_nodes").c_str(),
		    (Func_ptr)reduce_average_on_shared_nodes, "iI", types);

  COM_set_function( (mname+".reduce_maxabs_on_shared_nodes").c_str(),
		    (Func_ptr)reduce_maxabs_on_shared_nodes, "iI", types);

  COM_set_function( (mname+".reduce_minabs_on_shared_nodes").c_str(),
		    (Func_ptr)reduce_minabs_on_shared_nodes, "iI", types);

  COM_set_function( (mname+".update_ghosts").c_str(),
		    (Func_ptr)update_ghosts, "iI", types);

  types[0] = types[1] = COM_METADATA;
  COM_set_function( (mname+".compute_pconn").c_str(), 
		    (Func_ptr)compute_pconn, "io", types);
  
  types[0] = types[1] = COM_METADATA; types[2] = COM_INT;
  COM_set_function( (mname+".pane_border_nodes").c_str(), 
		    (Func_ptr)pane_border_nodes, "ioI", types);
  
  types[0] = COM_METADATA;
  types[1] = types[2] = types[3] = COM_INT;
  COM_set_function( (mname+".size_of_cpanes").c_str(), 
		    (Func_ptr)size_of_cpanes, "iioO", types);
  
  COM_window_init_done( mname.c_str());
}

void Rocmap::unload( const std::string &mname) {
  COM_delete_window( mname.c_str());
}

extern "C" void SurfMap_load_module( const char *mname) 
{ Rocmap::load( mname); }

extern "C" void SurfMap_unload_module( const char *mname) 
{ Rocmap::unload( mname); }


// Fortran bindings
extern "C" void surfmap_load_module( const char *mname, long int length) 
{ Rocmap::load( std::string(mname, length)); }

extern "C" void surfmap_unload_module( const char *mname, long int length) 
{ Rocmap::unload( std::string(mname, length)); }

extern "C" void SURFMAP_LOAD_MODULE( const char *mname, long int length) 
{ Rocmap::load( std::string(mname, length)); }

extern "C" void SURFMAP_UNLOAD_MODULE( const char *mname, long int length) 
{ Rocmap::unload( std::string(mname, length)); }

extern "C" void surfmap_load_module_( const char *mname, long int length) 
{ Rocmap::load( std::string(mname, length)); }

extern "C" void surfmap_unload_module_( const char *mname, long int length) 
{ Rocmap::unload( std::string(mname, length)); }

extern "C" void SURFMAP_LOAD_MODULE_( const char *mname, long int length) 
{ Rocmap::load( std::string(mname, length)); }

extern "C" void SURFMAP_UNLOAD_MODULE_( const char *mname, long int length) 
{ Rocmap::unload( std::string(mname, length)); }

MAP_END_NAMESPACE



