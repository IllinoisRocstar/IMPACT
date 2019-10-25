//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** \file Pane_boundary.h
 * Utility for detecting boundaries of a pane.
 */

#ifndef _PANE_BOUNDARIES_H_
#define _PANE_BOUNDARIES_H_

#include <vector>
#include "Simple_manifold_2.h"
#include "com_devel.hpp"
#include "mapbasic.h"

MAP_BEGIN_NAMESPACE

class Pane_boundary {
 public:
  typedef int Node_ID;
  typedef int Element_ID;
  typedef std::pair<Node_ID, Node_ID> Node_pair;

  /// Constructors
  Pane_boundary(const COM::Pane *p) : _pane(*p), _pm(NULL) {}

  /// Constructors
  Pane_boundary(const Simple_manifold_2 *pm) : _pane(*pm->pane()), _pm(pm) {}

  /// Determine the border nodes (excluding isolated nodes)
  void determine_border_nodes(std::vector<bool> &is_border,
                              std::vector<bool> &is_isolated,
                              std::vector<Facet_ID> *b = NULL,
                              int ghost_level = 0);

  /// Compute the minimum squared edge length of given edges.
  double min_squared_edge_len(const std::vector<Facet_ID> &);

  /** Determine the nodes at pane boundaries of a given mesh.
   *  The argument isborder must be a nodal dataitem of integer type.
   *  At return, isborder is set to 1 for border nodes, and 0 for others */
  static void determine_borders(const COM::DataItem *mesh,
                                COM::DataItem *isborder, int ghost_level = 0);

 protected:
  /// Determine the border nodes for a 3-D mesh.
  /// A ghost level of 0 returns all real border nodes
  /// A ghost level of 1,2,3 returns the border nodes for
  /// the 1st, 2nd, 3rd... ghost layer (structured meshes)
  /// A ghost level > 0 returns the border ghost nodes
  /// (unstructured meshes)
  void determine_border_nodes_3(std::vector<bool> &is_border,
                                std::vector<Facet_ID> *b, int ghost_level);

  /// Determine the isolated nodes (i.e. not belonging to any element)
  void determine_isolated_nodes(std::vector<bool> &is_isolated,
                                int ghost_level);

 private:
  const COM::Pane &_pane;
  const Simple_manifold_2 *const _pm;  // Manifold for determing border nodes
};

MAP_END_NAMESPACE

#endif /* _PANE_BOUNDARIES_H_ */
