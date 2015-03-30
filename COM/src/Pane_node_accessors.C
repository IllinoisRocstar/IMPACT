//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "Pane_node_accessors.hpp"
#include "com_assertion.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

COM_BEGIN_NAME_SPACE

Pane_node_enumerator::
Pane_node_enumerator( const Pane *pane, int i,
			 const Connectivity *conn)
  : _pane(pane), _conn( _pane->is_structured()?NULL:
			(conn==NULL?pane->connectivity(i-1):conn)) 
{
  if ( _pane->is_unstructured()) {
	if (i == 0) {
		_node_num = 1;
		_size_of_nodes = _pane->size_of_nodes();
	}

  }
  else if ( _pane->dimension()==2) {
	ni = _pane->size_i();
	nj = _pane->size_j();
	nk=1;
	_buffer = _pane->size_of_ghost_layers() - i;
	_buffer = _buffer < 0 ? 0 : _buffer;
	_node_num = 1;
	set_first();
  } else  if ( _pane->dimension()==3) {
	ni = _pane->size_i();
	nj = _pane->size_i();
	nk = _pane->size_k();
	_buffer = _pane->size_of_ghost_layers() - i;
	_buffer = _buffer < 0 ? 0 : _buffer;
	_node_num = 1;
	set_first();
    } else {
    COM_assertion(false); // Not yet supported.
    }
}
/// Determine in the current node number is a real node or a ghost node
/// If it is a ghost node, return false.
bool Pane_node_enumerator::position_allowed3D(){
	int indx = _node_num - 1;
	if (indx < ni*nj*_buffer) //too low
		return false;
	else if (((indx%(ni*nj))/ni) < _buffer)  //too near top
		return false;
	     else if ( indx%ni < _buffer )  //too far left
		     return false;
	          else if ( indx%ni > (ni-_buffer) ) //too far right
			  return false; 
		       else if ( (indx%(ni*nj))/ni > (nk-_buffer) ) //too near bottom
			       return false;
		            else if  ( indx > ni*nj*(nk-_buffer) ) //too high
				    return false;
			    else return true;
}

/// Determine in the current node number is a real node or a ghost node
/// If it is a ghost node, return false.
bool Pane_node_enumerator::position_allowed2D(){
	int indx = _node_num - 1;
	if (((indx%(ni*nj))/ni) < _buffer)  //too near top
		return false;
	     else if ( indx%ni < _buffer )  //too far left
		     return false;
	          else if ( indx%ni > (ni-_buffer) ) //too far right
			  return false; 
		       else if ( (indx%(ni*nj))/ni > (nk-_buffer) ) //too near bottom
			       return false;

			    else return true;
}

/// Return the next allowed node number.  For structured meshes with ghost 
/// layers, one cannot number sequentially, so this method iterates to the 
/// next allowed node number
int Pane_node_enumerator::next() {

  if (_node_num == _size_of_nodes)
	  return _node_num;

  if (_pane->is_unstructured()) {
	  return _node_num++;
  }
  else if (_pane->dimension()==2) { // Structured mesh
    _node_num++;
    while (!position_allowed2D()){
	    _node_num++;
    }
    return _node_num;
  }
  else if (_pane->dimension()==3) {
  
    _node_num++;
    while (!position_allowed3D()){
	    _node_num++;
    }
    return _node_num++;
  }
  else {
      
    COM_assertion(false); // Not yet supported.
    return -1;
  }
}


COM_END_NAME_SPACE



