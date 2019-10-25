//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef MESH_OVERLAY_H
#define MESH_OVERLAY_H

#include <iostream>
#include <list>
#include <queue>
#include <vector>
#include "HDS_accessor.h"
#include "Overlay_primitives.h"
#include "RFC_Window_overlay.h"

RFC_BEGIN_NAME_SPACE

// This class encapsulates the (simplified) overlay algorithm.
// The algorithm proceeds in four steps.
//    Step 1: Project blue vertices onto G.
//    Step 2: Compute intersection of blue edges with green
//       edges, and insert intersections into blue edges.
//    Step 3: Sort intersections on green edges.
//    Step 4: Project green vertices on B.
class Overlay {  // Updated: Feb. 25, 2001
 public:
  typedef Overlay Self;
  typedef std::pair<HEdge, HEdge> Parent_pair;
  typedef std::list<const INode *> INode_const_list;
  typedef std::vector<const INode *> Subface;
  typedef std::list<Subface> Subface_list;
  typedef RFC_Window_overlay::Feature_0 Feature_0;
  typedef RFC_Window_overlay::Feature_1 Feature_1;
  typedef RFC_Window_overlay::Feature_list_0 Feature_list_0;
  typedef RFC_Window_overlay::Feature_list_1 Feature_list_1;

 public:
  // Constructor
  Overlay(const COM::Window *w1, const COM::Window *w2, const char *pre);
  // Destructor
  ~Overlay();

  // This function is the main interface for the overlay algorithm.
  int overlay();

  // Set tolerance for snapping vertices.
  void set_tolerance(double tol);

  // Interfaces for the data transfer algorithms
  RFC_Window_overlay *get_rfc_window(const COM::Window *w) {
    return (B->base() == w) ? B : G;
  }
  const RFC_Window_overlay *get_rfc_window(const COM::Window *w) const {
    return (B->base() == w) ? B : G;
  }
  RFC_Window_overlay *get_blue_window() { return B; }
  RFC_Window_overlay *get_green_window() { return G; }
  const RFC_Window_overlay *get_blue_window() const { return B; }
  const RFC_Window_overlay *get_green_window() const { return G; }

  void get_inodes_of_face(const Face &f, INode_const_list &nodes);

  HEdge get_parent_face(const Subface &sf, int color);

  bool subdivide(const INode_const_list &face,
                 INode_const_list::const_iterator last_checked,
                 Subface_list &sub_faces, int color, int depth = 0) const;

  // Obtain the natural coordinates of an inode in the incident face of h.
  Point_2 get_nat_coor(const INode &i, const Generic_element &e, const HEdge &h,
                       int color) const;

  //=========== Subroutines for outputting the intermediate results
  //! Export the subdivisions to the two given windows.
  void export_windows(RFC_Window_base *, RFC_Window_base *);

  // Write out all the inodes in Tecplot format.
  void write_inodes_tec(std::ostream &os, const char *color);

  // Write out all the inodes in vect format for Geomview.
  //    rgbo [0..255] specifies drawing color.
  void write_inodes_vec(std::ostream &os);

  void write_overlay_tec(std::ostream &, const COM::Window *);

  void write_overlay_off(std::ostream &, const COM::Window *);

 protected:
  // ----------------- Functions for step 1 --------------------
  // The initialization for the overlay algorithm. It locates the
  //     green parent of a blue vertex and create an inode for it.
  //     This initialization step takes linear time.
  // Returns NULL if unsuccessful. Otherwise, creates a new INode.
  INode *overlay_init();
  // Helper for overlay_init which computes the parent of a point x
  void get_green_parent(const Node &v, const HEdge &b, HEdge *o, Parent_type *t,
                        Point_2 *nc);

  // This function ensures the consistency of the green parent of x.
  void insert_node_in_blue_edge(INode &x, const HEdge &b);

  // This subroutine is step 1 of the algorithm. It determines the
  //    associates of the vertices of the blue mesh in the green mesh.
  //    and computes the intersections of the blue edges with the green
  //    edges. These intersections   are stored in the lists associated
  //    with the blue edges. At input, x is an inode corresponding to
  //    a blue vertex.
  void intersect_blue_with_green();

  //  Sort the intersection points with respect to green edges and
  //       computes the projection of green vertices in B.
  void sort_on_green_edges();
  void associate_green_vertices();

  // The helper for sort_on_green_edges. It inserts a node v into the green
  // edge v->green_halfedge(). Tag is for marking the buckets. We assume
  // that each green(blue) edge intersects a blue(green) face at most twice.
  void insert_node_in_green_edge(INode *v, int tag);

  // Helper for associate_green_vertices, which determines the parent
  //   of g->destination() from the object containing (b0,t0).
  void project_adjacent_green_vertices(const INode *, const HEdge &);

  bool verify_inode(const INode *i);

  // Helpers for determine_edge_parents.
  Host_face get_edge_parent(const INode &i0, const INode &i1,
                            const int color) const;

  Parent_pair get_edge_pair_parents(const INode &i0, const INode &i1,
                                    const int color) const;

  //================= Other general helpers for the algorithm.
  // Computes the intersection of a halfedge b with
  //   the edges in Lk( g1,t1)-St( g0,t0).
  //   It takes O(n) time, where n is the number of incident
  //   edges of the green_parent of (g1,t1). It returns the intersection
  //   point on both b and g, and the parent of the intersection.
  //   This subroutine is used in associate_blue_vertices and
  //   intersect_blue_with_green.
  bool intersect_link(const INode *x, const HEdge &b, const HEdge &g0,
                      const Parent_type t0, HEdge &g1, const Parent_type t1,
                      Real start, Real *cb, Real *cg, HEdge *g, Parent_type *t,
                      int tryindex = 0);

  bool intersect_link_helper(const HEdge &b, const HEdge &g2, Real start,
                             Real *cb, Real *cg, HEdge *g, Parent_type *t,
                             bool *found, int snapcode, const Node &anchor,
                             bool panic);

  bool intersect_link_helper2(Real *cb, Real *cg, HEdge *g, Parent_type *t);

  // Checking whether an object <e1,t1> contains another object <e2,t2>.
  bool contains(const HEdge &e1, const Parent_type t1, const HEdge &e2,
                const Parent_type t2) const;

  // ===== Following subroutines are for matching 0-dimensional features
  void match_features_0();

  int count_edges(const HEdge &e) const {
    int i = 0;
    HEdge h = e;
    do {
      ++i;
    } while ((h = h.next_l()) != e);
    return i;
  }

 private:  // Data and functions for numering the subnodes and subfaces.
  void set_subnode_id(INode *i, int color, int pane_id, int l_id);
  int get_subnode_id(const INode *i, int color, int pane_id) const;
  int get_subnode_copies(const INode *i, int color) const;
  void convert_nat_coordinates(const INode *i, HEdge h, int color, int pane_id,
                               int &lid, RFC_Pane_overlay::ParentEdge_ID &eid,
                               Point_2 &nc) const;

  void number_a_subnode(INode *i, int color,
                        std::map<int, std::pair<int, int> > &cnts);
  void count_subnodes(INode *i, int color,
                      std::map<int, std::pair<int, int> > &cnts);
  void number_subnodes();
  void number_subfaces();

  std::vector<Node_ID> _subnode_ids_b;
  std::vector<Node_ID> _subnode_ids_g;
  std::vector<char> _subnode_copies_b;
  std::vector<char> _subnode_copies_g;

  std::map<int, std::map<int, int> > _subnode_imap_b;
  std::map<int, std::map<int, int> > _subnode_imap_g;

 private:
  void insert_edge_into_queue(const HEdge &h, INode *v, std::queue<HEdge> &q,
                              std::queue<HEdge> &q_rdg,
                              std::queue<HEdge> &q_crn);

  bool is_queue_empty(std::queue<HEdge> &q, std::queue<HEdge> &q_rdg,
                      std::queue<HEdge> &q_crn);

  // Evaluating the squared length of a halfedge. Verified: Jan. 4, 2001
  Real sq_length(const HEdge &h) const;

  const INode *get_next_inode(const INode *v1, const INode *v2, int) const;

  std::pair<const INode *, HEdge> get_next_inode_ccw(const INode *v0,
                                                     const INode *v1,
                                                     int color) const;

  std::pair<const INode *, HEdge> get_next_inode_cw(const INode *v0,
                                                    const INode *v1,
                                                    int color) const;

  bool logical_xor(bool a, bool b) const { return (a && !b) || (!a && b); }

 protected:
  RFC_Window_overlay *B;      // input blue window.
  RFC_Window_overlay *G;      // input green window.
  std::list<INode *> inodes;  // Container for all the inode objects.
  Overlay_primitives op;
  HDS_accessor acc;

  bool is_opposite;
  bool verbose;
  bool verbose2;
  std::string out_pre;  // Output prefix

  Real eps_e;
  Real eps_p;
};

RFC_END_NAME_SPACE

#endif  // MESH_OVERLAY_H
