//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

//===================================================================
// This file contains the prototypes for transfering data from nodes
// to faces and from faces to faces.
//
//  Author: Xiangmin Jiao
//===================================================================

#ifndef __TRANSFER_2F_H_
#define __TRANSFER_2F_H_

#include "Transfer_base.h"
#include "rfc_basic.h"

RFC_BEGIN_NAME_SPACE

//! Specialization for transfering from nodes to faces.
class Transfer_n2f : public Transfer_base {
  typedef Transfer_n2f Self;
  typedef Transfer_base Base;

 public:
  //! Constructor.
  Transfer_n2f(RFC_Window_transfer *s, RFC_Window_transfer *t) : Base(s, t) {}

  /** The main entry to the data transfer algorithm.
   *  \param sf    Souce data
   *  \param tf    Target data
   *  \param alpha Parameter to control interpolation of
   *               coordinates between the input meshes
   *  \param doa   Degree of accuracy for quadrature rule to be used
   *  \param verb  Verbose level
   */
  void transfer(const Nodal_data_const &sv, Facial_data &tf, const Real alpha,
                int doa = 0, bool verb = false);
};

//! Specialization for transfering from faces to faces.
class Transfer_f2f : public Transfer_base {
  typedef Transfer_f2f Self;
  typedef Transfer_base Base;

 public:
  //! Constructor.
  Transfer_f2f(RFC_Window_transfer *s, RFC_Window_transfer *t) : Base(s, t) {}

  /** The main entry to the data transfer algorithm.
   *  \see Transfer_n2f::transfer
   */
  void transfer(const Facial_data_const &sf, Facial_data &tf, const Real alpha,
                int doa = 0, bool verb = false);
};

RFC_END_NAME_SPACE

#endif  // __TRANSFER_2F_H_
