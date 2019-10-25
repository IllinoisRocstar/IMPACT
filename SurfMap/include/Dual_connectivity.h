//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef __DUAL_CONNECTIVITY_H
#define __DUAL_CONNECTIVITY_H

#include "com_devel.hpp"
#include "mapbasic.h"

MAP_BEGIN_NAMESPACE

/// Constructs the dual connectivity for the whole pane (including ghost
/// nodes and elements), which contains information about
/// incident elements for each node.
class Pane_dual_connectivity {
 public:
  /// Constructs the dual connectivity for a given pane.
  explicit Pane_dual_connectivity(const COM::Pane *p, bool with_ghost = true);

  /// Obtain the IDs of the elements incident on a given node
  void incident_elements(int node_id, std::vector<int> &elists);

 protected:
  /// Construct dual connectivity for 2-D structured meshes
  void construct_connectivity_str_2();
  /// Construct dual connectivity for unstructured meshes
  void construct_connectivity_unstr();

 private:
  const COM::Pane &_pane;     // Pane object
  bool _with_ghost;           // Whether to include ghost nodes/elements
  std::vector<int> _offsets;  // The offsets in _eids for each node
  std::vector<int> _eids;     // The incident element ids for all nodes
};

MAP_END_NAMESPACE

#endif /* __DUAL_CONNECTIVITY_H */
