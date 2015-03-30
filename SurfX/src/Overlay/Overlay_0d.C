//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "Overlay.h"
#include <cmath>
#include "KD_tree_3.h" // surfmap

RFC_BEGIN_NAME_SPACE

// Match 0-dimensional features.
void Overlay::
match_features_0() {
 
  const Real w2e_ratio = 1;
  const Real d2e_ratio_sq = 1;
  
  std::list<Feature_0>  &bf0 = B->flist_0();
  std::list<Feature_0>  &gf0 = G->flist_0();
  
  std::vector< Point_3> gf0_1;
  std::vector< Node>    verts;

  gf0_1.reserve( gf0.size());
  verts.reserve( gf0.size());

  for ( std::list<Feature_0>::iterator it=gf0.begin(); it!=gf0.end(); ++it) {
    gf0_1.push_back( it->point());
    verts.push_back( it->vertex());
  }

  // Create a 3-dimensional KD-tree for the 0-features of green mesh
  KD_tree_3 kdtree( &gf0_1[0][0], gf0.size());

  int dropped=0;
  // Query every 0-feature of the blue mesh in the KD-tree to find
  //      whether there is a unique corresponding one in the other mesh.
  for ( std::list<Feature_0>::iterator
	  it=bf0.begin(), iend=bf0.end(); it != iend; ++it) {
    Node v = it->vertex();
    // Find the corresponding green feature of *it in the KD-tree
    const Point_3 &p = v.point();

    // Compute the tolerance.
    Real tol=HUGE_VAL;
    HEdge b=v.halfedge_g(), b0=b;
    do {
      Real t = sq_length( b);
      if ( t<tol) tol=t;
    } while ( (b=acc.get_next_around_destination( b))!=b0);
    tol = std::sqrt( tol);

    int *indices;
    int nfound = kdtree.search( &p[0], tol*w2e_ratio, &indices);

    // Find the closest corner in the list
    Real dist_min = HUGE_VAL;
    int k=-1;

    for ( int i=0; i<nfound; ++i) {
      Real d=(p-gf0_1[indices[i]]).squared_norm();
      if ( d <= dist_min) {
	dist_min = d; k=i;
      }
    }

    if ( dist_min<HUGE_VAL) {
      HEdge g = verts[indices[k]].halfedge_g();
      if ( g.is_border_g()) g = g.opposite_g();
      else g = g.next_g();

      Real s=tol*tol;
      HEdge h=g, h0=h;
      do {
	Real t = sq_length( h);
	if ( t<s) s=t;
      } while ( (h=acc.get_next_around_origin( h))!=h0);

      if ( dist_min < s*d2e_ratio_sq) {
	if ( verbose2) {
	  std::cout << "Matched\t blue vertex " 
		    << b.pane()->get_index(b.origin_l())+1 
		    << " in pane " << b.pane()->id() << " at " << p
		    << "\n  with\t green vertex " 
		    << g.pane()->get_index(g.origin_l())+1 
		    << " in pane " << g.pane()->id() << " at " 
		    << g.origin_l().point() << std::endl;
	}

	HEdge b = v.halfedge_g();
	if ( b.is_border_g()) b = b.opposite_g();
	else b = b.next_g();
	
	// Create an inode for the o-feature
	INode *x = new INode();
	x->set_parent( b, Point_2(0,0), BLUE);
	x->set_parent( g, Point_2(0,0), GREEN);

	if ( B->snap_on_features()) {
	  // Update the coordinates for the blue point
	  Point_3 &pnt = const_cast<Point_3&>( b.origin_g().point());
	  pnt = g.origin_g().point();
	}

	insert_node_in_blue_edge( *x, b);
	continue;
      }
    }

    if ( verbose) {
      std::cout << "\nRocface Warning: Dropping blue corner vertex " 
		<< b.pane()->get_index(b.origin_l())+1 
		<< " in pane " << b.pane()->id() 
		<< " at " << p << std::endl;
    }

    // Remove the false blue 0-feature
    it = --B->remove_feature_0( it);
    ++dropped;
  }

  if ( verbose && dropped) {
    std::cout << "Dropped " << dropped << " corners in \"" << B->name() 
	      << "\" after feature matching" << std::endl;
  }

  dropped=0;
  // Remove the false green 0-features
  for ( std::list<Feature_0>::iterator
	  it=gf0.begin(), iend=gf0.end(); it != iend; ++it) {
    Node v = it->vertex();
    if ( acc.get_inode( v) == NULL) {

      if ( verbose) {
	std::cout << "\nRocface Warning: Dropping green corner vertex " 
		  << v.pane()->get_index(v)+1 
		  << " in pane " << v.pane()->id() 
		  << " at " << v.point() << std::endl;
      }
      
      // Unmark the 0-features in the green window
      it = --G->remove_feature_0( it);
      ++dropped;
    }
  }

  if ( verbose && dropped) {
    std::cout << "Dropped " << dropped << " corners in \"" << G->name() 
	      << "\" after feature matching" << std::endl;
  }
}

RFC_END_NAME_SPACE


