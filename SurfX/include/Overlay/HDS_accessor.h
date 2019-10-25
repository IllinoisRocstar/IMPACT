//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

//=======================================================================
//  This file contains the definition of an accessor of the halfedge data
//     structure (HDS). The overlay algorithms uses this accessor to access
//     information of HDS instead of directly using member functions of HDS.
//
//  Author: Xiangmin Jiao
//  Last modified: Feb. 09, 2001
//=======================================================================

#ifndef RFC_HDS_ACCESSOR_H
#define RFC_HDS_ACCESSOR_H

#include "RFC_Window_overlay.h"
#include "rfc_basic.h"

RFC_BEGIN_NAME_SPACE

class HDS_accessor {
 public:
  // TYPES
  // ----------------------------------
  // Point needed for Vertex constructor for efficiency reasons.
  typedef Point_3 Point;
  typedef Vector_3 Vector;

  // Access Functions
  // ----------------------------------

  HEdge get_prev_around_origin(const HEdge &h) const {
    return h.prev_g().opposite_g();
  }

  HEdge get_next_around_origin(const HEdge &h) const {
    return h.opposite_g().next_g();
  }

  HEdge get_prev_around_destination(const HEdge &h) const {
    return h.opposite_g().prev_g();
  }

  HEdge get_next_around_destination(const HEdge &h) const {
    return h.next_g().opposite_g();
  }

  // Const Access Functions
  // ----------------------------------
  bool is_on_feature(const Node &v) const {
    Node vprim = v.get_primary();
    return vprim.pane()->is_on_feature(vprim);
  }

  bool is_feature_0(const Node &v) const {
    Node vprim = v.get_primary();
    return vprim.pane()->is_feature_0(vprim);
  }

  bool is_feature_1(const HEdge &h) const { return h.pane()->is_feature_1(h); }

  const Vector_3 &get_normal(const HEdge &h) const {
    return h.pane()->get_normal(h, h.destination_l());
  }

  const Vector_3 &get_normal(const HEdge &h, const Node &v) const {
    return h.pane()->get_normal(h, v);
  }

  void mark(const HEdge &h) const { h.pane()->mark(h); }
  void unmark(const HEdge &h) const { h.pane()->unmark(h); }
  bool marked(const HEdge &h) const { return h.pane()->marked(h); }

  INode *get_inode(const Node &v) const {
    RFC_assertion(v.is_primary());
    return v.pane()->get_inode(v);
  }
  void set_inode(const Node &v, INode *i) const {
    RFC_assertion(v.is_primary());
    v.pane()->set_inode(v, i);
  }

  INode_list &get_inode_list(const HEdge &h) const {
    return h.pane()->get_inode_list(h);
  }
};

RFC_END_NAME_SPACE

#endif  // RFC_CHDS_DECORATOR_H //
// EOF //
