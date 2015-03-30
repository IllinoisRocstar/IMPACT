//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

//==========================================================
// The implementation of the overlay algorithm.
//
// Author: Xiangmin Jiao
//==========================================================

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <queue>
#include <set>
#include <utility>
#include "Timing.h"
#include "Overlay.h"

RFC_BEGIN_NAME_SPACE

// The initialization for the overlay algorithm. It locates the
//     green parent of a blue vertex and create an inode for it.
//     This initialization step takes linear time.
// Returns NULL if unsuccessful. Otherwise, creates a new INode.
INode *Overlay::overlay_init() {
  INode *v=NULL;

  // get an unmarked halfedge from the blue mesh and take its origin as x.
  HEdge b=B->get_an_unmarked_halfedge(), g;
  if ( b.pane()==NULL) return NULL;

  Node x = b.origin_l();

  // locate the green parent of x.
  Parent_type t;
  Point_2    nc;
  get_green_parent( x, b, &g, &t, &nc);
  RFC_assertion ( t != PARENT_NONE && g.pane() != NULL);

  // create a new inode for x
  v = new INode();
  v->set_parent( b, Point_2(0,0), BLUE);
  v->set_parent( g, nc, GREEN);

  if ( verbose2) {
    std::cout << "\nFound a seed at blue vertex (" 
	      << x.pane()->get_point(x) << ") \nin green ";
    const RFC_Pane_overlay *gpane=g.pane();
    switch ( t) {
    case PARENT_VERTEX:
      std::cout << "vertex (" << gpane->get_point( g.origin_l()) << ")\n";
      break;
    case PARENT_EDGE:
      std::cout << "edge (" << gpane->get_point( g.origin_l()) << "), (" 
		<< gpane->get_point( g.destination_l()) << ")\n";
      break;
    case PARENT_FACE: {
      std::cout << "face ";
      HEdge gt = g;
      do {
	std::cout << "\t(" << gpane->get_point( gt.origin_l()) << "), ";
      }	while ( (gt = gt.next_l()) != g);
      std::cout << std::endl;
      break;
    }
    default:
      RFC_assertion( false);
    }
    std::cout << std::endl;
  }

  return v;
}

// Get the green parent of a vertex v. This subroutine takes
//   linear time in terms of the number of vertices in G.
void Overlay::
get_green_parent( const Node &v,
		  const HEdge &b,
		  HEdge *h_out,
		  Parent_type  *t_out,
		  Point_2      *nc) 
{
  const Point_3 &p = v.pane()->get_point(v);
  Real  sq_dist = 1.e30;
  Node  w;

  //=================================================================
  // Locate the closest green vertex
  //=================================================================
  std::vector<RFC_Pane_overlay*>   ps;
  G->panes( ps);
  // Loop through all the panes of G
  for ( std::vector<RFC_Pane_overlay*>::iterator 
	  pit=ps.begin(); pit != ps.end(); ++pit) {
    RFC_Pane_overlay *pane = *pit;
    // loop through all the green vertices that are complete
    for ( int i=1; i<=pane->size_of_nodes(); ++i) {
      Node n(pane, i);

      if ( !n.is_isolated() && n.halfedge_l().destination_l() == n) {
	// if the distance is closer than previous ones, save it
	Real sq_d = ( p- pane->get_point(n)).squared_norm();
	if ( sq_d < sq_dist) { sq_dist = sq_d;  w = n; }
      }
    }
  }

  if ( w.pane() == NULL) return; 

  RFC_assertion( w.is_primary()); // Because we start from smaller ids.
  // Let h be a nonborder incident halfedge of w
  HEdge h = w.halfedge_g();
  h = !h.is_border_g() ? h.next_g() : h.opposite_g();

  // We perform breadth-first search starting from h
  std::queue< HEdge> q;
  std::list< HEdge>  hlist;

  q.push( h);
  // Mark the halfedges in the same face as h
  HEdge h0 = h;
  do { RFC_assertion( !acc.marked(h)); acc.mark( h); hlist.push_back( h); } 
  while ( (h=h.next_g()) != h0);

  Vector_3 vec(0.,0.,0.);
  while ( !q.empty()) {
    h = q.front(); q.pop();
    
    *t_out = PARENT_NONE; *h_out=h;
    if ( op.project_onto_element( p, h_out, t_out, vec, nc, eps_e) &&
	 std::abs(acc.get_normal(b)* acc.get_normal(h)) >= 0.6)
      break;

    // Insert the incident faces of h into the queue
    h0 = h;
    do {
      HEdge hopp=h.opposite_g();
      if ( !hopp.is_border_g() && ! acc.marked( hopp)) {
	HEdge h1 = hopp;
	q.push( h1);
	do { 
	  acc.mark(h1); hlist.push_back( h1); 
	} while ((h1=h1.next_g())!=hopp);
      }
    } while ( (h=h.next_g()) != h0);
  }
  // Unmark the halfedges
  while ( !hlist.empty()) 
  { acc.unmark( hlist.front()); hlist.pop_front(); }

  // If v is too far from p_out, return NULL.
  vec = op.get_point( *h_out, *nc) - p;
  if ( vec * vec > sq_length( *h_out) + sq_length( v.halfedge_l()))  {
    *h_out = HEdge(); *t_out = PARENT_NONE;
  }
  else {
    // Determine whether the two surfaces are facing each other
    is_opposite = ( b.pane()->get_normal( b, v) * 
		    op.get_face_normal( *h_out, *nc)) < 0;
  }
}

RFC_END_NAME_SPACE


