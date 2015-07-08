//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "RFC_Window_overlay.h"
#include "Overlay_primitives.h"
#include "HDS_accessor.h"
#include "Triangulation.h"
#include <functional>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

RFC_BEGIN_NAME_SPACE

using  namespace std;

static HDS_accessor acc;

const static int scheme = Overlay_primitives::ANGLE_WEIGHTED;

RFC_Pane_overlay::RFC_Pane_overlay( COM::Pane *b, int color)
  : Base(b, color) {}

// Constructor and deconstructors
RFC_Window_overlay::RFC_Window_overlay( COM::Window *b, 
					int color, const char *pre)
  : Base(b, color, MPI_COMM_SELF), out_pre(pre?pre:""), 
    _long_falseness_check(true) {

  init_feature_parameters();
  vector< Pane*> pns; panes(pns);
  vector< Pane*>::iterator pit=pns.begin(), piend=pns.end();
  for ( ; pit != piend; ++pit) {
    (*pit)->_window = this;
  }

  Window_manifold_2::init(b->dataitem(COM::COM_PMESH));

  // Preprocess pane connectivity.
  std::cout << "Building pane connectivity..." << std::flush;

  build_pc_tables();

  // Determine the counterpart for border edges of all panes.
  determine_counterparts();

  std::cout << "Done" << std::endl;
}

RFC_Window_overlay::~RFC_Window_overlay() {
  for (int i=_pms.size()-1; i>=0; --i)
    _pms[i] = NULL;
}

HEdge RFC_Window_overlay::get_an_unmarked_halfedge() const {
  HEdge hbrd;

  for ( Pane_set::const_iterator pit=_pane_set.begin(); 
	pit != _pane_set.end(); ++pit) {
    RFC_Pane_overlay &pane = reinterpret_cast<RFC_Pane_overlay &>(*pit->second);

    for (int i=1, s=pane.size_of_faces(); i<=s; ++i) {
      HEdge h( &pane, Edge_ID(i,0)), h0=h;
      do {
        if (h.opposite_l().is_border_l()) continue; 

        if (!pane.marked( h)) {
          if ( hbrd.pane()==NULL && pane.is_on_feature( h.origin_l()))
            hbrd = h;
          else if ( !pane.is_on_feature(h.origin_l())) 
            return h;
        }
      } while ( (h=h.next_l()) != h0);
    }
  }
  return hbrd;
}

/** Check whether given two vertices correspond to the same physical point. */
bool 
RFC_Window_overlay::is_same_node( const Node &v1, const Node &v2) {
  if ( v1==v2) return true;
  if ( !v1.is_border_l() || !v2.is_border_l()) 
    return false;
  return v1.get_primary() == v2.get_primary();
}

// Evaluate normals for every vertex within a pane as the average of
// face normals of the faces of its incident faces.
void
RFC_Pane_overlay::evaluate_normals() {
  _nrmls.resize( size_of_nodes(), Vector_3(0.,0.,0.));

  for ( int vid=1, s=size_of_nodes(); vid<=s; ++vid) {
    Node v( this, vid);
    if (v.is_isolated()) continue;
    Vector_3 d(0.,0.,0.);

    HEdge h = v.halfedge_l(), i = h;
    if ( h.destination_l() != v) continue;

    do {
      RFC_assertion( i.destination_l() == v);
      if ( !i.is_border_l())
	d += Overlay_primitives().get_face_normal( i, Point_2(0,0), scheme);
      
      if ( i.opposite_l().is_border_l()) break;
    } while ( (i = i.opposite_l().prev_l()) != h);

    // Note that the normal vector is not normalized.
    set_normal( vid, d);
  }
}

// Evaluate vertex normals as average of face normals of all incident faces.
void 
RFC_Window_overlay::evaluate_normals() {
  //  std::cout << "eval normals" << std::endl;
  // First, evaluate normals for each pane
  Pane_set::iterator it, iend=_pane_set.end();
  for ( it=_pane_set.begin() ; it != iend; ++it) {
    if ( it->second->is_master())
      ((RFC_Pane_overlay*)it->second)->evaluate_normals();
  }

  //  std::cout << "reducing" << std::endl;
  // Second, perform reduction over all vertices
  reduce_normals_to_all( MPI_SUM);
  //  std::cout << "reducing done" << std::endl;

  // Evaluate the one-sided normals for the halfedge edges incident 
  // on sharp features.
  for ( it=_pane_set.begin() ; it != iend; ++it) {
    RFC_Pane_overlay &pane = (RFC_Pane_overlay &)*it->second;
    free_vector( pane._f_nrmls); free_vector( pane._f_tngts);
    free_vector( pane._f_n_index); free_vector( pane._f_t_index);
    int num_hedgs=4*pane.size_of_faces() + pane.size_of_border_edges(); // pane.hds().size_of_halfedges();
    pane._f_n_index.resize( num_hedgs, -1);
    pane._f_t_index.resize( num_hedgs, -1);
  }

  Overlay_primitives      op;
  Point_2  one(1,0);
  // Loop throught the sharp halfedges in 1-features
  for ( Feature_list_1::iterator it = _f_list_1.begin(), iend=_f_list_1.end();
	it != iend; ++it) {
    for ( Feature_1::iterator j=it->begin(); j!=it->end(); ++j) {
      HEdge k = *j, kopp=k.opposite_g();
      do {
	if (!k.is_border_l() && k.pane()->
	    _f_n_index[k.pane()->get_index(k)] == -1) {
	  // First, compute the vector
	  Vector_3 v = op.get_face_normal( k, one, scheme);
	  HEdge h0 = k, h = acc.get_next_around_destination(k);
	  do {
	    if ( is_feature_1( h)) break;
	    RFC_assertion( !h.is_border_l());
	    v += op.get_face_normal( h, one, scheme);
	  } while ( (h=acc.get_next_around_destination(h)) != h0);

	  // Second, insert the vector into the table
	  normalize(v);
	  swap( h, h0);
	  do {
	    RFC_Pane_overlay &pane = *h.pane();
	    if ( pane._f_nrmls.size() && v == pane._f_nrmls.back()) {
	      pane._f_n_index[ pane.get_index( h)] = pane._f_nrmls.size()-1;
	    }
	    else {
	      pane._f_n_index[ pane.get_index( h)] = pane._f_nrmls.size();
	      pane._f_nrmls.push_back( v);
	    }
	  } while ( (h=acc.get_next_around_destination(h)) != h0);
	}
      } while ( (k=(k==kopp)?*j:kopp) != *j);
      
      // Compute the tangent
      Vector_3 tng = ( k.destination_l().point() - kopp.destination_l().point());
      k.pane()->add_tangent( k, tng);
      if ( !is_feature_0( k.destination_g())) {
	Feature_1::iterator t = j; ++t;
	if ( t==it->end()) t = it->begin();
	HEdge h = (*t).opposite_g();
	h.pane()->add_tangent( h, tng);
      }
      kopp.pane()->add_tangent( kopp, tng);
      if ( !is_feature_0( kopp.destination_g())) {
	Feature_1::iterator t = (j==it->begin()) ? it->end() : j; 
	HEdge h = *(--t);
	h.pane()->add_tangent( h, tng);
      }
    }
  }

  // Loop through the vertices in 0-features
  for ( Feature_list_0::iterator it = _f_list_0.begin(), iend=_f_list_0.end();
	it != iend; ++it) {
    HEdge h = it->vertex().halfedge_g();
    RFC_Pane_overlay &pane = *h.pane();

    if ( !h.is_border_l() && pane._f_n_index[ pane.get_index( h)] == -1) {
      HEdge h0 = h;
      do {
	Vector_3 v = op.get_face_normal( h, one, scheme)+
	  op.get_face_normal( h.opposite_g(), Point_2(0,0), scheme);
	normalize(v);

	if ( pane._f_nrmls.size() && v == pane._f_nrmls.back()) {
	  pane._f_n_index[ pane.get_index( h)] = pane._f_nrmls.size()-1;
	}
	else {
	  pane._f_n_index[ pane.get_index( h)] = pane._f_nrmls.size();
	  pane._f_nrmls.push_back( v);
	}
      } while ( (h=acc.get_next_around_destination(h)) != h0);
    }
  }

  // Finally, normalize normals and tangents
  for ( it=_pane_set.begin() ; it != iend; ++it) {
    RFC_Pane_overlay &pane = (RFC_Pane_overlay &)*it->second;

    if ( pane.is_master()) {
      for ( int i=0, size = pane._nrmls.size(); i<size; ++i) 
	normalize( pane._nrmls[i]);
      for ( int i=0, size = pane._f_tngts.size(); i<size; ++i) 
	normalize( pane._f_tngts[i]);
    }
  }
  //  std::cout << "eval normals done." << std::endl;
}

// Get the normal of a vertex.
const Vector_3 &RFC_Pane_overlay::get_normal( HEdge h, const Node &v) const { 
  RFC_assertion( h.pane() == this && v.pane()==this);

  if ( v != h.destination_l()) {
    h = h.prev_l();
    RFC_assertion( v == h.destination_l());
  }
  if ( h.id().is_border()) 
    h = h.opposite_g().prev_l();

  int j=_f_n_index[get_index( h)];
  if ( j < 0)
    return _nrmls[ get_index(v)];
  else
    return _f_nrmls[j];
}

// Get the tangents of a vertex on feature. 
// Note that the tangent is always along the direction of
// the halfedges in the feature list.
const Vector_3&RFC_Pane_overlay::get_tangent( HEdge h, const Node &v) const {
  if ( v != h.destination_l()) {
    h = h.opposite_g();
    return h.pane()->get_tangent( h, h.destination_l());
  }
  RFC_assertion( is_feature_1(h));
  
  int j=_f_t_index[get_index( h)];
  RFC_assertion( j >= 0 && int(_f_tngts.size())>j);
  return _f_tngts[j];
}

// Determine counterparts of border edges.
void
RFC_Pane_overlay::determine_counterparts() {
  typedef set< pair<int,int> >      V2h;
  typedef map<int, V2h>             V2h_table;
  V2h_table  _v2h_table;

  //`Loop through the B2v_table
  for ( B2v_table::iterator 
	  it=_b2v_table.begin(), iend=_b2v_table.end(); it != iend; ++it) {
    for ( int i=0, size=it->second.size(); i<size; ++i) {
      pair<int,int> p(it->first, i);
      if ( it->first==id()) {
	if ( i&1) --p.second; else ++p.second;
      }
      _v2h_table[it->second[i]].insert( p);
    }
  }

  _cntrs_hedge.resize( size_of_border_edges());

  // Loop through all border edges.
  for ( int i=0; i<size_of_border_edges(); ++i) {
    HEdge h( this, Edge_ID( i+1, Edge_ID::BndID));
    
    Node v = h.destination_l();
    V2h_table::iterator it = _v2h_table.find( get_lid(v));
    if ( it == _v2h_table.end()) continue;

    V2h_table::iterator i0=_v2h_table.find(get_lid( h.origin_l()));

    // If the origin not incident with any, there is no counterpart.
    if ( i0==_v2h_table.end()) continue;

    // check whether the two vertices are overlapping.
    V2h::iterator j0=i0->second.begin(), j1=it->second.begin();

    while ( j0 != i0->second.end() && j1 != it->second.end()) {
      if ( j0->first == j1->first) {
	bool found=false;
	V2h::iterator k0, k1;
	for ( k0=j0; k0!=i0->second.end()&&k0->first==j0->first; ++k0) {
	  if (found) continue;
	  for ( k1=j1; k1!=it->second.end()&&k1->first==j1->first; ++k1) {
	    if (found) continue;
	    // Check whether it is an edge in that pane
	    const RFC_Pane_overlay &p = _window->pane(k0->first);
	    const B2v  &b2v = p._b2v_table.find(id())->second;
	    pair<int,int> e( b2v[k0->second], b2v[k1->second]);
	    RFC_assertion( e.first>0 && e.second>0);

	    map< pair<int,int>, HEdge>::const_iterator 
              b2e_it = p._bv2edges.find( e);
	    
	    if ( b2e_it != p._bv2edges.end()) {
	      _cntrs_hedge[i] = b2e_it->second;
	      found=true;
	    }
	    else if ( k0->first != id()) {
	      swap( e.first, e.second);
	      if (p._bv2edges.find( e) != p._bv2edges.end()) {
		std::cerr << "ERROR: Inconsistency in window \"" 
			  << _window->name() << "\"!!!! \n\tPanes " << id()
			  << " and " << j0->first 
			  << " have opposite orientation!" << std::endl
			  << "\tComputation can not continue. Aborting..."
			  << std::endl;
		RFC_assertion( p._bv2edges.find( e) == p._bv2edges.end()); 
		abort();
	      }
	    }
	  }
	}
	if ( found) { j0 = k0; j1 = k1; }
	else { ++j0; ++j1; }
      }
      else 
	if ( j0->first < j1->first) ++j0; else ++j1;
    }
  }

  // Fix physical border ones to have them point to themselves
  for ( int i=0; i<size_of_border_edges(); ++i) {
    HEdge h( this, Edge_ID( i+1, Edge_ID::BndID));
    
    if ( _cntrs_hedge[i].pane() == NULL) _cntrs_hedge[i] = h;
  }

  // Determine the primaries of the border vertices
  _primnodes.resize( size_of_border_edges());  
  for ( int i=1; i<=size_of_border_edges(); ++i) {
    HEdge h( this, Edge_ID( i, Edge_ID::BndID));

    int vid = get_lid( h.destination_l());
    pair<Self*,int> t=_window->get_primary( this, vid);

    Node n(this, vid);
    _primnodes[ get_border_index( n.halfedge_l())] = 
      t.first->get_node_from_id( t.second);
  }
}

void
RFC_Pane_overlay::construct_bvpair2edge() {
  for ( int i=1, s=size_of_border_edges(); i<=s; ++i) {
    HEdge h = HEdge( this, Edge_ID(i,Edge_ID::BndID)).opposite_l();

    _bv2edges[ make_pair( get_lid( h.origin_l()), 
                          get_lid( h.destination_l()))] = h;
  }
}

// Determine the counterparts for all pane border edges.
void
RFC_Window_overlay::determine_counterparts() {
  // Construct a mapping from vertex pair to border edges
  Pane_set::iterator it=_pane_set.begin(), iend=_pane_set.end();
  for (; it != iend; ++it) 
    ((RFC_Pane_overlay*)it->second)->construct_bvpair2edge();

  // Construct mapping from panal border edges to their counterpart.
  for ( it = _pane_set.begin(); it != iend; ++it)
    ((RFC_Pane_overlay*)it->second)->determine_counterparts();

  // Clear the mapping
  for ( it = _pane_set.begin(); it != iend; ++it) 
    ((RFC_Pane_overlay*)it->second)->_bv2edges.clear();
}

void
RFC_Window_overlay::unmark_alledges( ) {
  Pane_set::iterator it=_pane_set.begin(), iend=_pane_set.end();
  for ( ; it != iend; ++it) {
    RFC_Pane_overlay &p = (RFC_Pane_overlay &)*it->second;
    fill( p._e_marks.begin(), p._e_marks.end(), 0);
  }
}

// Functions for supporting the overlay algorithm
void RFC_Pane_overlay::create_overlay_data() { 
  _v_nodes.resize(0); 
  _e_node_list.resize(0);
  _e_node_buf.resize(0);
  _e_marks.resize(0);

  _v_nodes.resize( size_of_nodes(),0);

  int n=4*size_of_faces() + size_of_border_edges();
  _e_node_list.resize( n, INode_list( color()));
  _e_node_buf.resize( n, 0);
  _e_marks.resize( n, 0);
}

// Functions for supporting the overlay algorithm
void RFC_Pane_overlay::delete_overlay_data() { 
  free_vector( _v_nodes);
  free_vector( _e_node_list);
  free_vector( _e_node_buf);
  free_vector( _e_marks);

  free_vector( _cntrs_hedge); free_vector( _primnodes);
  free_vector( _f_nrmls); free_vector( _f_tngts);
  free_vector( _f_n_index); free_vector( _f_t_index);
  free_vector( _ad_0); free_vector( _ea_0); free_vector( _fd_1);
}

void RFC_Window_overlay::create_overlay_data() {
  // Loop through panes
  Pane_set::iterator pit=_pane_set.begin(), piend=_pane_set.end();
  for ( ; pit != piend; ++pit) {
    RFC_Pane_overlay &p = (RFC_Pane_overlay &)*pit->second;
    p.create_overlay_data();
  }

}

void RFC_Window_overlay::delete_overlay_data() {
  // Loop through the panes to remove the buffer spaces
  Pane_set::iterator pit, piend;
  for (pit=_pane_set.begin(), piend=_pane_set.end(); pit != piend; ++pit) {
    RFC_Pane_overlay &pane = (RFC_Pane_overlay &)*pit->second;
    if ( !pane.is_master()) continue;

    pane.delete_overlay_data();
  }
}

/*! \param idx the index of the subface (i.e., starting from 0).
 *  \param plid the local id of the parent face of the subface.
 *  \param h an halfedge of the its parent face in the pane.
 *  \param lids the local ids of the nodes of the subface.
 *  \param nc the local coordinates of the nodes of the subface within its 
 *            parent face.
 *  \param rp_id the pane id of the correponding subface in the other window.
 *  \param cnt the local id of the subface's counterpart in the other window.
 *  \param rids the local ids of the nodes of the correponding subface in the
 *            other window.
 */
void RFC_Pane_overlay::insert_subface(int idx, int plid, const int *lids, 
				      const ParentEdge_ID *eids, const Point_2 *nc,
				      int rp_id, int cnt, const int *rids) {
  RFC_assertion( idx>=0 && idx<int(_subface_parents.size()) && cnt>=1);
  for ( int i=0; i<3; ++i) {
    _subfaces[idx][i] = lids[i];
    _subnode_parents[ lids[i]-1] = eids[i]; 
    _subnode_nat_coors[ lids[i]-1] = Point_2S(nc[i][0],nc[i][1]);
    _subnode_counterparts[ lids[i]-1] = Node_ID( rp_id, rids[i]);
  }
  _subface_parents[idx] = plid;
  _subface_counterparts[idx] = Face_ID( rp_id, cnt);
}

/*!
  Reduces values for each node form all its instances to all instances. 
*/
void 
RFC_Window_overlay::reduce_normals_to_all( MPI_Op op) {
  //  std::cout << "reduce enter" << std::endl;
  std::vector<void *> ptrs;
  // Loop through panes
  Pane_set::iterator pit=_pane_set.begin(), piend=_pane_set.end();
  for ( pit=_pane_set.begin(); pit != piend; ++pit) {
    RFC_Pane_overlay &pane=(RFC_Pane_overlay &)*pit->second;

    ptrs.push_back( &pane._nrmls[0]);
  }
  //  std::cout << "loop done." << std::endl;
  _map_comm.init( &ptrs[0], COM_DOUBLE, 3);
  //  std::cout << "init done." << std::endl;
  _map_comm.begin_update_shared_nodes();
  //  std::cout << "begin update done." << std::endl;
  _map_comm.reduce_on_shared_nodes( op);
  //  std::cout << "rosn done." << std::endl;
  _map_comm.end_update_shared_nodes();
  //  std::cout << "reduce done." << std::endl;
}

RFC_END_NAME_SPACE

