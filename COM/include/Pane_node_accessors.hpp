//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

/** \file Pane_node_accessors.h
 * Some definitions of classes for accessing data in a mesh.
 */

#ifndef _PANE_NODE_ACCESSORS_H_
#define _PANE_NODE_ACCESSORS_H_

#include "com.h"
#include <cassert>
#include <algorithm>

COM_BEGIN_NAME_SPACE

class Pane_node_enumerator {
public:
  Pane_node_enumerator() : _pane(NULL){}

  /// Constructor for an element in a structured or an unstructured mesh.
  /// If conn==NULL, then i is an element index local to the pane. 
  /// If conn!=NULL, then i is an element index local to the connectivity.
  Pane_node_enumerator( const Pane *pane, int i,
			   const Connectivity *conn=NULL);

  /// Go to the next element within the connectivity tables of a pane.
  int next();

  /// Get the dimension of the base pane.
  int dimension() const { return _pane->dimension(); }

  /// Get the local id of the element within the pane.
  int id() const {
	  return _node_num;
  }

  // Obtain a reference to the pane
  inline const Pane* pane() const { return _pane; }
  
protected:
  const Pane         *_pane;   // Owner pane.
  const Connectivity *_conn;
  bool position_allowed3D();
  bool position_allowed2D();
  int	_node_num;	//pane level id of node
  int   ni;
  int   nj;
  int   nk;
  int   _buffer;
  int   _size_of_nodes;
  void set_first() {
	  _node_num = ni*nj*_buffer + ni*_buffer + _buffer+1;
  }

};


COM_END_NAME_SPACE

#endif // _PANE_NODE_ENUMERATOR_H_
// EOF



