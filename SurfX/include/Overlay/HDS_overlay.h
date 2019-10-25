//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

//==============================================
//  This file contains definition of the halfedge data structure for
//     the overlay algorithm.
//
//  Author: Xiangmin Jiao
//  Optimized: Apr. 4, 2002
//==============================================

#ifndef HDS_OVERLAY_H
#define HDS_OVERLAY_H

#include <iterator>
#include "In_place_list.h"
#include "Manifold_2.h"
#include "rfc_basic.h"

RFC_BEGIN_NAME_SPACE

using SURF::Access_Mode;
typedef SURF::Pane_manifold_2::Edge_ID Edge_ID;

class RFC_Pane_overlay;
class Node;
class HEdge;
class Face;

/** This class encapsulate a node over a window manifold. */
class Node {
 public:
  /// Default constructor
  Node() : _pm(NULL), _vID(0) {}

  /// Construct a Node object from a pane manifold and its node ID
  Node(RFC_Pane_overlay *pm, int vid) : _pm(pm), _vID(vid) {}

  /** Get an incident halfedge destinated at the node. If the node is on
   *  the physical boundary, the halfedge must be a physical border edge. */
  inline HEdge halfedge_l() const;
  inline HEdge halfedge_g() const;

  /// Obtain the pane manifold that owns the node.
  inline RFC_Pane_overlay *pane() const { return _pm; }

  /// Obtain the node ID within its RFC_Pane_overlay.
  inline int id() const { return _vID; }

  /// Obtain the coordinates of a node
  inline const Point_3 &point() const;

  inline bool is_border_l() const;

  inline bool is_border_g() const;

  inline bool is_isolated() const;

  inline bool is_primary() const;

  inline Node get_primary() const;

  inline bool operator==(const Node &v) const {
    return _pm == v._pm && _vID == v._vID;
  }

  inline bool operator!=(const Node &v) const {
    return _pm != v._pm || _vID != v._vID;
  }

  inline bool operator<(const Node &v) const;

 protected:
  RFC_Pane_overlay *_pm;  // Reference to RFC_Pane_overlay
  int _vID;               // Node id
};

/** This class encapsulate a halfedge over a window manifold. */
class HEdge {
 public:
  HEdge() : _pm(NULL) {}
  /// Construct a Halfedge object from a pane manifold and its edge ID
  HEdge(RFC_Pane_overlay *pm, Edge_ID e) : _pm(pm), _eID(e) {}

  /// Get the ID of the opposite edge of a given edge
  inline HEdge opposite_l() const;
  inline HEdge opposite_g() const;

  /// Get the previous halfedge of its owner element. If the current edge
  /// is a border edges, it returns the previous border edge along
  /// the boundary.
  inline HEdge prev_l() const;
  inline HEdge prev_g() const;

  /// Get the next halfedge of its owner element. If the current edge
  /// is a border edges, it returns the next border edge along
  /// the boundary.
  inline HEdge next_l() const;
  inline HEdge next_g() const;

  /// Obtain the (primary copy of the) origin of the edge
  inline Node origin_l() const;
  inline Node origin_g() const;

  /// Obtain the (primary copy of the) destination of the edge
  inline Node destination_g() const;

  /// Obtain the (primary copy of the) destination of the edge
  inline Node destination_l() const;

  /// Obtain the (primary copy of the) origin of the edge
  inline Face face() const;

  /// Is the edge a border edge?
  inline bool is_border_l() const;
  inline bool is_border_g() const;

  /// Obtain the pane manifold that owns the edge.
  inline RFC_Pane_overlay *pane() const { return _pm; }

  /// Obtain the ID of the edge.
  inline Edge_ID id() const { return _eID; }

  /// Are the two halfedges equal?
  inline bool operator==(const HEdge &h) const {
    return _pm == h._pm && _eID == h._eID;
  }

  inline bool operator!=(const HEdge &h) const {
    return _pm != h._pm || _eID != h._eID;
  }

  /// Does the current halfedge has a smaller ID then the given one?
  inline bool operator<(const HEdge &h) const;

 protected:
  RFC_Pane_overlay *_pm;  // Reference to RFC_Pane_overlay
  Edge_ID _eID;           // Edge ID
};

/** This class encapsulate a face in a window manifold. */
class Face {
 public:
  Face() : _pm(NULL), _fID(0) {}
  /// Construct a Face object from a pane manifold and its face ID
  Face(RFC_Pane_overlay *pm, int fid) : _pm(pm), _fID(fid) {}

  /// Obtain the pane manifold that owns the edge.
  inline RFC_Pane_overlay *pane() const { return _pm; }

  /// Obtain the ID of the edge.
  inline int id() const { return _fID; }

  inline HEdge halfedge() const { return HEdge(_pm, Edge_ID(_fID, 0)); }

  /// Are the two halfedges equal?
  inline bool operator==(const Face &f) const {
    return _pm == f._pm && _fID == f._fID;
  }

  inline bool operator!=(const Face &f) const {
    return _pm != f._pm || _fID != f._fID;
  }

  /// Does the current face has a smaller ID then the given one?
  inline bool operator<(const Face &f) const;

 protected:
  RFC_Pane_overlay *_pm;  // Reference to RFC_Pane_overlay
  int _fID;               // Face ID
};

struct Host_face {
  Host_face() : _h(), _t(PARENT_NONE){};
  Host_face(HEdge h, Parent_type t) : _h(h), _t(t) {}

  HEdge halfedge() const { return _h; }
  Parent_type parent_type() const { return _t; }
  bool operator==(const Host_face &h) const { return _h == h._h && _t == h._t; }
  bool operator!=(const Host_face &h) const { return _h != h._h || _t != h._t; }

 private:
  HEdge _h;
  Parent_type _t;
};

class INode : private In_place_list_base<INode, 2> {
 public:
  typedef INode Self;
  typedef In_place_list_base<INode, 2> Base;

  using Base::next_link;
  using Base::prev_link;

 protected:
  Host_face bp;   // Blue parent object
  Host_face gp;   // Green parent object
  Point_2S b_nc;  // The natrual coordinate in the original blue mesh
  Point_2S g_nc;  // The natrual coordinate in the original green mesh
  int _id;

 public:
  INode() : _id(-1) {}
  ~INode() { RFC_assertion_code(bp = gp = Host_face()); }

  bool operator==(const INode &x) {
    return (b_nc == x.b_nc && g_nc == x.g_nc && bp == x.bp && gp == x.gp);
  }

  const Point_2S &nat_coor(const int color) const {
    if (color == BLUE)
      return b_nc;
    else {
      RFC_assertion(color == GREEN);
      return g_nc;
    }
  }

  void nat_coor(const int color, Point_2 &p) const {
    if (color == BLUE) {
      p[0] = b_nc[0];
      p[1] = b_nc[1];
    } else {
      RFC_assertion(color == GREEN);
      p[0] = g_nc[0];
      p[1] = g_nc[1];
    }
  }

  void set_parent(HEdge h, const Point_2 &p, int color) {
    if (color == BLUE) {
      bp = Host_face(h, pt(p));
      b_nc[0] = p[0];
      b_nc[1] = p[1];
    } else {
      RFC_assertion(color == GREEN);
      gp = Host_face(h, pt(p));
      g_nc[0] = p[0];
      g_nc[1] = p[1];
    }
  }

  int id() const { return _id; }
  void set_id(int i) { _id = i; }

  Parent_type parent_type(const int color) const {
    if (color == BLUE) {
      return bp.parent_type();
    } else {
      RFC_assertion(color == GREEN);
      return gp.parent_type();
    }
  }
  Parent_type blue_parent_type() const { return bp.parent_type(); }
  Parent_type green_parent_type() const { return gp.parent_type(); }

  HEdge halfedge(const int color) const {
    if (color == BLUE) {
      return bp.halfedge();
    } else {
      RFC_assertion(color == GREEN);
      return gp.halfedge();
    }
  }

 private:
  Parent_type pt(const Point_2 &p) {
    if (p[1] != 0)
      return PARENT_FACE;
    else if (p[0] != 0)
      return PARENT_EDGE;
    else
      return PARENT_VERTEX;
  }
};

RFC_END_NAME_SPACE

#endif
