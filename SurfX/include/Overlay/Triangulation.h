//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

// This file implements the ear-removal algorithm for triangulating a
//   convex polygon described in "Computational Geometry in C".

#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <list>
#include <vector>
#include "Tuple.h"
#include "rfc_basic.h"

RFC_BEGIN_NAME_SPACE

//! Triangulating a convex polygon by ear-removal.
/*!
  This class implements the ear-removal algorithm described in the book
  "Computational Geometry in C" for triangulating a convex polygon.
  It triangulates a polygon by cutting off ears incrementally until only
  one triangle is left. This method may generate triangles with poor quality,
  but quality is not a concern because we will compute integration instead
  of differentiation.
*/
class Triangulation {
 private:
  // A helper class for tracking information about a node of the polygon.
  class Node {
    short int _id;
    bool _ear;

   public:
    //! A constructor.
    explicit Node(short int i) : _id(i), _ear(false) {}

    int id() const { return _id; }
    bool is_ear() const { return _ear; }
    void set_ear(bool b) { _ear = b; }
  };

  typedef std::list<Node> Node_list;
  typedef Node_list::iterator Node_iterator;
  typedef Node_list::const_iterator Node_const_iterator;

 public:
  typedef Three_tuple<int> Triangle;
  //!< Connectivity for a triangle.
  typedef std::vector<Triangle> Connectivity;
  //!< Element connectivity table.

  //! A constructor.
  Triangulation() {}

  //! Main entry for triangulation.
  /*!
    \param ps the coordinates of the nodes.
    \param n  the number of nodes.
    \param tri the element connectivity for output.
  */
  void triangulate(const Point_2 *ps, int n, Connectivity *tri);

 private:
  //============= Helper subroutines
  const Point_2 &get_point(const Node &v) const {
    return (Point_2 &)_pnts[v.id()];
  }

  Node_iterator get_next(Node_iterator i) {
    return (++i == _nodes.end()) ? _nodes.begin() : i;
  }
  Node_iterator get_prev(Node_iterator i) {
    if (i == _nodes.begin()) i = _nodes.end();
    return --i;
  }

  Node_const_iterator get_next(Node_const_iterator i) const {
    return (++i == _nodes.end()) ? _nodes.begin() : i;
  }
  Node_const_iterator get_prev(Node_const_iterator i) const {
    if (i == _nodes.begin()) i = _nodes.end();
    return --i;
  }

  bool is_diagonal(Node_const_iterator u, Node_const_iterator v) const {
    return in_cone(u, v) && in_cone(v, u) && is_diagonalie(u, v);
  }

  bool in_cone(Node_const_iterator u, Node_const_iterator v) const;

  bool is_diagonalie(Node_const_iterator u, Node_const_iterator v) const;

 private:  // Data members
  const Point_2 *_pnts;
  Node_list _nodes;
};

RFC_END_NAME_SPACE

#endif
