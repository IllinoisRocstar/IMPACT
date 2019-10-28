//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** \file Rocmap.h
 */
#ifndef __ROCMAP_H_
#define __ROCMAP_H_

#include "com_devel.hpp"
#include "mapbasic.h"

MAP_BEGIN_NAMESPACE

class Rocmap {
 public:
  Rocmap() {}

  /// Loads Rocmap onto Roccom with a given module name.
  static void load(const std::string &mname);
  /// Unloads Rocmap from Roccom.
  static void unload(const std::string &mname);

  /** Compute pane connectivity map between shared nodes.
   *  If pconn was not yet initialized, this routine will allocate memory
   *  for it. Otherwise, this routine will copy up to
   *  the capacity of the array */
  static void compute_pconn(const COM::DataItem *mesh, COM::DataItem *pconn);

  /** Determine the nodes at pane boundaries of a given mesh.
   *  The argument isborder must be a nodal attribute of integer type.
   *  At return, isborder is set to 1 for border nodes, and 0 for others */
  static void pane_border_nodes(const COM::DataItem *mesh,
                                COM::DataItem *isborder,
                                int *ghost_level = NULL);

  /// Get the number of communicating panes.
  static void size_of_cpanes(const COM::DataItem *pconn, const int *pane_id,
                             int *npanes_total, int *npanes_ghost = NULL);

  /// Perform an average-reduction on the shared nodes for the given attribute.
  static void reduce_average_on_shared_nodes(COM::DataItem *att,
                                             COM::DataItem *pconn = NULL);

  /// Perform a maxabs-reduction on the shared nodes for the given attribute.
  static void reduce_maxabs_on_shared_nodes(COM::DataItem *att,
                                            COM::DataItem *pconn = NULL);

  /// Perform a minabs-reduction on the shared nodes for the given attribute.
  static void reduce_minabs_on_shared_nodes(COM::DataItem *att,
                                            COM::DataItem *pconn = NULL);

  /// Update ghost nodal or elemental values for the given attribute.
  static void update_ghosts(COM::DataItem *att,
                            const COM::DataItem *pconn = NULL);
};

MAP_END_NAMESPACE

#endif
