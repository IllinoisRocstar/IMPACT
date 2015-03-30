//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

//==========================================================
// The implementation of the function for triangulating a 2-D convex 
// region and some helper functions .
//
// Author: Xiangmin Jiao
//==========================================================
#include "Triangulation.h"


USE_RFC_NAME_SPACE;

inline bool leftturn( const Point_2 &p, const Point_2 &q, const Point_2 &r) {
  Real det = (p.x()-r.x()) * (q.y()-r.y()) - (q.x()-r.x()) *(p.y()-r.y());
  return det > 0;
}
	
inline bool rightturn( const Point_2 &p, const Point_2 &q, const Point_2 &r) {
  Real det = (p.x()-r.x()) * (q.y()-r.y()) - (q.x()-r.x()) *(p.y()-r.y());
  return det < 0;
}

inline bool do_intersect_intervals( double iv1[2], double iv2[2]) {
  int i2min = ( iv2[1] <  iv2[0]) + 1;

  if ( iv1[1] >  iv1[0]) {
    return ( iv1[0] >=  iv2[i2min - 1]) && ( iv1[0] <=  iv2[2 - i2min]) ||
      ( iv2[i2min - 1] <=  iv1[1]) && ( iv2[i2min - 1] >= iv1[0]) ||
      ( iv2[2 - i2min] <=  iv1[1]) && ( iv2[2 - i2min] >= iv1[0]) ||
      ( iv1[1] >=  iv2[i2min - 1]) && ( iv1[1] <=  iv2[2 - i2min]);
  }
  else {
    return ( iv2[2 - i2min] >=  iv1[1]) && ( iv2[2 - i2min] <= iv1[0])||
      ( iv2[i2min - 1] >=  iv1[1]) && ( iv2[i2min - 1] <= iv1[0]) ||
      ( iv1[1] >= iv2[i2min - 1]) &&( iv1[1] <=  iv2[2 - i2min]);
  }
}

static bool do_intersect( const Point_2 &p1, const Point_2 &p2, 
                          const Point_2 &q1, const Point_2 &q2) {
  Real   u1[2], v21[2], v22[2], u2[2], v11[2];
  Real   a11, a12, a21, a22, sqn1;
  Real   v12[2], uref[2];
  Real   sqn2;
  static Real dtemp_2[2] = {0.0, 1.0};
  int i;

   /* Compute the signed areas a11, a12, a21, and a22.  */
  u1[0] =  p2[0] -  p1[0];
  u1[1] =  p2[1] -  p1[1];

  v21[0] =  q1[0] -  p1[0];
  v21[1] =  q1[1] -  p1[1];
  v22[0] =  q2[0] -  p1[0];
  v22[1] =  q2[1] -  p1[1];

  a11 = -u1[1] *  v21[0];
  a11 = a11 +  u1[0] *  v21[1];
  a12 = -u1[1] *  v22[0];
  a12 = a12 +  u1[0] *  v22[1];

  u2[0] =  q2[0] -  q1[0];
  u2[1] =  q2[1] -  q1[1];

  v11[0] =  p1[0] -  q1[0];
  v11[1] =  p1[1] -  q1[1];
  v12[0] =  p2[0] -  q1[0];
  v12[1] =  p2[1] -  q1[1];

  a21 = -u2[1] *  v11[0];
  a21 = a21 +  u2[0] *  v11[1];
  a22 = - u2[1] *  v12[0];
  a22 = a22 +  u2[0] *  v12[1];

  if (a11 == 0.0 && a12 == 0.0) {
    /* Two line segments are colinear. Use intersect_intervals. */
    sqn1 = 0.0;
    sqn2 = 0.0;
    for (i=0; i<=1; i+=1) {
      sqn1 = sqn1 +  u1[i] *  u1[i];
      sqn2 = sqn2 +  u2[i] *  u2[i];
    }
    if (sqn1 > sqn2) {
      uref[0] =  u1[0] / sqn1;
      uref[1] =  u1[1] / sqn1;

      Real dtemp_0[2];
      dtemp_0[0] =  v21[0] *  uref[0] +  v21[1] *  uref[1];
      dtemp_0[1] =  v22[0] *  uref[0] +  v22[1] *  uref[1];
      return do_intersect_intervals(dtemp_2, dtemp_0);
    }
    else if (sqn2 > 0.0) {
      uref[0] =  u2[0] / sqn2;
      uref[1] =  u2[1] / sqn2;

      Real dtemp_1[2];
      dtemp_1[0] =  v11[0] *  uref[0] +  v11[1] *  uref[1];
      dtemp_1[1] =  v12[0] *  uref[0] +  v12[1] *  uref[1];
      return do_intersect_intervals(dtemp_1, dtemp_2);
    }
    else {
      for (i=0; i<=1; i+=2) {
        if ( p1[i] !=  q1[i] || p1[1 + i] !=  q1[1 + i])
          return false;
      }
      return true;
    }
  }
  else if (a12 <= 0.0 && a11 >= 0.0 || a12 >= 0.0 && a11 <= 0.0) {
    return a22 <= 0.0 && a21 >= 0.0 || a22 >= 0.0 && a21 <= 0.0;
  }
  else {
    return false;
  }
}

RFC_BEGIN_NAME_SPACE

bool Triangulation::in_cone( Node_const_iterator u, Node_const_iterator v) const {
  Node_const_iterator a0 = get_prev( u);
  Node_const_iterator a1 = get_next( u);
      	
  if ( !rightturn( get_point(*u), get_point(*a1), get_point(*a0))) {
    return ( leftturn(get_point(*u), get_point(*v), get_point(*a0)) &&
             leftturn(get_point(*v), get_point(*u), get_point(*a1)));
  }
      	
  return ( !rightturn(get_point(*v), get_point(*u), get_point(*a1)) &&
           !rightturn(get_point(*a0), get_point(*u), get_point(*v)));
}

bool Triangulation::is_diagonalie( Node_const_iterator u, Node_const_iterator v) const {
  for ( Node_const_iterator i=_nodes.begin(); i!=_nodes.end(); ++i) {
    Node_const_iterator j = get_next(i);
      		
    if ( u!=i && u!=j && v!=i && v!=j &&
         do_intersect( get_point(*u), get_point(*v),
                       get_point(*i), get_point(*j)))
      return false;
  }
  return true;
}


void Triangulation::triangulate( const Point_2 *ps, int n, Connectivity *tri) {
  _pnts = ps;
  _nodes.clear();
  for ( int i=0; i<n; ++i) _nodes.push_back( Node(i));
      	
  // Initialize ears
  for ( Node_iterator it=_nodes.begin(); it!=_nodes.end(); ++it) {
    it->set_ear( is_diagonal( get_prev(it), get_next(it)));
  }
      	
  tri->resize(0); tri->reserve( n-2);
  // Loop through to remove ears.
  for ( ; n>3; --n) {
    // The inner loop searches for an ear
    for ( Node_iterator it=_nodes.begin(); it!=_nodes.end(); ++it) {
      if ( it->is_ear()) {
        Node_iterator v1=get_prev( it), v0=get_prev( v1);
        Node_iterator v3=get_next( it), v4=get_next( v3);
      				
        // Output the ear
        tri->push_back( Triangle( v1->id(), it->id(), v3->id()));
      				
        // update neighbor vertices
        v1->set_ear( is_diagonal( v0, v3));
        v3->set_ear( is_diagonal( v1, v4));
      				
        // Cut off the ear
        _nodes.erase( it);
        break;
      }
    } 
  }
  RFC_assertion( n==3);
      	
  tri->push_back( Triangle( _nodes.front().id(), (++_nodes.begin())->id(),
                            _nodes.back().id()));
  _nodes.clear();
}

RFC_END_NAME_SPACE

