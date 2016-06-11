/* *******************************************************************
 * Illinois Open Source License                                      *
 *                                                                   *
 * University of Illinois/NCSA                                       * 
 * Open Source License                                               *
 *                                                                   *
 * Copyright@2008, University of Illinois.  All rights reserved.     *
 *                                                                   *
 *  Developed by:                                                    *
 *                                                                   *
 *     Center for Simulation of Advanced Rockets                     *
 *                                                                   *
 *     University of Illinois                                        *
 *                                                                   *
 *     www.csar.uiuc.edu                                             *
 *                                                                   *
 * Permission is hereby granted, free of charge, to any person       *
 * obtaining a copy of this software and associated documentation    *
 * files (the "Software"), to deal with the Software without         *
 * restriction, including without limitation the rights to use,      *
 * copy, modify, merge, publish, distribute, sublicense, and/or      *
 * sell copies of the Software, and to permit persons to whom the    *
 * Software is furnished to do so, subject to the following          *
 * conditions:                                                       *
 *                                                                   *
 *                                                                   *
 * @ Redistributions of source code must retain the above copyright  * 
 *   notice, this list of conditions and the following disclaimers.  *
 *                                                                   * 
 * @ Redistributions in binary form must reproduce the above         *
 *   copyright notice, this list of conditions and the following     *
 *   disclaimers in the documentation and/or other materials         *
 *   provided with the distribution.                                 *
 *                                                                   *
 * @ Neither the names of the Center for Simulation of Advanced      *
 *   Rockets, the University of Illinois, nor the names of its       *
 *   contributors may be used to endorse or promote products derived * 
 *   from this Software without specific prior written permission.   *
 *                                                                   *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,   *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   *
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          *
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE CONTRIBUTORS OR           *
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       * 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   *
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE    *
 * USE OR OTHER DEALINGS WITH THE SOFTWARE.                          *
 *********************************************************************
 * Please acknowledge The University of Illinois Center for          *
 * Simulation of Advanced Rockets in works and publications          *
 * resulting from this software or its derivatives.                  *
 *********************************************************************/
// $Id: Overlay.C,v 1.28 2011/05/24 16:43:49 mtcampbe Exp $

//==========================================================
// The implementation of the overlay algorithm.
//
// Author: Xiangmin Jiao
// Last modified:   06/23/2006 (Added implicit feature matching.)
//
//==========================================================

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <queue>
#include <set>
#include <utility>
#include <cstdio>
#include <cmath>
#include "Timing.h"
#include "Overlay.h"

RFC_BEGIN_NAME_SPACE

Overlay::Overlay( const COM::Window *w1, const COM::Window *w2, 
		  const char *pre)
  : op(1.e-9, 1.e-6, 1.e-2), is_opposite(true), 
    verbose(true), verbose2(false), out_pre(pre?pre:""),
    eps_e( 1.e-2), eps_p(1.e-6) {
  B = new RFC_Window_overlay( const_cast<COM::Window*>(w1), BLUE,  
			      out_pre.c_str());
  G = new RFC_Window_overlay( const_cast<COM::Window*>(w2), GREEN, 
			      out_pre.c_str());
  
#if DEBUG
  verbose2 = true;
#endif
}

Overlay::~Overlay() {
  delete G;
  delete B; 
}

void Overlay::set_tolerance( double tol) {
  COM_assertion_msg( tol<=0.2, "Tolerance too large");

  eps_e = std::max(tol, 1.e-3); // Set snap tolerance.
}

// This function is the main interface for the overlay algorithm. 
int Overlay::overlay() {
  Bbox_3 bbox=B->get_bounding_box(), gbox = G->get_bounding_box();

  std::cout << "\tMesh " << B->name() << ": " 
	    << B->size_of_nodes() << " vertices, "
	    << B->size_of_faces() << " faces, with bounding box "
	    << bbox << "\n\tMesh " << G->name() << ": "
	    << G->size_of_nodes() << " vertices, "
	    << G->size_of_faces() << " faces, with bounding box "
	    << gbox << std::endl;

  double tol_high = 3.e-1, tol_low=1.e-1;
 
  // First, check whether the scales of the two bounding boxes are
  // the same by comparing the total lengths of bounding boxes and 
  // the individual dimensions.
  double bl = (bbox.xmax()-bbox.xmin()) + 
    (bbox.ymax()-bbox.ymin()) + (bbox.zmax()-bbox.zmin());
  double gl = (gbox.xmax()-gbox.xmin()) + 
    (gbox.ymax()-gbox.ymin()) + (gbox.zmax()-gbox.zmin());

  if ( bl>gl*(1+tol_high) || gl>bl*(1+tol_high) || 
       !bbox.do_match( gbox, std::min(bl,gl)*tol_high)) {
    std::cerr << "ERROR: The bounding boxes differ by more than "
	      << tol_high*100 << "%. Please check the geometries. Stopping..." 
	      << std::endl;
    abort();
  }

  // If the bounding boxes do not match at all sides within a fraction 
  // of the longest dimension, then stop the code. 
  // the difference between the two boxes is smaller than a fraction (eps)
  // of the largest dimension of the two boxes.
  if ( !bbox.do_match( gbox, std::min(bl,gl)*tol_low)) {
    std::cerr << "WARNING: The bounding boxes differ by more than "
	      << tol_low*100 << "% but less than "
	      << tol_high*100 << "%. Continuing anyway." << std::endl;
  }

  double t0 = get_wtime();

  // Create helper data in two input meshes.
  B->create_overlay_data();
  G->create_overlay_data();

  // Detect the 0-features of the two meshes.
  std::cerr << "Detecting features in " << B->name() << "..." << std::endl;
  B->detect_features();

  std::cerr << "Detecting features in " << G->name() << "..." << std::endl;
  G->detect_features();
  
  match_features_0();

  // Evaluate normals for the nodes.
  B->evaluate_normals();
  G->evaluate_normals();

  std::cout << "Performing mesh overlay..." << std::flush;
  // Step 1: Project the blue vertices onto G and
  // compute the intersection points of blue edges with green
  //  edges, and insert the intersections into the blue edges on the fly.
  intersect_blue_with_green();
  std::cout << "..." << std::flush;

  // Step 2: Sort the intersection points corresponding to each green 
  //   edge and insert them into green edges (in linear time).
  sort_on_green_edges();
  std::cout << "..." << std::flush;

  // Step 3: Compute the projection of the green vertices on B.
  associate_green_vertices();
  std::cout << "..." << std::flush;
  
  if ( verbose) {
    std::cout << "\n\tLocated " << inodes.size() 
	      << " edge intersections.\n\tDone in " 
	      << get_wtime()-t0 << " sec.\n"
	      << "Exporting the subdivision..." << std::flush;
    t0 = get_wtime();
  }
  
  // Finally, assign local ids to the subnodes within panes and 
  number_subnodes();

  // subdivide the faces and assign local ids to these subfaces,and
  // create the nodal and face lists for the panes.
  number_subfaces();

  // Now, clean up the overlay data
  // Destroy helper data in the input windows
  B->delete_overlay_data(); G->delete_overlay_data();
  while ( !inodes.empty()) {
    INode* x = inodes.front(); inodes.pop_front();
    delete x; 
  }

  std::cout << "Done";
  if ( verbose) {
    std::cout << " in " << get_wtime()-t0 << " sec.";
  }
  std::cout << std::endl;
  return 0;
}

Real Overlay::sq_length( const HEdge &h) const {
  return (h.origin_l().point() - h.destination_l().point()).squared_norm();
}

// This subroutine corrects the green parent of an inode i by intersecting 
//    it with the previous assigned green parent.
// TODO: Currently does not resolve inconsistencies involving multiple 
//       blue edges. Needs to add support for it.
void Overlay::
insert_node_in_blue_edge( INode &x, const HEdge &b1) {
  HEdge b = x.halfedge( BLUE);
  HEdge g = x.halfedge( GREEN); RFC_assertion( !g.is_border_g());

  // Step 1, verify that x is not at a green vertex that already has
  //    an inode associated with. If so, merge the two inodes by making
  //    its blue parent the intersection of the two blue edges/vertices,
  //    and delete the old one.
  if ( x.parent_type( GREEN) == PARENT_VERTEX) {
    INode *i = acc.get_inode( g.origin_g());
    if (i) {
      if ( i->parent_type( BLUE) == PARENT_VERTEX) {
	std::cerr << "ERROR: One-to-many mapping at green vertex "
		  << g.pane()->get_index(g.origin_l())+1 
		  << " of pane " << g.pane()->id() 
		  << " at " << g.origin_l().point() 
		  << ".\n It is projected onto from blue vertices \n\t"
		  << b.pane()->get_index(b.origin_l())+1 << " of pane " 
		  << b.pane()->id() << " at " << b.origin_l().point() << " and\n\t";
	b = i->halfedge(BLUE);
	std::cerr << b.pane()->get_index(b.origin_l())+1 
		  << " of pane " << b.pane()->id() 
		  << " at " << b.origin_l().point() << std::endl;
	RFC_assertion(i->parent_type( BLUE) != PARENT_VERTEX); abort();
      }
      if ( !contains( i->halfedge( BLUE), i->parent_type( BLUE), 
		      b, PARENT_VERTEX)) {
	b = b.next_g();
	RFC_assertion( contains( i->halfedge( BLUE), i->parent_type( BLUE), 
				 b, PARENT_VERTEX));
      }
      x.set_parent( b, Point_2(0,0), BLUE);
    }
  }

  if ( x.parent_type( BLUE) == PARENT_VERTEX) {
    // Step 2a, assign blue parent
    INode *i = acc.get_inode( b.origin_g());
    if ( i != NULL) {
      if ( !contains( i->halfedge( GREEN), i->parent_type( GREEN),
		      g, x.parent_type( GREEN))) {
	// Otherwise, we must correct the green parent of i by computing 
	// the intersection of the two computed parents
	if ( x.parent_type( GREEN) == PARENT_FACE) {
	  Parent_type pg = i->parent_type(GREEN);

	  bool found=false;
	  if ( pg ==PARENT_FACE) {
	    // Find the intersection of the two faces
	    HEdge h = g; RFC_assertion( !h.is_border_g());
	    do {
	      if ( contains( i->halfedge( GREEN), pg, h, PARENT_EDGE)) { 
		// the edge is the intersection of the two
		Point_2 nc = get_nat_coor( x, Generic_element(count_edges(g)),
					   h, GREEN);
		RFC_assertion( nc[0]>0 && nc[0]<1 && nc[1]>0 && nc[1]<0.5);
		x.set_parent( g, Point_2(nc[0],0), GREEN);

		found = true; 
		break; 
	      }
	    } while ( (h=h.next_g()) != g);
	  }

	  if ( !found) {
	    // Search for the common vertex 
	    HEdge h = g;
	    do {
	      if ( contains( i->halfedge( GREEN), pg, h, PARENT_VERTEX)) {
		g = h; x.set_parent( g, Point_2(0,0), GREEN); 
		
		found = true; 
		break; 
	      }
	    } while ( (h=h.next_g()) != g);
	    RFC_assertion( found);
	  }
	}
	else { // green parent of x is not a face.
	  if ( x.parent_type(GREEN)==PARENT_EDGE &&
	       x.parent_type( GREEN) == i->parent_type( GREEN) &&
	       !contains( i->halfedge( GREEN), i->parent_type( GREEN),
			  g, PARENT_VERTEX)) {
	    g = g.next_g();
	    RFC_assertion(contains(i->halfedge( GREEN), i->parent_type( GREEN),
				   g, PARENT_VERTEX));
	  }
	  RFC_assertion( acc.get_inode(g.origin_g())==NULL ||
			 acc.get_inode(g.origin_g())->parent_type(BLUE)
			 == PARENT_EDGE);

	  x.set_parent( g, Point_2(0,0), GREEN);

	  if ( !contains( i->halfedge( GREEN), i->parent_type( GREEN),
			  g, x.parent_type( GREEN))) {
	    std::cerr << "ERROR: One-to-many mapping at blue vertex "
		      << b.pane()->get_index(b.origin_l())+1 
		      << " of pane " << b.pane()->id() 
		      << " at " << b.origin_l().point() 
		      << ". It projects onto \n\tgreen vertex "
		      << b.pane()->get_index(g.origin_l())+1 << " of pane " 
		      << g.pane()->id() << " at " << g.origin_l().point() 
		      << " and\n\tgreen ";
	    switch (i->parent_type(GREEN)) {
	    case PARENT_VERTEX: std::cerr << "vertex"; break;
	    case PARENT_EDGE: std::cerr << "edge"; break;
	    case PARENT_FACE: std::cerr << "face incident on"; break;
	    default: ;
	    }
	    g = i->halfedge(GREEN);
	    std::cerr << " (" 
		      << g.pane()->get_index(g.origin_l())+1 << ","
		      << g.pane()->get_index(g.destination_l())+1
		      << ") of pane " << g.pane()->id() << " at (" 
		      << g.origin_l().point() << "," 
		      << g.destination_l().point() << ")." << std::endl;

	    RFC_assertion( contains( i->halfedge( GREEN), i->parent_type( GREEN),
				     g, x.parent_type( GREEN))); abort();
	  }
	}
      }
      delete i; i=NULL;
    }
    acc.set_inode( b.origin_g(), &x);

    // Step 3a
    // Delete the inconsistent inodes on adjacent edges of b
    HEdge h = b;
    do {
      INode_list &il = acc.get_inode_list( h);
      if ( !il.empty()) {
	// Throw away the false intersection points.
	while ( ! il.empty()) {
	  INode *i=&il.front();
	  
	  if ( contains( i->halfedge( GREEN), i->parent_type( GREEN), 
			 g, x.parent_type( GREEN))) {
	    il.pop_front();
	    delete i; i=NULL;
	  }
	  else
	    break;
	}
      }
      else {
	INode_list &ilr = acc.get_inode_list( h.opposite_g());
	// Throw away the false intersection points.
	while ( ! ilr.empty()) {
	  INode *i=&ilr.back();
	  
	  if ( contains( i->halfedge( GREEN), i->parent_type( GREEN), 
			 g, x.parent_type( GREEN))) {
	    ilr.pop_back();
	    delete i; i=NULL;
	  }
	  else
	    break;
	}
      }
    } while ( ( h = acc.get_next_around_origin( h)) != b);
  }
  else {
    // If x is on a blue edge, remove the inconsistent ones
    RFC_assertion( x.parent_type( BLUE) == PARENT_EDGE && b1==b);

    // Step 3b
    INode_list &il = acc.get_inode_list( b);
    // Throw away the false intersection points.
    while ( !il.empty()) {
      INode *i=&il.back();
      if ( i->nat_coor(BLUE)[0] >= x.nat_coor(BLUE)[0]) {
	std::cerr << "WARNING: Intersections are out-of-order on blue edge ("
		  << b.pane()->get_index(b.origin_l())+1 << ","
		  << b.pane()->get_index(b.destination_l())+1
		  << ") on pane " << b.pane()->id() << " at ("
		  << b.origin_l().point() << "," << b.destination_l().point()
		  << ").\n\tPrevious intersection was " 
		  << i->nat_coor(BLUE)[0]
		  << " but current one is at " << x.nat_coor(BLUE)[0] 
		  << " and green parameterization is "
		  << x.nat_coor(GREEN)[0]  << "." << std::endl;

	RFC_assertion( std::abs(i->nat_coor(0)[0] - x.nat_coor(0)[0]) < 0.1);
	RFC_assertion( i->parent_type( GREEN) != PARENT_VERTEX);

	if ( !contains( i->halfedge( GREEN), i->parent_type( GREEN),
			g, PARENT_VERTEX) &&
	     !contains( i->halfedge( GREEN), i->parent_type( GREEN),
			g.next_l(), PARENT_VERTEX)) {
	  std::cerr << "Cannot be continued. Stopping..." << std::endl;
	  abort();
	}
	
	// Otherwise, we must correct the green parent of i by computing 
	// the intersection of the two computed parents
	if ( !contains( i->halfedge( GREEN), i->parent_type( GREEN),
			g, PARENT_VERTEX))
	  g = g.next_g();
	
	x.set_parent( g, Point_2(0,0), GREEN);

	{ // Determine the blue parameterization of x based on closest point 
	  // of the green vertex on the blue edge.
	  Vector_3 t = b.destination_l().point()-b.origin_l().point();
	  Real c = (g.origin_l().point()-b.origin_l().point())*t / (t*t);
	  
	  // If snapped onto a blue vertex, then call recursively.
	  if ( c>=1-eps_e) {
	    x.set_parent( b.next_l(), Point_2(0,0), BLUE);
	    insert_node_in_blue_edge( x, x.halfedge(BLUE)); return;
	  }
	  else if ( c<=eps_e) {
	    x.set_parent( b, Point_2(0,0), BLUE); 
	    insert_node_in_blue_edge( x, x.halfedge(BLUE)); return;
	  }

	  // Otherwise, pop the neighbor vertex.
	  x.set_parent( b, Point_2(c,0), BLUE);
	}
      }
      
      if ( contains( i->halfedge( GREEN), i->parent_type( GREEN), 
		     g, x.parent_type( GREEN))) {
	il.pop_back();
	delete i; i=NULL;
      }
      else
	break;
    }
    il.push_back( x);
  }

  // If x is a vertex and b is border, change to a nonborder edge.
  if ( x.parent_type(BLUE) == PARENT_VERTEX && b.is_border_g()) {
    b = acc.get_next_around_origin(b);
    x.set_parent( b, Point_2(0,0), BLUE);
  }

  // If the green parent of x is a vertex, associate x with the green vertex.
  if ( x.parent_type(GREEN) == PARENT_VERTEX) {
    RFC_assertion(g==x.halfedge(GREEN));    
    acc.set_inode( g.origin_g(), &x);
  }

  // Output some debugging information.
  static int count=0, countmax=10;
  if ( verbose && count<countmax) {
    if ( x.parent_type(BLUE) == PARENT_VERTEX) {
      b = x.halfedge(BLUE);
      double dist = ( b.origin_l().point() -
		      op.get_point( x.halfedge(GREEN), x.nat_coor(GREEN))).squared_norm();

      if ( dist > 0.1*(b.destination_l().point()-
		       b.origin_l().point()).squared_norm()) {
	if ( count == 0) std::cout << std::endl;
	std::cout << "WARNING: Distance from blue vertex " 
		  << b.pane()->get_index(b.origin_l())+1 
		  << " of pane " << b.pane()->id() 
		  << " at " << b.origin_l().point() 
		  << " to green mesh is relatively large: " << std::sqrt(dist) << std::endl;

	if ( count++==0) {
	  std::cout << "Note: Only " << countmax << " such warnings " 
		    << " will be printed." << std::endl;
	}
      }
    }
    else if ( x.parent_type(GREEN) == PARENT_VERTEX) {
      g = x.halfedge( GREEN);
      double dist = ( g.origin_l().point() -
		      op.get_point( x.halfedge(BLUE), x.nat_coor(BLUE))).squared_norm();
      
      if ( dist > 0.1*( g.destination_l().point()-
			g.origin_l().point()).squared_norm()) {

	if ( count == 0) std::cout << std::endl;
	std::cout << "WARNING: Distance from green vertex " 
		  << g.pane()->get_index(g.origin_l())+1 
		  << " of pane " << g.pane()->id() 
		  << " at " << g.origin_l().point() 
		  << " to blue mesh is relatively large: " << std::sqrt(dist) << std::endl;

	if ( count++==0) {
	  std::cout << "NOTE: Only " << countmax << " such warnings" 
		    << " will be printed." << std::endl;
	}
      }
    }
  }
}

// Move an edge from ridge- or corner-queue into main queue.
// We use q_rdg to store edges that point from ridge vertices to 
// non-feature vertices and use q_crn for edges from corner
// vertices to non-corner vertices. Move edge from q_rdg into
// q only if q is empty; and move edge from q_crn to q if both q
// and q_rdg are empty.
void Overlay::
insert_edge_into_queue( const HEdge &h, INode *v, 
			std::queue<HEdge>   &q, 
			std::queue<HEdge>   &q_rdg, 
			std::queue<HEdge>   &q_crn) {
  // If the edge or its opposite has been marked, then skip.
  if ( acc.marked( h)) return;

  // First, determine feature rank of origin and destination vertices.
  int frank_org = acc.is_on_feature( h.origin_l());
  if ( frank_org) frank_org += acc.is_feature_0( ( h.origin_l()));

  if ( frank_org != 2) { // Consider feature type of green parent.
    Parent_type pt = v->parent_type(GREEN);
    HEdge g = v->halfedge(GREEN);
    if ( pt == PARENT_VERTEX) {
      Node gv;
      if ( acc.is_feature_0( gv=g.origin_l()))
	frank_org = 2; // If mapped onto a corner, consider vertex as corner
      else if ( frank_org == 0 && acc.is_on_feature( gv))
	frank_org = 1;
    }
    else if ( frank_org==0 && pt==PARENT_EDGE && acc.is_feature_1( g)) {
      frank_org = 1; // Ridge vertex
    }
  }

  int frank_dst = 0;
  if ( frank_org) { // Determine type of destination vertex
    frank_dst = acc.is_on_feature( ( h.destination_l()));
    if ( frank_dst) frank_dst += acc.is_feature_0( h.destination_l());
  }

  HEdge hopp = h.opposite_g();
  acc.mark( h);
  if ( frank_org == 0 || frank_org < frank_dst || hopp.is_border_g() ||
       frank_dst != 2 && !hopp.is_border_g() && acc.marked( hopp)) { 
    // If push edge into main queue, then mark both the edge and its opposite
    q.push( h); acc.mark( hopp); 
  }
  else if ( frank_org == 2 && !hopp.is_border_g() && acc.marked( hopp)) {
    // If I am a corner and the other edge is candidate, move into main queue
    q.push( hopp);
  }
  else if ( frank_org == 1) {
    // Otherwise, if frank_org is ridge, then push into ridge queue
    q_rdg.push( h);
  }
  else {
    // Otherwise frank_org is corner, don't push it.
    q_crn.push(h);
  }
}

// Move an edge from ridge- or corner-queue into main queue.
// We use q_rdg to store edges that point from ridge vertices to 
// non-feature vertices and use q_crn for edges from corner
// vertices to non-corner vertices. Move edge from q_rdg into
// q only if q is empty; and move edge from q_crn to q if both q
// and q_rdg are empty.
bool Overlay::
is_queue_empty(  std::queue<HEdge>   &q, 
		 std::queue<HEdge>   &q_rdg, 
		 std::queue<HEdge>   &q_crn) {
  if ( !q.empty()) return false;

  for ( int i=0; i<2; ++i) {
    std::queue<HEdge>   &qbuf = (i==0) ? q_rdg : q_crn;
    while ( !qbuf.empty()) {
      HEdge h = qbuf.front(); qbuf.pop();
      HEdge hopp = h.opposite_g();
      if ( !acc.marked( hopp)) {
	q.push( h); acc.mark( h.opposite_g());
	return false;
      }
    }
  }

  return true;
}

// This is step 1 of the overlay algorithm. It projects the blue vertices
//      and computes the intersections of the blue edges with the green
//      edges. These intersections are stored in the lists associated with
//      with the blue edges. 
//      At input, x is an inode corresponding to a blue vertex.
void Overlay::
intersect_blue_with_green() {
  // Precondition: None of the blue halfedges is marked.
  std::queue<HEdge>   q, q_rdg, q_crn;

  for ( int ncomp=1; ;++ncomp) { // Top loop over connected components.
    INode *seed = overlay_init();
    if ( seed==NULL) return; // If all halfedges are marked, then stop.
    RFC_assertion( seed->parent_type( BLUE) == PARENT_VERTEX);
    HEdge b = seed->halfedge( BLUE), h=b;
  
    if ( verbose) {
      if ( ncomp>1) 
        std::cout << "There appear to be multiple connected components "
                  << "in the input mesh.\n\tWorking on component " 
                  << ncomp << std::endl;
      else
        std::cout << "\nThe input meshes have " 
                  << ( is_opposite ? "opposite orientations" : 
                       "the same orientation") << std::endl;
      
      std::cout << "Starting with seed vertex " 
                << b.pane()->get_index(b.origin_l())+1 
                << " in pane " << b.pane()->id() << " of blue mesh at "
                << b.origin_l().point() << std::endl;
    }
    
    // Embed the seed into the blue vertex, (and the green vertex if its
    //       green parent is a vertex.)
    insert_node_in_blue_edge( *seed, seed->halfedge(BLUE));
    do {
      insert_edge_into_queue( h, seed, q, q_rdg, q_crn);
    } while ( (h = acc.get_next_around_origin(h)) != b);

    while ( !is_queue_empty( q, q_rdg, q_crn)) {
      HEdge b = q.front(); q.pop();
      HEdge bopp=b.opposite_g();

      INode_list &il = acc.get_inode_list( b);
      INode *x = acc.get_inode( b.origin_g()); assert(x);
      Node dst = b.destination_g();
      INode *idst= acc.get_inode( dst);
    
      bool is_feature = b.pane()->is_feature_1( b);
      bool is_border = b.is_border_g() || b.opposite_g().is_border_g();
      bool is_dst_feature = is_feature || acc.is_on_feature( dst);
      bool is_dst_border = is_border || dst.is_border_g();

      // This is to snap open-ends of features.
      const Real tol_snap_fea = 0.2, tol_snap_border = 2./3.;
      Real  cb=0, cg;

      for (;;) {
        if (x->parent_type( BLUE)==PARENT_VERTEX &&
            x->halfedge( BLUE).origin_g()==dst ||
            idst && get_edge_parent(*x,*idst,GREEN)!=Host_face())
          break; // The last two inodes on the edge are adjacent

        HEdge  g1, g0, g2;
        Parent_type t1, t0, t2;

        if ( il.empty()) {
          INode *iorg = acc.get_inode( b.origin_g());
          g1 = iorg->halfedge( GREEN); t1 = iorg->parent_type( GREEN);
          g0 = HEdge(); t0 = PARENT_NONE;
        }
        else {
          INode_list::iterator it=--il.end();
          g1 = it->halfedge( GREEN); t1 = it->parent_type( GREEN);
          if ( it==il.begin()) {
            INode *iorg = acc.get_inode( b.origin_g());
            g0 = iorg->halfedge( GREEN); t0 = iorg->parent_type( GREEN);
          }
          else {
            --it; g0 = it->halfedge( GREEN); t0 = it->parent_type( GREEN);
          }
        }

        // Compute the intersection of b with the edges in the link
        //    of <g1,t1> but not in the star of <g0, t0>.
        Real start = cb;
        intersect_link( x, b, g0, t0, g1, t1, start, &cb, &cg, &g2, &t2);
      
        // If current edge is a ridge edge, then attempt to snap it onto ridge.
        if ( is_feature && cb < 1 && cb > 0) {
          op.snap_blue_ridge_edge( b, g2, &cb, &cg, &t2, 
                                   is_border ? tol_snap_border : tol_snap_fea);
        }
        else if ( !is_feature && cb < 1 && 
                  ( is_dst_feature && cb>1-tol_snap_fea && 
                    (t2 == PARENT_VERTEX && acc.is_on_feature(g2.origin_l()) ||
                     t2 == PARENT_EDGE && acc.is_feature_1(g2)) ||
                    is_dst_feature && cb>1-tol_snap_border &&
                    (t2 == PARENT_VERTEX && g2.origin_l().is_border_g() ||
                     t2 == PARENT_EDGE && g2.is_border_g()))) {
          // Enforce snapping vertex onto feature/border
          cb = 1 + tol_snap_fea; 
          t2 = PARENT_EDGE;
        }

        Point_2 nc(cg,0.);
        RFC_assertion( t2 != PARENT_NONE);
        if ( cb > 1) { // If intersects beyond the end point, do projection.
          RFC_assertion( t2 != PARENT_VERTEX);
          bool onto=op.project_onto_element( dst.point(),&g2,&t2,
                                             Vector_3(0,0,0), &nc, eps_e, 0.2);
          RFC_assertion( onto);
        }

        if ( is_dst_feature && t2 != PARENT_VERTEX && cb>=1) {
          op.snap_blue_ridge_vertex( dst, &g2, &t2, &nc, is_dst_border ?
                                     tol_snap_border : tol_snap_fea);
        }

        // Create an inode for the intersection point.
        x = new INode();
        if ( cb < 1)
          x->set_parent( b, Point_2( cb, 0), BLUE);
        else
          x->set_parent( bopp, Point_2(0,0), BLUE);

        if ( nc[0] == 1.) {
          nc[0] = 0;
          g2 = !g2.is_border_g() ? g2.next_l() : g2.opposite_g();
        }

        static int count=0, countmax = 10;
        if ( verbose && count < countmax) {
          if ( dst.is_border_g() && cb>=1 && acc.get_inode( dst)==NULL && 
               !( t2 == PARENT_EDGE && g2.opposite_g().is_border_g() ||
                  t2 == PARENT_VERTEX && g2.origin_l().is_border_g())) {
            if ( count == 0) std::cerr << std::endl;
            std::cerr << "WARNING: Blue border vertex " << b.destination_l().point()
                      << " is not matched with green boundary" << std::endl;
            ++count;
          }
          else if ( is_border && cb>0 && cb<1 && 
                    !( t2 == PARENT_EDGE && g2.opposite_g().is_border_g() ||
                       t2 == PARENT_VERTEX && g2.origin_l().is_border_g())) {

            if ( count == 0) std::cerr << std::endl;
            std::cerr << "WARNING: Blue border edge " << b.origin_l().point() 
                      << b.destination_l().point()
                      << " is not matched with green boundary " << std::endl;
            ++count;
          }

          if ( is_dst_feature && cb>=1 && acc.get_inode( dst)==NULL && 
               !( t2 == PARENT_EDGE && g2.pane()->is_feature_1(g2) ||
                  t2 == PARENT_VERTEX && acc.is_on_feature(g2.origin_l()))) {
            std::cerr << "WARNING: Blue ridge vertex " << b.destination_l().point()
                      << " is not matched with green ridge. cb is " << cb
                      << " and nc is " << nc << std::endl;
            ++count;
          }
          else if ( is_feature && cb<1 && 
                    !( t2 == PARENT_EDGE && g2.pane()->is_feature_1(g2) ||
                       t2 == PARENT_VERTEX && acc.is_on_feature(g2.origin_l()))) {
            std::cerr << "WARNING: Blue ridge edge " << b.origin_l().point() 
                      << b.destination_l().point()
                      << " is not matched with green ridge. cb is " << cb
                      << " and nc is " << nc << std::endl;
            ++count;
          }
        }

        x->set_parent( g2, nc, GREEN);
      
        insert_node_in_blue_edge( *x, b);
      }

      // Copy inode from border edge into neighbor edge
      if ( b.is_border_g() && !il.empty()) {
        INode_list &ilr = acc.get_inode_list( bopp);

        while ( !il.empty()) {
          INode &i =  il.front(); il.pop_front();
          ilr.push_front( i); 
          i.set_parent( bopp, Point_2(1-i.nat_coor(BLUE)[0],0), BLUE);
        }
      }

      // Push unvisited incident halfedges of b.destination_l() into q.
      HEdge h;  h = b.next_g();

      do {
        // Push the edge into queue and mark it as nonzero.
        insert_edge_into_queue( h, x, q, q_rdg, q_crn);
      } while ( ( h = acc.get_next_around_origin( h)) != bopp);
    }
  } // end for

  // Unmark all the halfedges of B
  B->unmark_alledges();
}

//  This is step 2 of the algorithm. It sorts the intersection points 
//      corresponding to each green edge and insert them into green edges
//      in linear time.
void Overlay::
sort_on_green_edges() {
  std::vector<RFC_Pane_overlay*>   ps;
  B->panes( ps);

  inodes.clear();
  // Loop through all the panes of B to insert the inodes into a list
  for ( std::vector<RFC_Pane_overlay*>::iterator 
	  pit=ps.begin(); pit!=ps.end(); ++pit){
    // Loop through the vertices.
    for (int i=1, s=(*pit)->size_of_nodes(); i<=s; ++i) {
      Node v(*pit, i);
      if ( !v.is_isolated() && v.halfedge_l().destination_l() == v 
           && v.is_primary()) {
	INode *i = acc.get_inode( v); RFC_assertion( i);
	i->prev_link[GREEN]=i->next_link[GREEN]=NULL;
	inodes.push_back( i);
      }
    }

    for ( int i=1, s=(*pit)->size_of_faces(); i<=s; ++i) {
      HEdge h( *pit, Edge_ID( i, 0)), h0=h;
      do {
        INode_list &il = acc.get_inode_list( h);
        for ( INode_list::iterator it=il.begin(); it!=il.end(); ++it) {
          INode *i = &*it; RFC_assertion( i);
          i->prev_link[GREEN]=i->next_link[GREEN]=NULL;
          inodes.push_back( i);
        }
        
        HEdge lopp = h.opposite_l();
        if ( lopp.is_border_l()) {
          INode_list &il = acc.get_inode_list( lopp);
          for ( INode_list::iterator it=il.begin(); it!=il.end(); ++it) {
            INode *i = &*it; RFC_assertion( i);
            i->prev_link[GREEN]=i->next_link[GREEN]=NULL;
            inodes.push_back( i);
          }
        }
      } while ( (h=h.next_l()) != h0);
    }
  }

  int count = 1;
  // Loop through all the panes of B
  for ( std::vector<RFC_Pane_overlay*>::iterator 
	  pit=ps.begin(); pit!=ps.end(); ++pit) {
    // Loop through the real faces of each pane.
    for (int j=1, s=(*pit)->size_of_faces(); j<=s; ++j, ++count) {
      HEdge b(*pit,Edge_ID(j,0)), h=b;
      do {
	INode *i=acc.get_inode( h.origin_g());
	if ( i)	insert_node_in_green_edge( i, count);
	
	INode_list &il = acc.get_inode_list( h);
	if ( !il.empty()) {
	  RFC_assertion(acc.get_inode_list(h.opposite_g()).empty());
	  // Loop through the intersections on the edges
	  INode_list::iterator v = il.begin(), vend = il.end();
	  for ( ; v != vend; ++v) 
	    insert_node_in_green_edge( &*v, count);
	}
	else {
	  INode_list &ilr = acc.get_inode_list( h.opposite_g());
	  if ( !ilr.empty()) {
	    INode_list::reverse_iterator vr=ilr.rbegin(),vrend=ilr.rend();
	    for ( ; vr != vrend; ++vr) 
	      insert_node_in_green_edge(&*vr, count);
	  }
	}
	// Increment the iterator
      } while ( (h = h.next_g()) != b);
    }
  }
  
  // Loop through all the inodes
  for (std::list<INode*>::iterator it=inodes.begin(); it!=inodes.end(); ++it) {
    INode *i=*it;
    if ( i->parent_type( GREEN) == PARENT_EDGE) {
      INode_list &il = acc.get_inode_list( i->halfedge(GREEN));
      if ( il.empty()) {
	while ( i->prev_link[GREEN]) { i = i->prev_link[GREEN]; }

	for (;;) {
	  INode *j = i->next_link[GREEN];
	  il.push_back(*i);
	  if ( j) i=j; else break;
	}
      }
    }
  }
}

// The helper for sort_on_green_edges. It inserts a node v into the green
// edge v->green_halfedge(). Tag is for marking the buffer. We assume 
// that each green(blue) edge intersects a blue(green) face at most twice. 
void Overlay::
insert_node_in_green_edge( INode* v, int tag) {
  switch ( v->parent_type( GREEN)) {
  case PARENT_VERTEX: {
    RFC_assertion(acc.get_inode(v->halfedge( GREEN).origin_g())==v);
    return;
  }
  case PARENT_EDGE: {
    HEdge g = v->halfedge( GREEN), gopp=g.opposite_g();
    if ( !gopp.is_border_g() && gopp<g) {
      g = gopp;
      v->set_parent( gopp, Point_2( 1.-v->nat_coor(GREEN)[0],0.), GREEN);
    }
    INode *y = g.pane()->get_buffered_inode( g, tag);
    
    if ( y ) {
      INode *x = v;

      if ( y->nat_coor(GREEN)[0] > x->nat_coor(GREEN)[0]) {
	RFC_assertion(x->next_link[GREEN] == NULL || x->next_link[GREEN] == y);
 	RFC_assertion(y->prev_link[GREEN] == NULL || y->prev_link[GREEN] == x);
 	x->next_link[GREEN] = y; y->prev_link[GREEN] = x;
      }
      else {
 	RFC_assertion(x->prev_link[GREEN] == NULL || x->prev_link[GREEN] == y);
 	RFC_assertion(y->next_link[GREEN] == NULL || y->next_link[GREEN] == x);
 	x->prev_link[GREEN] = y; y->next_link[GREEN] = x;
      }
    }
    // Otherwise, store the node
    else
      g.pane()->set_buffered_inode( g, tag, v);
  }
  case PARENT_FACE:
    return;
  default:
    RFC_assertion(false);
  }
}

// This is step 3 of the algorithm. It computes the associates of the
//    green vertices in B.
void Overlay::
associate_green_vertices() {
  std::vector<RFC_Pane_overlay*>   ps;
  B->panes( ps); // Process all the panes including extension panes.

  // Loop through all panes of B
  for (std::vector<RFC_Pane_overlay*>::iterator 
	 pit=ps.begin(); pit!=ps.end(); ++pit) {
    // Loop through the core faces of each pane.
    for (int j=1, s= (*pit)->size_of_faces(); j<=s; ++j) {
      HEdge b(*pit, Edge_ID(j,0)), h=b; 
      COM_assertion( !b.is_border_g());

      do {
	INode *i=acc.get_inode( h.origin_g());
	if ( i && i->parent_type( GREEN)!=PARENT_FACE) 
	  project_adjacent_green_vertices( i, b);
	
	INode_list &il = acc.get_inode_list( h);

	if ( ! il.empty()) {
	  // Loop through the intersections on the edges
	  INode_list::iterator v = il.begin(), vend = il.end();
	  for ( ; v != vend; ++v) project_adjacent_green_vertices( &*v, b);
	}
	else {
	  INode_list &ilr = acc.get_inode_list( h.opposite_g());
	  if ( !ilr.empty()) {
	    INode_list::reverse_iterator vr=ilr.rbegin(),vrend=ilr.rend();
	    for ( ; vr!=vrend; ++vr) project_adjacent_green_vertices(&*vr,b);
	  }
	}
	// Increment the iterator
      } while ( (h = h.next_g()) != b);
    }
  }
}

/// Helper for associate_green_vertices().
void Overlay::
project_adjacent_green_vertices( const INode *i, const HEdge &b) {
  std::queue< INode*> q;

  // From every known subnode, projects its adjacent green vertices
  // onto blue faces and creates new subnodes for the green vertices.
  do {
    HEdge g = i->halfedge(GREEN), gopp, g0=g;
    Parent_type igp = i->parent_type( GREEN);
    // Loop through all the incident halfedges.
    do {
      gopp = g.opposite_g();
      Node dst = g.destination_g();
      
      INode_list &il = acc.get_inode_list( g);
      INode_list &ilr = acc.get_inode_list( gopp);

      // If there is any subnode between i and dst, skip the edge.
      if ( igp == PARENT_VERTEX) {
	if (!il.empty() || !ilr.empty())
	  continue;
      }
      else if ( igp == PARENT_EDGE) {
	INode *j;
	if ( !il.empty()) {
	  if ( &il.back() != i) continue;
	  j = ( il.size() > 1 ? i->prev_link[GREEN] : acc.get_inode( g.origin_g()));
	}
	else {
	  if ( &ilr.front() != i) continue;
	  j = ( ilr.size() > 1 ? i->next_link[GREEN] : acc.get_inode( g.origin_g()));
	}
	if ( j && contains( b, PARENT_FACE, 
			    j->halfedge( BLUE), j->parent_type( BLUE)))
	  continue;
      }

      // If dst has already been projected, skip the edge.
      INode *x=NULL;
      if ( (x=acc.get_inode( dst)) != NULL) {
	RFC_assertion( x->parent_type( BLUE) != PARENT_FACE ||
		       contains( x->halfedge(BLUE), PARENT_FACE,
				 i->halfedge(BLUE), i->parent_type(BLUE)));
	continue;
      }

      // Determine whether the vertex projects onto the blue face, and if so,
      // create a new subnode at it and insert the subnode into the queue.
      HEdge b1 = b; Parent_type t=PARENT_FACE; 
      Point_2 nc;
      const RFC_Pane_overlay *gpane = g.pane();
      Vector_3 v1 = gpane->get_normal( g, g.destination_l());
      if (op.project_onto_element( g.destination_l().point(), 
				   &b1, &t, v1, &nc, eps_p)) {
	Vector_3 v2 = op.get_face_normal( b1, nc); 
	if (v2 != Vector_3(0,0,0)) v2 = v2 / std::sqrt( v2*v2);

	if ( logical_xor( is_opposite, v1 * v2 < 0.15)) continue;
	RFC_assertion( nc[0] != 0. && nc[1] != 0.);

	x = new INode();
	  
	x->set_parent( b1, nc, BLUE);
	x->set_parent( gopp, Point_2(0,0), GREEN);
	// Check whether consistency rules are satisfied
	if ( verify_inode( x)) {
	  inodes.push_back( x);
	  acc.set_inode( dst, x);
	  q.push( x);
	}
	else
	  delete x;
      }
      RFC_assertion( igp != PARENT_FACE); // Must have been projected.
    } while ( ( g = ( igp==PARENT_VERTEX ? gopp.next_g() : gopp)) != g0);
    if ( !q.empty()) { i = q.front(); q.pop(); } else i = NULL;
  } while (i);
       
}

bool Overlay::
verify_inode( const INode *i) {
  RFC_precondition( i->parent_type(BLUE) == PARENT_FACE);

  HEdge g = i->halfedge(GREEN), gopp, g0=g;
  // Loop through all the incident halfedges.
  do {
    gopp = g.opposite_g();
    
    INode_list &il = acc.get_inode_list( g);
    INode_list &ilr = acc.get_inode_list( gopp);
    
    // If there is any subnode between i and dst, skip the edge.
    INode *j;
    if ( !il.empty()) {
      if ( &il.back() != i) continue;
      j = ( il.size() > 1 ? i->prev_link[GREEN] : acc.get_inode( g.origin_g()));
    }
    else if ( !ilr.empty()) {
      if ( &ilr.front() != i) continue;
      j = ( ilr.size() > 1 ? i->next_link[GREEN] : acc.get_inode( g.origin_g()));
    }
    else 
      j = acc.get_inode( g.destination_g());

    if ( j && !contains( i->halfedge( BLUE), PARENT_FACE, 
			 j->halfedge( BLUE), j->parent_type( BLUE)))
      return false;
  } while ( ( g = gopp.next_g()) != g0);

  return true;
}

// The return value indicates whether this call is conclusive.
bool Overlay::
intersect_link_helper( const HEdge &b,
		       const HEdge &g2,
		       Real start,
		       Real *cb,
		       Real *cg,
		       HEdge *g,
		       Parent_type *t,
		       bool *found, 
		       int snapcode, 
		       const Node &anchor, bool panic) {
  RFC_assertion( !g2.is_border_l());

  Real tcb, tcg;     // Parameterization of intersection on b and g2
  Real orient_match; // To what degree [0..1] b and g2 are counterclockwise
  Real normal_match; // To what degree [0..1] the intersection have same normal

  // Tolerance for normal matching
  const Real tol_nm_large = 0.6, tol_nm_small=0.3; // about 50 and 80 degrees
  const Real tol_nm = panic ? tol_nm_small : tol_nm_large;
  const Real tol_proj=0.5, tol_snap=0.2;

  bool do_intersect = op.intersect( b, g2, start, &tcb, &tcg, &orient_match,
				    &normal_match, is_opposite, eps_e);
  do_intersect &= (normal_match >= tol_nm);

  if ( verbose && panic) {
    std::cerr << "INFO: Intersection with green edge (" 
	      << g2.origin_l().point() << "," << g2.destination_l().point()
	      << ") is at bparam=" << tcb << ", gparam=" << tcg << std::endl;
    
    if ( tcb<HUGE_VAL) {
      std::cerr << "\t inner product of normals equal to " 
		<< normal_match << std::endl;

      if ( normal_match<tol_nm_large) {
	std::cerr << "WARNING: Intersection was rejected due to normal mismatch under tolerance " 
		  << tol_nm_large << std::endl;
	
	if ( normal_match > tol_nm_small) {
	  std::cerr << "WARNING: Trying with reduced normal-matching tolerance of " 
		    << tol_nm_small << std::endl;
	}
      }
    }
  }

  if ( do_intersect && tcb == 0) {
    // Inconsistency detected. It has backfired!
    // Or we need to snap to feature edge if nothing else works.
    *cb = 0.; *g = g2; *cg = tcg; *found=false;
    *t = ( tcg==0 || tcg==1) ? PARENT_VERTEX : PARENT_EDGE;

    return true;
  }

  if ( (tcb*eps_e > 1.) || orient_match < -eps_p || 
       normal_match < tol_nm || tcb<0.) {
    if ( *cb == HUGE_VAL) *g = g2; // Keep a record of the last intersection.

    // Do not treat it as fatal inconsistency because this might
    // happen at locations with large curvature and trying to repair it
    // can fail the algorithm. Instead, we simply ignore this intersection.
    return false;
  }

  if ( verbose && snapcode == 7 && tcg > tol_snap && tcg < 1-tol_snap ) {
    std::cerr << "WARNING: Blue edge (" << b.origin_l().point() 
	      << ", " << b.destination_l().point()
	      << " appears to walk over a green corner "
	      << g2.prev_l().origin_l().point()
	      << " in an ambiguous way. This may cause overlay to failure. " 
	      << std::endl;
  }

  if ( do_intersect) {
    bool case1;
    if ( ( case1 = tcg == 1. || snapcode==6 || snapcode==7 && 
	   op.normalmatch(b, g2.next_l()) > op.normalmatch(b, g2.prev_l())) || 
	 tcg == 0. || snapcode==5 || snapcode==7 && 
	 op.normalmatch( b, g2.next_l()) < op.normalmatch( b, g2.prev_l()) || 
	 ( case1 = *found && tcg>0.5 && 
	   contains(*g, *t, g2.next_g(), PARENT_VERTEX)) ||
	 *found && tcg<0.5 && contains( *g, *t, g2, PARENT_VERTEX)) { 
      *g = case1 ? g2.next_g() : g2; *t = PARENT_VERTEX; *cg = 0.; 
      if ( *found && *cb >1 && *cb < 1+tol_proj || tcb == 1) *cb = 1;
      else if ( !*found || tcb < *cb) *cb = tcb;

      *found = true;
    }
    else if ( !*found || !snapcode && tcb < *cb ||
	      snapcode && std::abs(0.5-tcg)>std::abs(0.5-*cg)) 
    { *g = g2; *t = PARENT_EDGE; *cg = tcg; *cb = tcb; *found = true; }
  }
  else if ( tcg >=0. && tcg <= 1.) {
    RFC_assertion( tcb > 1.); // Otherwise, they would have intersected.   

    bool case1;
    if ( ( case1 = tcg == 1. || snapcode==6 || snapcode==7 && 
	   op.normalmatch(b, g2.next_l()) > op.normalmatch(b, g2.prev_l())) || 
	 tcg == 0. || snapcode==5 || snapcode==7 && 
	 op.normalmatch( b, g2.next_l()) < op.normalmatch( b, g2.prev_l()) || 
	 *found && *t != PARENT_VERTEX && g2.face()!=(*g).face() &&
	 ( ( case1 = tcg>0.5 && contains( *g, *t, g2.next_g(), PARENT_VERTEX)) ||
	   tcg<0.5 && contains( *g, *t, g2, PARENT_VERTEX))) {

      if ( *found && *cb <= 1) 
      {	*g = case1 ? g2.next_l() : g2; *t = PARENT_VERTEX; *cg = 0.; *cb = 1; }
      else if ( case1) { 
	*g = g2.next_l(); *cg = 0;
	*t = (g->destination_g() == anchor) ? PARENT_EDGE : PARENT_FACE; 

	if ( !*found || tcb < *cb) *cb = tcb; 
      }
      else {
	*g = g2.prev_l(); *cg = 1.; 
	*t = (g->origin_g() == anchor) ? PARENT_EDGE : PARENT_FACE; 

	if ( !*found || tcb < *cb) *cb = tcb; 
      }

      *found = true;
    }
    else if ( !*found || !snapcode && tcb < *cb ||
	      snapcode && std::abs(0.5-tcg)>std::abs(0.5-*cg))
    { *cg = tcg; *g = g2; *t = PARENT_FACE; *cb = tcb; *found = true; }
  }
  else if ( !*found && tcb>=0) {
    bool case1;
    if ( *cb > 0 && *cb < HUGE_VAL && 
	 *t != PARENT_VERTEX && g2.face() != (*g).face() &&
	 ( ( case1 = tcg>0.5 && contains( *g, *t, g2.next_g(), PARENT_VERTEX)) ||
	   tcg<0.5 && contains( *g, *t, g2, PARENT_VERTEX))) {

      if ( tcb > 1 && *cb >1) {
        assert(!g2.is_border_l());
	*g = case1 ? g2.next_l() : g2.prev_l(); *t = PARENT_EDGE; *cg = case1 ? 0. : 1.; 
	if ( tcb < *cb) *cb = tcb; 
      }
      else {
	*g = case1 ? g2.next_g() : g2; *t = PARENT_VERTEX; *cg = 0.; 
	if ( tcb ==1 || tcb >1 && tcb < 1+tol_proj && *cb < 1 || 
	     tcb<=1 && *cb > 1 && *cb < 1+tol_proj)
  	  *cb = 1;
	else if ( tcb < *cb) *cb = tcb;
      }
    }
    else if ( *cb == HUGE_VAL || 
	      *cg < tcg && tcg < 0 || *cg > tcg && tcg > 1) {
      // NOTE: cg is not used outside intersect_link if *cb>1, but we
      //       need to keep the value of tcg for later comparisons 
      assert(!g2.is_border_l());
      if (  tcg <0 && tcb>1 && g2.prev_l().origin_g() == anchor)
      { *g = g2.prev_l(); *t = PARENT_EDGE; }
      else if ( tcg>1 && tcb>1 && g2.next_l().destination_g() == anchor) 
      { *g = g2.next_l(); *t = PARENT_EDGE; }
      else
      { *g = g2; *t = PARENT_FACE; }
      
      *cb = tcb; *cg = tcg;
    }
  }

  return false;
}

// The return value indicates whether this call is conclusive.
bool Overlay::
intersect_link_helper2( Real *cb,
			Real *cg,
			HEdge *g,
			Parent_type *t) {
  if ( *cb > 1.) {
    if ( *cg>1) { *cg = 1; } else if ( *cg <0) { *cg = 0.; }
    return true; // Preserve the parent-type.
  }
  else {
    // Otherwise, there is an inconsistency. 
    if ( *cb <=0.) *cb = 0.;

    if ( *cg > 0. && *cg < 1.){ *t = PARENT_EDGE; }
    else if ( *cg >= 1.)
    { *g = g->next_g(); *t = PARENT_VERTEX; *cg = 0.; }
    else if ( *cg <= 0.) 
    { *t = PARENT_VERTEX; *cg = 0.; }
    return false;
  }
}

// This function computes the intersection of a halfedge b 
//   with the edges in Lk( g1,t1)-Lk( (g0,t0),(g1,t1)).
//   It takes O(n) time, where n is the number of incident
//   edges of the green_parent of (g1,t1). It returns the intersection
//   point on both b and g, and the parent of the intersection.
//   This subroutine is used in intersect_blue_with_green.
bool Overlay::
intersect_link( const INode *x,
		const HEdge &b,
		const HEdge &g0,
		const Parent_type t0,
		HEdge &g1,
		const Parent_type t1,
		Real start,
		Real *cb,
		Real *cg,
		HEdge *g,
		Parent_type *t, int trytime) {

  bool found = false, is_feature = acc.is_feature_1(b);
  bool is_border= b.is_border_g() || b.opposite_g().is_border_g();
  *cb = *cg = HUGE_VAL;
  *g = HEdge();

  switch ( t1) {
  case PARENT_FACE: {
    RFC_assertion( !g1.is_border_l());
    // Loop through the edges of the face
    HEdge g2 = g1;
    do {
      int snapcode = is_feature ? 4 : 0;

      if ( intersect_link_helper( b, g2, start, cb, cg, g, t, &found, 
				  snapcode, Node(), trytime)) {
	if ( found) return true;
	else return intersect_link_helper2( cb, cg, g, t);
      }
    } while ( ( g2 = g2.next_l()) != g1);

    break;
  }
  case PARENT_EDGE: {
    HEdge g1_0=g1;  

    // if g1 intersects b from left to right
    do {
      if ( g1.is_border_l() || contains( g1, PARENT_FACE, g0, t0)) continue;
      HEdge g2 = g1.next_l();
      
      do {
	int snapcode = 0;
	if (is_feature) {
	  snapcode = 4;
	  if ( is_border && g1.opposite_g().is_border_g() || 
	       !is_border && acc.is_feature_1( g1)) {
	    if ( g2.prev_l()==g1) snapcode += 1;
	    else if (g2.next_l()==g1) snapcode += 2;
	  }
	}

	if ( intersect_link_helper( b, g2, start, cb, cg, g, t, &found, 
				    snapcode, Node(), trytime)) {
	  if ( found) return true;
	  else return intersect_link_helper2( cb, cg, g, t);
	}
      } while ( ( g2 = g2.next_l()) != g1);
    } while ( ( g1 = g1.opposite_g()) != g1_0);

    break;
  }
  case PARENT_VERTEX: {
    HEdge g1_0=g1, g1_1; 
    Node  v = g1.origin_g();

    do {
      g1_1 = g1.prev_g();
      if ( g1.is_border_l() || contains( g1, PARENT_FACE, g0, t0)) continue;
      HEdge g2 = g1.next_l();
      do {
	int snapcode = 0;
	if (is_feature) {
	  snapcode = 4;

	  if ( is_border &&  g2.prev_l().opposite_g().is_border_g() ||
	       !is_border && acc.is_feature_1( g2.prev_l()) &&
	       contains(g2.prev_l(), PARENT_EDGE, g1, PARENT_VERTEX)) 
	    snapcode += 1;
	  if (  is_border &&  g2.next_l().opposite_g().is_border_g() ||
                !is_border && acc.is_feature_1( g2.next_l()) &&
		contains(g2.next_l(), PARENT_EDGE, g1, PARENT_VERTEX))
	    snapcode += 2;
	}

	if ( intersect_link_helper( b, g2, start, cb, cg, g, t, &found, 
				    snapcode, v, trytime))  {
	  if ( found) return true;
	  else return intersect_link_helper2( cb, cg, g, t);
	}
      } while ( ( g2 = g2.next_l()) != g1_1);
    } while ( ( g1 = g1_1.opposite_g()) != g1_0);

    break;
  }
  default:
    RFC_assertion( false);
    break;
  }

  if (found) return true;   
  if ( *cb < HUGE_VAL) return intersect_link_helper2( cb, cg, g, t);

  if ( trytime==0) {
    // Print out error messages
    std::cerr << "WARNING: Encountered inconsistency when intersecting blue edge ("
	      << b.pane()->get_index(b.origin_l())+1 << ","
	      << b.pane()->get_index(b.destination_l())+1 << ") in pane "
	      << b.pane()->id() << " at \n\t("
	      << b.origin_l().point() << "), (" << b.destination_l().point()
	      << ") \nwith the link of the ";
    if ( t1 == PARENT_FACE)   std::cerr << "green face incident on";
    if ( t1 == PARENT_EDGE)   std::cerr << "green";
    if ( t1 == PARENT_VERTEX) std::cerr << "source vertex of green";
    std::cerr << " halfedge (" << g1.pane()->get_index(g1.origin_l())+1 
	      << ","  << g1.pane()->get_index(g1.destination_l())+1 
	      << ") in pane " << g1.pane()->id() << " at \n\t("
	      << g1.origin_l().point() << "), (" 
	      << g1.destination_l().point() << ")." << std::endl;

    // One more try with print info
    intersect_link( x, b, g0, t0, g1, t1, start, cb, cg, g, t, 1);

    if ( *cb < HUGE_VAL) {
      std::cerr << "WARNING: Input meshes appear to have severe normal mismatch." << std::endl;
      std::cerr << "WARNING: Trying to continue with relaxed normal matching.\n\n"	<< std::endl;
    }
    else {
      // Continue with projecting onto face
      *cb = HUGE_VAL; *cg = HUGE_VAL; *t = PARENT_FACE;
      if ( (*g).pane() == NULL || t1==PARENT_VERTEX) *g = g1;
      std::cerr << "WARNING: Could not find edge intersection." << std::endl;
      std::cerr << "WARNING: Trying to recover with point projection.\n\n" << std::endl;
    }
  }

  return false;
}

// Determining the parent of a directed sub-edge between two inodes. 
Host_face Overlay::
get_edge_parent( const INode &i0,
		 const INode &i1,
		 const int   color) const {
  HEdge h0 = i0.halfedge( color);
  HEdge h1 = i1.halfedge( color);
  
  Parent_type t0 = i0.parent_type( color);
  Parent_type t1 = i1.parent_type( color);
  
  if ( t1 == PARENT_FACE) {
    if ( !h1.is_border_g() && contains( h1, PARENT_FACE, h0, t0))
      return Host_face( h1, PARENT_FACE);
    else 
      return Host_face();
  }
  
  switch ( t0) {
  case PARENT_VERTEX: {
    HEdge h = h0;
    // First check whether they lie on the same edge
    do {
      if ( t1 == PARENT_EDGE && (h == h1 || h.opposite_g() == h1) ||
	   t1 == PARENT_VERTEX && h.destination_g()==h1.origin_g()) {
	return Host_face( h, PARENT_EDGE);
      }
      RFC_assertion( t1 != PARENT_VERTEX || h0.origin_g() != h1.origin_g());
    } while ( (h=acc.get_next_around_origin(h)) != h0);
    // If not, check whether they lie on the same face
    do {
      if ( !h.is_border_g() && contains(h, PARENT_FACE, h1, t1)) 
	return Host_face( h, PARENT_FACE);
    } while ( (h=acc.get_next_around_origin(h)) != h0);
    break;
  }
  case PARENT_EDGE: {
    HEdge hopp=h0.opposite_g();
    if ( t1 == PARENT_EDGE && ( h0 == h1 || hopp == h1)) {
      Real nc0=i0.nat_coor( color)[0], nc1=i1.nat_coor( color)[0];
      if ( h0 == h1 && nc0<nc1 || hopp == h1 && 1.-nc0<nc1)
	return Host_face( h0, PARENT_EDGE);
      else 
	return Host_face( hopp, PARENT_EDGE);
    }
    if ( t1 == PARENT_VERTEX) {
      if (h0.destination_g() == h1.origin_g())
	return Host_face( h0, PARENT_EDGE);
      else if (h0.origin_g() == h1.origin_g())
	return Host_face( hopp, PARENT_EDGE);
    }
    if ( ! h0.is_border_g() && contains( h0, PARENT_FACE, h1, t1))
      return Host_face( h0, PARENT_FACE);
    if ( ! hopp.is_border_g() && contains( hopp, PARENT_FACE, h1, t1))
      return Host_face( hopp, PARENT_FACE);
    break;
  }
  case PARENT_FACE: {
    RFC_assertion( ! h0.is_border_g());
    if ( contains( h0, PARENT_FACE, h1, t1)) 
      return Host_face(h0, PARENT_FACE);
    break;
  }
  default:
    RFC_assertion( false);
    break;
  }
  return Host_face();
}
  
// Determines the parents for a halfedge and its opposite.
Overlay::Parent_pair Overlay::
get_edge_pair_parents( const INode &i0,
		       const INode &i1,
		       const int color) const {
  Host_face f = get_edge_parent( i0, i1, color);
  HEdge g = f.halfedge(); 
  RFC_assertion( !g.is_border_g());

  // There are two cases for f:
  // 1. g is not a border edge and the parent type is PARENT_FACE
  // 2. g is not a border edge and the parent type is PARENT_EDGE

  switch ( f.parent_type()) {
  case PARENT_FACE:
    return std::make_pair( g, g);
  case PARENT_EDGE:
    return std::make_pair( g, g.opposite_g());
  default:
    RFC_assertion( false);
    return std::make_pair( HEdge(), HEdge());
  }
}

// Checking whether an object <e1,t1> contains another object <e2,t2>. 
bool 
Overlay::contains( const HEdge &e1, const Parent_type t1,
		   const HEdge &e2, const Parent_type t2) const {
  if ( e2.pane() == NULL) return false;

  switch ( t1) {
  case PARENT_FACE: {
    Face f1=e1.face();
    switch( t2) {
    case PARENT_FACE: {
      return f1 == e2.face();
    }
    case PARENT_EDGE: {
      return f1 == e2.face() || f1 == e2.opposite_g().face();
    }
    case PARENT_VERTEX: {
      Node v2=e2.origin_g();
      HEdge h = e1;
      do {
	if ( h.destination_g() == v2) return true;
      } while ( ( h = h.next_g()) != e1);
      break;
    }
    default:
      RFC_assertion( false);
      break;
    }
    break;
  }
  case PARENT_EDGE:
    if ( t2 == PARENT_EDGE)
      return ( e1 == e2 || e1 == e2.opposite_g());
    else if ( t2 == PARENT_VERTEX) {
      Node v2=e2.origin_g();
      return e1.origin_g() == v2 || e1.destination_g() == v2;
    }
    break;
  case PARENT_VERTEX:
    return t2 == PARENT_VERTEX && e1.origin_g() == e2.origin_g();
  default:
    RFC_assertion( false);
    break;
  }
  return false;
}

//=========== Subroutines for outputting the intermediate results
// Write out all the inodes in Tecplot format.
void Overlay::
write_inodes_tec(std::ostream &os, const char *color) {
  int n=0;
  std::list<INode*>::iterator it = inodes.begin(), iend = inodes.end();
  for ( ; it != iend; ++it,++n) {
    if ( n%50==0) {
      os << "GEOMETRY T=LINE3D";
      if ( color) os << " C=" << color;
      os << " LT=0.2" << std::endl;
      os << std::min(int(inodes.size()-n),int(50)) << std::endl;
    }
    INode *i = *it;
    os << 2 << ' ' << op.get_point( i->halfedge(BLUE), i->nat_coor( BLUE))
       << ' ' << op.get_point( i->halfedge(GREEN), i->nat_coor( GREEN))
       << std::endl;
  }
}

void Overlay::
write_inodes_vec( std::ostream &os) {
  int n = inodes.size();

  os << "VECT\n";
  os << n << ' ' << n*2 << ' ' << 0 << '\n';

  int i;
  // Print the number of vertices in each inode
  for ( i=n; i>0; --i) os << 2 << '\n';
  for ( i=n; i>0; --i) os << 0 << '\n';

  // Print the coordinates of each node
  std::list<INode*>::iterator it = inodes.begin(), iend = inodes.end();
  for ( ; it != iend; ++it) {
    INode *i = *it;
    os << op.get_point( i->halfedge(BLUE), i->nat_coor( BLUE))
       << ' ' << op.get_point( i->halfedge(GREEN), i->nat_coor( GREEN))
       << std::endl;
  }
}

std::pair<const INode *, HEdge>
Overlay::get_next_inode_ccw(const INode *v0, const INode *v1, int color) const 
{
  INode *v2=NULL;
  switch( v1->parent_type( color)) {
  case PARENT_FACE:
    // Rule 1: When making turns, it must be at an edge/vertex.
    return std::pair<const INode *, HEdge>(NULL,HEdge());
  case PARENT_EDGE: {
    Host_face hf = get_edge_parent( *v0, *v1, color);
    if ( hf.parent_type() == PARENT_EDGE) {
      // Rule 2: No three vertices on the same edge can be in the same subface.
      return  std::pair<const INode *, HEdge>(NULL,HEdge());
    }
    else {
      RFC_assertion( hf.parent_type() == PARENT_FACE);
      // next inode is the next one along the edge
      HEdge h = v1->halfedge( color);
      if ( h.face() != hf.halfedge().face()) {
	h = h.opposite_g();
	RFC_assertion( h.face() == hf.halfedge().face());
      }
      
      INode_list &il = acc.get_inode_list( h);
      if ( !il.empty()) {
	if ( v1 == &il.back()) 
	  v2 = acc.get_inode(h.destination_g());
	else
	  v2 = v1->next_link[color];
      }
      else {
	INode_list &ilr = acc.get_inode_list( h.opposite_g());
	RFC_assertion( !ilr.empty());
	if ( v1 == &ilr.front())
	  v2 = acc.get_inode(h.destination_g());
	else
	  v2 = v1->prev_link[color];
      }
      RFC_assertion( v2);
      return std::pair<const INode*, HEdge>(v2, h);
    }
  }
  case PARENT_VERTEX: {
    Host_face hf = get_edge_parent( *v0, *v1, color);
    
    HEdge h = v1->halfedge(color);
    while ( h.face() != hf.halfedge().face()) {
      h = acc.get_next_around_origin( h); 
      RFC_assertion( h != v1->halfedge(color));
    }
    
    INode_list &il = acc.get_inode_list( h);
    if ( !il.empty())
      v2 = &il.front();
    else {
      INode_list &ilr = acc.get_inode_list( h.opposite_g());
      if ( !ilr.empty())
	v2 = &ilr.back();
      else
	v2 = acc.get_inode( h.destination_g());
    }
    RFC_assertion( v2);
    return std::pair<const INode*, HEdge>(v2, h);
  }
  default:
    RFC_assertion( false);
    return std::pair<const INode *, HEdge>(NULL,HEdge());
  }
}
  
std::pair<const INode *, HEdge>
Overlay::get_next_inode_cw(const INode *v0, const INode *v1, int color) const 
{
  INode *v2=NULL;
  switch( v1->parent_type( color)) {
  case PARENT_FACE:
    // Rule 1: When making turns, it must be at an edge/vertex.
    return std::pair<const INode *, HEdge>(NULL,HEdge());
  case PARENT_EDGE: {
    Host_face hf = get_edge_parent( *v0, *v1, color);
    if ( hf.parent_type() == PARENT_EDGE) {
      // Rule 2: No three vertices on the same edge can be in the same subface.
      return  std::pair<const INode *, HEdge>(NULL,HEdge());
    }
    else {
      RFC_assertion( hf.parent_type() == PARENT_FACE);
      // next inode is the next one along the edge
      HEdge h = v1->halfedge( color);
      if ( h.face() != hf.halfedge().face()) {
	h = h.opposite_g();
	RFC_assertion( h.face() == hf.halfedge().face());
      }
      
      INode_list &il = acc.get_inode_list( h);
      if ( !il.empty()) {
	if ( v1 == &il.front()) 
	  v2 = acc.get_inode(h.origin_g());
	else
	  v2 = v1->prev_link[color];
      }
      else {
	INode_list &ilr = acc.get_inode_list( h.opposite_g());
	RFC_assertion( !ilr.empty());
	if ( v1 == &ilr.back())
	  v2 = acc.get_inode(h.origin_g());
	else
	  v2 = v1->next_link[color];
      }
      return std::pair<const INode*, HEdge>(v2, h);
    }
  }
  case PARENT_VERTEX: {
    Host_face hf = get_edge_parent( *v0, *v1, color);

    HEdge h = v1->halfedge(color);
    HEdge j = ( (hf.parent_type() == PARENT_EDGE) ?
                hf.halfedge().opposite_g() : hf.halfedge());

    while ( h.face() != j.face()) {
      h = acc.get_next_around_origin( h); 
      RFC_assertion( h != v1->halfedge(color));
    }
    h = h.prev_l();
    
    INode *v2;
    INode_list &il = acc.get_inode_list( h);
    if ( !il.empty())
      v2 = &il.back();
    else {
      INode_list &ilr = acc.get_inode_list( h.opposite_g());
      if ( !ilr.empty())
	v2 = &ilr.front();
      else
	v2 = acc.get_inode( h.origin_g());
    }
    return std::pair<const INode*, HEdge>(v2, h);
  }
  default:
    RFC_assertion( false);
    return std::pair<const INode *, HEdge>(NULL,HEdge());
  }
}

const INode *
Overlay::get_next_inode( const INode *v0, const INode *v1, int color) const {
  // First, find the blue face that contain both inodes.
  std::pair<const INode*,HEdge> b, g;

  b = get_next_inode_ccw( v0, v1, color);
  if ( !is_opposite)
    g = get_next_inode_ccw( v0, v1, !color);
  else
    g = get_next_inode_cw( v0, v1, !color);

  if ( g.first==NULL) return b.first;
  if ( b.first!=NULL) {
    const Host_face bf = get_edge_parent( *v1, *b.first, !color);
  
    if ( !g.second.is_border_g() && 
	 contains( g.second, PARENT_FACE, bf.halfedge(), bf.parent_type()))
      // If the blue one is contained in the green face, return the blue one
      return b.first;
  }

  return g.first;
}

// The following subroutine subdivides a given face into a number of
// sub faces. The input face is given by a list of inodes in CCW order,
// and the output subfaces are given by a list of faces, each of which
// is represented by a vectors of inodes.
// At input, the nodes in the face between face.begin() and 
// last_checked (inclusively) are known belong to the same sub-face.
// Return true if the face is not closed.
bool
Overlay::subdivide( const INode_const_list &face, 
		    INode_const_list::const_iterator last_checked,
		    Subface_list &sub_faces,
		    int color, int depth) const {
  if ( face.size() < 3) return true; // Do nothing
  RFC_assertion( last_checked != face.begin() && last_checked != face.end() && depth < 1000);

  INode_const_list::const_iterator it=last_checked, iend=face.end(),inext;
  const INode *v0 = *(--last_checked), *v1 = *it, *v2;
  // v0 and v1 are the last pair of adjacent nodes that are known belonging
  // to the sub-face of previous nodes in the node list.

  for ( RFC_assertion_code(unsigned int c=0);;RFC_assertion(++c<face.size())) {
    inext = it; ++inext; // Equivalent to inext = it + 1;
    if ( inext == iend) { // All nodes in the list are processed.
      if ( face.front() == get_next_inode( v0, v1, color)) {
	// Insert a new item in the sub-face list.
	sub_faces.push_back( std::vector< const INode*>());
	std::vector< const INode*> &vec = sub_faces.back();
	vec.reserve( face.size());
	vec.insert( vec.end(),face.begin(), face.end());
	return false;
      }
      return true; 
    }
    else {
      v2 = get_next_inode( v0, v1, color);
      if ( !v2) { 
	INode_const_list sub2( it, face.end());
	subdivide( sub2, ++sub2.begin(), sub_faces, color, depth+1);
	return true;
      }
      v0 = v1; v1 = v2;
      if ( v2 != *inext) break; // the face is split at v2
      
      it = inext;
    }
  }

  // The sub-face is a proper subset of given face. We split it into two parts.
  // Create a binary tree structure for the nodes.
  typedef std::map< const INode *, INode_const_list::const_iterator> IMap;
  IMap  imap;
  for ( INode_const_list::const_iterator i = face.begin(); i!=iend; ++i)
    imap.insert( std::make_pair( *i, i));

  IMap::const_iterator mit;
  // Create two sub lists
  std::list< const INode *> sub1, sub2;
  sub1.insert( sub1.end(), face.begin(), inext);

  // Find the end of the chain that cuts the face
  for ( RFC_assertion_code( int count=0);;RFC_assertion(++count<100)) {
    if ( (mit=imap.find( v1)) == imap.end()) {
      sub1.push_back( v1);
      sub2.push_front( v1);
      v2 = get_next_inode( v0, v1, color);
      if (v2) { v0=v1; v1 = v2; } else break;
    }
    else {
      break;
    }
  }

  bool ret1=true, ret2=true;
  // Cut the face along the inodes (*mit->second,sub2.front,...,sub2.end(),*it)
  INode_const_list::const_iterator sub_it1 = --sub1.end();
  if (v2 && mit->second != face.begin())
  { sub1.insert( sub1.end(), mit->second, iend); ++sub_it1; }
  if (v2) ret1 = subdivide( sub1, sub_it1, sub_faces, color, depth+1); 
  sub1.clear();

  if ( v2) sub2.push_front( *mit->second);
  if ( v2 && mit->second != face.begin())
  { sub2.insert(sub2.end(),it,mit->second); }
  else
  { sub2.insert(sub2.end(),it,iend); }
  ret2 = subdivide( sub2, ++sub2.begin(), sub_faces, color, depth+1);
  return ret1 || ret2;
}

HEdge Overlay::get_parent_face( const Subface &sf, int color) {
  RFC_assertion( sf.size()>0);

  Host_face f = get_edge_parent( *sf[0], *sf[1], color);
  if ( !f.halfedge().is_border_l() && 
       contains( f.halfedge(), PARENT_FACE, 
		 sf[2]->halfedge(color), sf[2]->parent_type(color)))
    return f.halfedge();
  else
    return f.halfedge().opposite_g();
}

Point_2
Overlay::get_nat_coor( const INode &i, const Generic_element &e,
		       const HEdge &h, int color) const {
  HEdge hi = i.halfedge( color);
  Point_2 nc; i.nat_coor( color, nc);

  if ( hi.face() != h.face()) {
    switch ( i.parent_type( color)) {
    case PARENT_VERTEX: {
      Node v =  hi.origin_g();
      hi = h;
      while ( hi.origin_g() != v) 
      { hi = hi.next_g(); RFC_assertion( hi != h); }
      break;
    }
    case PARENT_EDGE: {
      HEdge hj = hi.opposite_g();
      if ( hj.face() == h.face()) {
	hi = hj; nc = Point_2( 1.-nc[0], 0.); break;
      }
      // Otherwise, it is in an extension edge and we treat it as a face.
    }
    default: {
      RFC_assertion( hi.is_border_g());

      // In the extension region
      while ( hi.opposite_g() != h) hi = hi.next_g();
      
      return Point_2(1.-get_nat_coor(i, Generic_element(4), hi, color)[0],0.);
    }
    }
  }

  switch( e.size_of_edges()) {
  case 3: {
    if ( hi == h)
      return nc;
    else if ( hi == h.prev_l()) {
      return Point_2( nc[1], 1.-nc[0]-nc[1]);
    }
    else {
      RFC_assertion( hi == h.next_l());
      return Point_2( 1.-nc[0]-nc[1], nc[0]);
    }
  }
  case 4: {
    if ( hi == h)
      return nc;
    else if ( hi == h.prev_l()) {
      return Point_2( nc[1], 1-nc[0]);
    }
    else if ( hi == h.next_l()) {
      return Point_2( 1-nc[1], nc[0]);
    }
    else {
      RFC_assertion( hi == h.next_l().next_l());
      return Point_2( 1-nc[0], 1-nc[1]);
    }
  }
  default: {
    RFC_assertion( false);
  }
  }
  return Point_2(0,0);
}

// Construct a list of nodes for the blue face
void
Overlay::get_inodes_of_face( const Face &f,
			     INode_const_list &nodes) {
  HEdge b = f.halfedge();
  if ( b.is_border_g()) {
    // Make sure the opposite of b is a border edge
    while ( !b.opposite_g().is_border_l()) b=b.next_g(); 
  }
  HEdge h=b;
  do {
    const INode *i=acc.get_inode( h.origin_g());
    if ( i) nodes.push_back( i);
    
    const INode_list &il = acc.get_inode_list( h);
    
    if ( ! il.empty()) {
      // Loop through the intersections on the edges
      INode_list::const_iterator v = il.begin(), vend = il.end();
      for ( ; v != vend; ++v) nodes.push_back( &*v);
    }
    else {
      const INode_list &ilr = acc.get_inode_list( h.opposite_g());
      if ( !ilr.empty()) {
	INode_list::const_reverse_iterator vr=ilr.rbegin(),vrend=ilr.rend();
	for ( ; vr!=vrend; ++vr) nodes.push_back(&*vr);
      }
    }
    
    // Increment the iterator
  } while ( (h = h.next_g()) != b);

  return;
}

void
Overlay::write_overlay_tec( std::ostream &os, const COM::Window *w) {
  int color = (w==B->base()) ? BLUE : GREEN;
  int num_faces=0;
  // Loop through all the original blue faces
  std::vector<RFC_Pane_overlay*>   ps;
  if ( color==GREEN) G->panes( ps); else B->panes( ps);
  // Loop through all the panes of G
  for (std::vector<RFC_Pane_overlay*>::iterator 
	 pit=ps.begin(); pit != ps.end(); ++pit) {
    // Loop through the faces of each pane.
    for ( int i=1, s=(*pit)->size_of_faces(); i<=s; ++i) {
      Face fid( *pit, i); 

      // Construct a list of nodes for the blue face
      INode_const_list nodes;
      get_inodes_of_face( fid, nodes);
      if ( nodes.size() <= 2) continue;

      Subface_list   sub_faces;
      // subdivide the face
      RFC_assertion_code( bool ret = )
	subdivide( nodes, ++nodes.begin(), sub_faces, color);
      RFC_assertion( !ret);
      
      // output the subfaces of the blue face
      num_faces += sub_faces.size();
      Subface_list::iterator sfit = sub_faces.begin(), sfend=sub_faces.end();
      os << "GEOMETRY T=LINE3D C=BLACK"
	 << " LT=0.15\n" << sub_faces.size() << std::endl;
      for ( ; sfit != sfend; ++sfit) {
	// Loop through the points in the sub face
	os << sfit->size()+1 << " ";
	for ( Subface_list::value_type::iterator
		vit=sfit->begin(); vit!=sfit->end(); ++vit) {
	  os << op.get_point( (*vit)->halfedge( color), 
			      (*vit)->nat_coor( color))
	     << std::endl;
	}
	os << op.get_point( (*sfit->begin())->halfedge( color), 
			    (*sfit->begin())->nat_coor( color))
	   << std::endl;
      } 
    }
  }
  if ( verbose)
    std::cout << "\tTotal number of sub-faces in the overlay is " 
	      << num_faces << std::endl;
}

void
Overlay::write_overlay_off( std::ostream &os, const COM::Window *w) {
  int color = (w==B->base()) ? BLUE : GREEN;
  os << "OFF" << std::endl;

  // Loop through all the original blue faces
  std::vector<RFC_Pane_overlay*>   ps;
  if ( color==GREEN) G->panes( ps); else B->panes( ps);

  int nfaces = 0;

  // Loop through all the panes of the window to count the number of faces
  for (std::vector<RFC_Pane_overlay*>::iterator 
	 pit=ps.begin(); pit != ps.end(); ++pit) {
    // Loop through the faces of each pane.
    for (int i=1, s=(*pit)->size_of_faces(); i<=s; ++s) {
      Face fit(*pit, i);

      // Construct a list of nodes for the blue face
      INode_const_list nodes;
      get_inodes_of_face( fit, nodes);
      if ( nodes.size() <= 2) continue;

      Subface_list   sub_faces;
      // subdivide the face
      RFC_assertion_code( bool ret = )
	subdivide( nodes, ++nodes.begin(), sub_faces, color);
      RFC_assertion( !ret);
      
      // output the subfaces of the blue face
      nfaces += sub_faces.size();
    }
  }

  os << inodes.size() << ' ' << nfaces << " 0" << std::endl;

  // Assign ids for the vertices
  int id=0;
  std::map< const void*, int>   ids;
  for ( std::list< INode*>::const_iterator
	  i = inodes.begin(); i != inodes.end(); ++i) {
    os << op.get_point( (*i)->halfedge( color), 
			(*i)->nat_coor( color)) 
       << std::endl;
    ids[*i] = id++;
  }

  // Loop through all the original blue faces
  for (std::vector<RFC_Pane_overlay*>::iterator 
	 pit=ps.begin(); pit != ps.end(); ++pit) {
    // Loop through the faces of each pane.

    for (int i=1, s=(*pit)->size_of_faces(); i<=s; ++s) {
      Face fit(*pit, i);

      // Construct a list of nodes for the blue face
      INode_const_list nodes;
      get_inodes_of_face( fit, nodes);
      if ( nodes.size() <= 2) continue;

      Subface_list   sub_faces;
      // subdivide the face
      RFC_assertion_code( bool ret = )
	subdivide( nodes, ++nodes.begin(), sub_faces, color);
      RFC_assertion( !ret);
      
      // output the subfaces of the blue face
      Subface_list::iterator sfit = sub_faces.begin(), sfend=sub_faces.end();

      for ( ; sfit != sfend; ++sfit) {
	// Loop through the points in the sub face
	os << sfit->size();
	for ( Subface_list::value_type::iterator
		vit=sfit->begin(); vit!=sfit->end(); ++vit) {
	  os << " " << ids[*vit];
	}
	os << std::endl;
      } 
    }
  }

  if ( verbose)
    std::cout << "\tTotal number of sub-faces in the overlay is " 
	      << nfaces << std::endl;
}

RFC_END_NAME_SPACE

