//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

//===============================================================
// This file defines RFC_Pane_overlay and RFC_Window_overlay based on
//    RFC_Pane_base and RFC_Window_base respectively.
// Author: Xiangmin Jiao
// Creation date:   Dec. 21, 2001
//===============================================================

#ifndef RFC_WINDOW_H
#define RFC_WINDOW_H

#include <cmath>
#include <list>
#include <map>
#include "HDS_overlay.h"
#include "In_place_list.h"
#include "RFC_Window_base.h"
#include "commpi.h"

#ifndef HUGE_VALF
#define HUGE_VALF 1e+36F
#endif

RFC_BEGIN_NAME_SPACE

class INode;
class HDS_accessor;
typedef In_place_list<INode, false> INode_list;

class RFC_Window_overlay;

// RFC_Pane_overlay is based on RFC_Pane_base with the extension of
//    a halfedge structure for storing element connectivity and
//    extra dataitem for storing normals of all vertices.
class RFC_Pane_overlay : public RFC_Pane_base, protected SURF::Pane_manifold_2 {
 public:
  typedef RFC_Pane_overlay Self;
  typedef RFC_Pane_base Base;
  typedef MAP::Simple_manifold_2::Edge_ID Edge_ID;

  friend class RFC_Window_overlay;
  friend class Node;
  friend class HEdge;
  friend class Face;

  using RFC_Pane_base::is_border_node;
  using RFC_Pane_base::is_isolated_node;
  using RFC_Pane_base::size_of_faces;
  using RFC_Pane_base::size_of_nodes;

  RFC_Pane_overlay(COM::Pane *b, int color);
  virtual ~RFC_Pane_overlay() {}

  RFC_Window_overlay *window() { return _window; }
  const RFC_Window_overlay *window() const { return _window; }

  const HEdge &get_counterpart(Edge_ID eID) const {
    COM_assertion(eID.is_border());
    return (HEdge &)_cntrs_hedge[eID.eid() - 1];
  }

  /// Obtain the opposite edge in a given access mode
  inline HEdge get_opposite_edge(Edge_ID eID) const {
    Edge_ID opp = get_opposite_real_edge(eID);

    if (opp.is_border()) return get_counterpart(opp);
    return HEdge(const_cast<RFC_Pane_overlay *>(this), opp);
  }

  /// Obtain the previous edge in a given access mode
  inline HEdge get_prev_edge(Edge_ID eID) {
    if (!eID.is_border()) {
      Edge_ID prv = get_prev_real_edge(eID);
      return HEdge(this, prv);
    } else {  // the edge is on physical boundary.
      HEdge h = get_opposite_edge(eID);
      while (!h.id().is_border())
        h = HEdge(h.pane(), h.pane()->get_next_real_edge(h.id())).opposite_g();
      return h;
    }
  }

  /// Obtain the next edge in a given access mode
  inline HEdge get_next_edge(Edge_ID eID) {
    if (!eID.is_border()) {
      Edge_ID nxt = get_next_real_edge(eID);
      return HEdge(this, nxt);
    } else {  // the edge is on physical boundary.
      HEdge h = get_opposite_edge(eID);
      while (!h.id().is_border())
        h = HEdge(h.pane(), h.pane()->get_prev_real_edge(h.id())).opposite_g();
      return h;
    }
  }

  // Functions for supporting the DCEL data structure
  int get_index(const Node &v) const { return v.id() - 1; }
  int get_index(const HEdge &h) const { return get_index(h.id()); }
  int get_index(const Edge_ID &id) const {
    if (id.is_border())
      return size_of_faces() * 4 + id.eid() - 1;
    else
      return (id.eid() - 1) * 4 + id.lid();
  }

  int get_index(const Face &f) const { return f.id() - 1; }
  int get_lid(const Node &v) const { return v.id(); }
  int get_lid(const Face &f) const { return f.id(); }

  Node get_node_from_id(int i) {
    RFC_assertion(i > 0);
    return Node(this, i);
  }

  // If a vertex is a border vertex, it can have instances in multiple
  // panes. The instance in the pane with smallest id is the primary copy.
  bool is_primary(const Node &v) const {
    return !v.is_border_l() ||
           _primnodes[get_border_index(v.halfedge_l())] == v;
  }

  const Node get_primary(const Node &v) const {
    if (!v.is_border_l())
      return v;
    else
      return _primnodes[get_border_index(v.halfedge_l())];
  }

  const Point_3 &get_point(int id) const { return Base::get_point(id); }

  const Point_3 &get_point(const Node &v) const {
    return Base::get_point(get_index(v), 0);
  }

  HEdge get_counterpart(const HEdge &h) {
    if (!h.id().is_border())
      return h;
    else
      return get_counterpart(h.id());
  }

  // Obtaining the normal direction of a vertex
  Vector_3 &get_normal(int v) {
    RFC_assertion(v > 0);
    return _nrmls[v - 1];
  }
  const Vector_3 &get_normal(int v) const {
    RFC_assertion(v > 0);
    return _nrmls[v - 1];
  }
  void set_normal(int v, const Vector_3 &vec) {
    RFC_assertion(v > 0);
    _nrmls[v - 1] = vec;
  }

  // Get the normal of a vertex.
  Vector_3 &get_normal(const HEdge &h, const Node &v) {
    return const_cast<Vector_3 &>(
        ((const RFC_Pane_overlay *)this)->get_normal(h, v));
  }

  Vector_3 &get_tangent(const HEdge &h, const Node &v) {
    return const_cast<Vector_3 &>(
        ((const RFC_Pane_overlay *)this)->get_tangent(h, v));
  }

  Vector_3 &get_normal(const HEdge &h, int i) {
    return get_normal(h, get_node_from_id(i));
  }

  const Vector_3 &get_normal(const HEdge &h, int i) const {
    return get_normal(
        h, const_cast<RFC_Pane_overlay *>(this)->get_node_from_id(i));
  }

  const Vector_3 &get_normal(HEdge h, const Node &v) const;
  const Vector_3 &get_tangent(HEdge h, const Node &v) const;

  void add_tangent(const HEdge &h, const Vector_3 &v) {
    int &ind = _f_t_index[get_index(h)];
    if (ind < 0) {
      ind = _f_tngts.size();
      _f_tngts.push_back(v);
    } else
      _f_tngts[ind] += v;
  }

  INode *get_inode(const Node &v) const {
    RFC_assertion(is_primary(v));
    return _v_nodes[get_index(v)];
  }
  void set_inode(const Node &v, INode *i) {
    RFC_assertion(is_primary(v));
    _v_nodes[get_index(v)] = i;
  }
  INode_list &get_inode_list(const HEdge &h) {
    return _e_node_list[get_index(h)];
  }
  const INode_list &get_inode_list(const HEdge &h) const {
    return _e_node_list[get_index(h)];
  }

  INode *get_buffered_inode(const HEdge &h, int tag) const {
    int i = get_index(h);
    if (_e_marks[i] == tag)
      return _e_node_buf[i];
    else
      return NULL;
  }
  void set_buffered_inode(const HEdge &h, int tag, INode *inode) {
    int i = get_index(h);
    _e_marks[i] = tag;
    _e_node_buf[i] = inode;
  }

  void mark(const HEdge &h) { _e_marks[get_index(h)] = true; }
  void unmark(const HEdge &h) { _e_marks[get_index(h)] = false; }
  bool marked(const HEdge &h) const { return _e_marks[get_index(h)]; }

  bool is_on_feature(const Node &v) const {
    assert(v.is_primary());
    return _is_on_f[get_index(v)];
  }
  bool is_feature_0(const Node &v) const {
    assert(v.is_primary());
    return _is_f_0[get_index(v)];
  }
  bool is_feature_1(const HEdge &h) const { return _is_f_1[get_index(h)]; }

  // ===========================
  // Routines for building the subdivision
  const int &size_of_subfaces() const { return _size_of_subfaces; }
  int &size_of_subfaces() { return _size_of_subfaces; }

  //! Allocate memory space for the arrays for storing the subdivision.
  void allocate_subnodes(int num_sn) {
    _subnode_parents.resize(num_sn);
    _subnode_nat_coors.resize(num_sn);
    _subnode_counterparts.resize(num_sn);
  }

  //! Allocate memory space for the arrays for storing the subdivision.
  void allocate_subfaces(const std::vector<int> &cnts) {
    int nf = size_of_faces();
    RFC_assertion(nf == int(cnts.size()));
    _subface_offsets.resize(nf + 1);
    _subface_offsets[0] = 0;
    for (int i = 1; i <= nf; ++i) {
      _subface_offsets[i] = _subface_offsets[i - 1] + cnts[i - 1];
    }

    int nsf = _subface_offsets[nf];
    _subfaces.resize(nsf);
    _subface_parents.resize(nsf);
    _subface_counterparts.resize(nsf);
  }

  //! Insert all the infomation related to a subface into the database.
  void insert_subface(int idx, int plid, const int *lids,
                      const ParentEdge_ID *eids, const Point_2 *nc, int rp_id,
                      int cnt, const int *rids);

 protected:
  //=========================================================
  // The following functions are for constructing the internal
  // data structure of RFC_Pane.
  //=========================================================

  // Evaluating normals for all vertices within a pane.
  void evaluate_normals();

  // Determing counterparts of local border halfedges.
  void determine_counterparts();

  void unmark_alledges();

  //=========== Functions for supporting feature detection
  float &get_cos_face_angle(const HEdge &h) {
    Edge_ID opp = get_opposite_real_edge(h.id());
    int n1 = get_index(h), n2 = get_index(opp);
    return _fd_1[std::min(n1, n2)];
  }
  const float &get_cos_face_angle(const HEdge &h) const {
    Edge_ID opp = get_opposite_real_edge(h.id());
    int n1 = get_index(h), n2 = get_index(opp);
    return _fd_1[std::min(n1, n2)];
  }

  void init_face_angle() {
    _fd_1.clear();
    int n = 4 * size_of_faces() + size_of_border_edges();
    _fd_1.resize(n, HUGE_VALF);
  }

  const float &get_angle_defect(const Node &v) const {
    return _ad_0[get_index(v)];
  }
  float &get_angle_defect(const Node &v) { return _ad_0[get_index(v)]; }
  void init_angle_defect() {
    _ad_0.clear();
    _ad_0.resize(size_of_nodes(), HUGE_VALF);
  }

  const float &get_cos_edge_angle(const Node &v) const {
    return _ea_0[get_index(v)];
  }
  float &get_cos_edge_angle(const Node &v) { return _ea_0[get_index(v)]; }
  void reset_cos_edge_angle(const Node &v) { _ea_0[get_index(v)] = HUGE_VALF; }
  void init_edge_angle() {
    _ea_0.clear();
    _ea_0.resize(size_of_nodes(), HUGE_VALF);
  }

  void set_strong_edge(const HEdge &h) { _is_f_1[get_index(h)] = true; }
  void unset_strong_edge(const HEdge &h) { _is_f_1[get_index(h)] = false; }
  void set_feature_0(const Node &v) { _is_f_0[get_index(v)] = true; }
  void unset_feature_0(const Node &v) { _is_f_0[get_index(v)] = false; }
  void set_on_feature(const Node &v) { _is_on_f[get_index(v)] = true; }
  void unset_on_feature(const Node &v) { _is_on_f[get_index(v)] = false; }

  //=========== Functions for supporting the overlay algorithm
  void create_overlay_data();
  void delete_overlay_data();

  void construct_bvpair2edge();
  int get_border_index(const HEdge &h) const {
    assert(h.id().is_border());
    return h.id().eid() - 1;
  }

 protected:
  // Data member
  RFC_Window_overlay *_window;  // Point to parent window.

  std::map<std::pair<int, int>, HEdge> _bv2edges;
  std::vector<HEdge> _cntrs_hedge;  // Counterparts
  std::vector<Node> _primnodes;     // Primaries of border vertices

  // Extra dataitems for the normals of the vertices.
  std::vector<Vector_3> _nrmls;  // Normals.

  std::vector<Vector_3> _f_nrmls;  // Normals for vertices on 1-features
                                   // Each entry coorresponds to a halfedge
                                   // on 1-features, storing the
                                   // normal/binormal of its destination.
  std::vector<int> _f_n_index;     // Indices for the halfedges in _f_nrmls.

  std::vector<Vector_3> _f_tngts;
  std::vector<int> _f_t_index;  // Map fron halfedges to indices of _f_b_index

  // Extra data members for features
  std::vector<float> _ad_0;  // Angle defect at vertices
  std::vector<float> _ea_0;  // Edge angle at vertices
  std::vector<float> _fd_1;  // Face angle at edges

  std::vector<bool> _is_f_0;
  std::vector<bool> _is_on_f;
  std::vector<bool> _is_f_1;

  // Data for supporting overlay algorithm.
  std::vector<INode *> _v_nodes;         // INodes at vertices
  std::vector<INode *> _e_node_buf;      // INode buffer for edges
  std::vector<INode_list> _e_node_list;  // INodes on edges
  std::vector<int> _e_marks;             // Marks for edges

  int _size_of_subfaces;
  std::vector<const INode *> _subnodes;
};

/// A window is a collection of panes.
class RFC_Window_overlay : public RFC_Window_derived<RFC_Pane_overlay>,
                           protected SURF::Window_manifold_2 {
 public:
  typedef RFC_Window_overlay Self;
  typedef RFC_Window_derived<RFC_Pane_overlay> Base;
  typedef MAP::Simple_manifold_2::Edge_ID Edge_ID;

  using RFC_Window_base::size_of_faces;
  using RFC_Window_base::size_of_nodes;

  RFC_Window_overlay(COM::Window *b, int color, const char *pre = NULL);
  virtual ~RFC_Window_overlay();

  /** @defgroup pio Public Interface for Overlay
   * @{
   */
  bool is_same_node(const Node &v1, const Node &v2);

  void create_overlay_data();
  void delete_overlay_data();

  void determine_counterparts();
  void unmark_alledges();

  HEdge get_an_unmarked_halfedge() const;
  /** @} end of pio */

  /** @defgroup ntc Normal and Tangent Computation
   * @{
   */
 public:
  void evaluate_normals();

  void print_features();

 protected:
  void reduce_normals_to_all(MPI_Op);
  virtual SURF::Pane_manifold_2 *init_pane_manifold(int pane_id) {
    return &pane(pane_id);
  }

  Vector_3 get_tangent(const HEdge &h) {
    return h.destination_l().point() - h.origin_l().point();
  }
  /** @} end of ntc */

 public:
  /** @defgroup pifd Public Interface for Feature Detection
   * @{
   */
  class Feature_0 {
   public:
    Feature_0() {}
    explicit Feature_0(const Node &v) : _v(v) {}
    const Point_3 &point() const { return _v.point(); }

    Node &vertex() { return _v; }
    const Node &vertex() const { return _v; }

    bool operator<(const Feature_0 &f) const {
      return _v.point() < f._v.point();
    }

   private:
    Node _v;
  };

  struct Feature_1 : public std::list<HEdge> {
    typedef std::list<HEdge> Base;
    Feature_1() {}
    template <class Iter>
    Feature_1(Iter it1, Iter it2) : Base(it1, it2) {}
    Bbox_3 bbox;
  };
  typedef std::list<Feature_0> Feature_list_0;
  typedef std::list<Feature_1> Feature_list_1;

  void detect_features();
  bool snap_on_features() const { return _snap_on_features; }
  Feature_list_0 &flist_0() { return _f_list_0; }
  Feature_list_1 &flist_1() { return _f_list_1; }
  const Feature_list_0 &flist_0() const { return _f_list_0; }
  const Feature_list_1 &flist_1() const { return _f_list_1; }
  /** @} end of pifd */

  /** @addtogroup fdp Feature-Detection Primitives
   * @{
   */
 protected:
  float comp_angle_defect(const Node &v);

  float cos_face_angle(const HEdge &h, const HEdge &hopp);
  float cos_face_angle(const HEdge &h);

  float cos_edge_angle(const HEdge &h1, const HEdge &h2);
  float cos_edge_angle(const Feature_1 &f1, Feature_1::const_iterator it,
                       bool isloop);
  bool is_strong_ad(const Node &v);
  bool is_rstrong_ea(const Feature_1 &f1, Feature_1::const_iterator hprev,
                     Feature_1::const_iterator hnext, float cos_ea,
                     bool isloop);
  /** @} end of fdp */

  /** @addtogroup mf1 Manipulation of Strong Curves
   * @{
   */
 protected:
  bool check_false_strong_1(Feature_1 &);
  void subdiv_feature_curve(const Feature_1 &f1, Feature_list_1 &new_flist,
                            int &dropped);
  void merge_features_1(const Node &v, Feature_1 &f1, Feature_1 &f2);

 public:
  void remove_feature_1(Feature_1 &f);
  /** @} end of mf1 */

  /** @addtogroup mf0 Manipulation of Strong Vertices
   * @{
   */
 protected:
  void identify_features_0();

 public:
  Feature_list_0::iterator remove_feature_0(Feature_list_0::iterator i);
  /** @} end of mf0 */

  /** @addtogroup fdh Feature-Detection Helpers
   * @{
   */
 private:
  void init_feature_parameters();
  void set_feature_0(const Node &v) const {
    assert(v.is_primary());
    v.pane()->set_feature_0(v);
  }
  void set_on_feature(const Node &v) const {
    assert(v.is_primary());
    v.pane()->set_on_feature(v);
  }

  bool is_feature_0(const Node &v) const {
    assert(v.is_primary());
    return v.pane()->is_feature_0(v);
  }
  bool is_on_feature(const Node &v) const {
    assert(v.is_primary());
    return v.pane()->is_on_feature(v);
  }
  bool is_feature_1(const HEdge &h) const { return h.pane()->is_feature_1(h); }
  void unset_strong_edge(const HEdge &h) { h.pane()->unset_strong_edge(h); }
  /** @} end of fdh */

  /** @addtogroup io Output Routines
   * @{
   */
  void dump_strong_edges(const std::vector<std::pair<float, HEdge> > &,
                         const std::vector<std::pair<float, HEdge> > &);

  /** @} end of io */

  /** Miscellaneous Helpers */
  void normalize(Vector_3 &v) {
    if (v != Vector_3(0, 0, 0)) v = v / std::sqrt(v * v);
  }

 private:
  float _cos_uf, _cos_lf, _rf, _cos_weakend;
  float _ud, _ld, _rd;
  float _cos_ue, _cos_le, _re;
  int _min_1f_len;
  int verb;

  std::string out_pre;
  Feature_list_0 _f_list_0;
  Feature_list_1 _f_list_1;
  std::map<Node, int> _f0_ranks;
  bool _long_falseness_check;
  bool _strong_ended;
  // Whether to snap blue features onto green features
  bool _snap_on_features;

  static const float r2d;
};

HEdge Node::halfedge_l() const {
  Edge_ID eid = _pm->get_incident_real_edge(_vID);
  if (!eid.is_border())
    return HEdge(_pm, _pm->get_opposite_real_edge(eid));
  else
    return HEdge(_pm, _pm->get_prev_real_edge(eid));
}

HEdge Node::halfedge_g() const {
  Edge_ID eID = _pm->get_incident_real_edge(_vID);

  // the current node (vertex) is in general the origin of the edge or
  // the edge center, unless the node is isolated.
  if (!eID.is_border()) return HEdge(_pm, _pm->get_opposite_real_edge(eID));

  HEdge h(_pm, _pm->get_opposite_real_edge(eID)), h0 = h;
  do {
    if (h.is_border_l()) break;
  } while ((h = h.next_g().opposite_g()) != h0);

  assert(h.destination_g() == get_primary());
  return h;
}

/// Obtain the coordinates of a node
const Point_3 &Node::point() const { return _pm->get_point(_vID); }

bool Node::is_border_l() const { return halfedge_l().is_border_l(); }

bool Node::is_border_g() const { return halfedge_g().is_border_g(); }

bool Node::is_isolated() const { return _pm->is_isolated_node(_vID); }

bool Node::is_primary() const { return _pm->is_primary(*this); }

Node Node::get_primary() const { return _pm->get_primary(*this); }

HEdge HEdge::opposite_l() const {
  return HEdge(_pm, _pm->get_opposite_real_edge(_eID));
}

HEdge HEdge::opposite_g() const { return _pm->get_opposite_edge(_eID); }

HEdge HEdge::prev_l() const {
  return HEdge(_pm, _pm->get_prev_real_edge(_eID));
}

HEdge HEdge::prev_g() const { return _pm->get_prev_edge(_eID); }

HEdge HEdge::next_l() const {
  return HEdge(_pm, _pm->get_next_real_edge(_eID));
}

HEdge HEdge::next_g() const { return _pm->get_next_edge(_eID); }

Node HEdge::origin_g() const {
  Node n(_pm, _pm->get_origin(_eID));
  return _pm->get_primary(n);
}

Node HEdge::origin_l() const { return Node(_pm, _pm->get_origin(_eID)); }

Node HEdge::destination_g() const {
  Node n(_pm, _pm->get_destination(_eID));
  return _pm->get_primary(n);
}

Node HEdge::destination_l() const {
  return Node(_pm, _pm->get_destination(_eID));
}

Face HEdge::face() const {
  if (_eID.is_border()) {
    return Face();
  } else {
    return Face(_pm, _eID.eid());
  }
}

bool HEdge::is_border_l() const { return _eID.is_border(); }

bool HEdge::is_border_g() const { return _pm->is_physical_border_edge(_eID); }

bool Node::operator<(const Node &v) const {
  int pid1 = _pm->pane()->id(), pid2 = v._pm->pane()->id();
  return pid1 < pid2 || (pid1 == pid2 && _vID < v._vID);
}

bool HEdge::operator<(const HEdge &h) const {
  int pid1 = _pm->pane()->id(), pid2 = h._pm->pane()->id();
  return pid1 < pid2 || (pid1 == pid2 && _eID < h._eID);
}

bool Face::operator<(const Face &f) const {
  int pid1 = _pm->pane()->id(), pid2 = f._pm->pane()->id();
  return pid1 < pid2 || (pid1 == pid2 && _fID < f._fID);
}

RFC_END_NAME_SPACE

#endif
