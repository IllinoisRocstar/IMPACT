//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

//===================================================================
// This file contains the prototypes for transfering data from nodes
// to nodes and from faces to nodes.
//
//  Author: Xiangmin Jiao
//===================================================================

#ifndef __TRANSFER_2N_H_
#define __TRANSFER_2N_H_

#include "rfc_basic.h"
#include "Transfer_base.h"

RFC_BEGIN_NAME_SPACE

//! Specialization for transfering from nodes to nodes.
class Transfer_n2n : public Transfer_base {
  typedef Transfer_n2n            Self;
  typedef Transfer_base           Base;

public:
  //! Constructor.
  Transfer_n2n( RFC_Window_transfer *s, RFC_Window_transfer *t) 
    : Base(s, t) {}

  /** The main entry to the data transfer algorithm.
   *  \param sf    Souce data
   *  \param tf    Target data
   *  \param alpha Parameter to control interpolation of 
   *               coordinates between the input meshes
   *  \param tol   Tolerance of iterative solver
   *  \param iter  Number of iterations of iterative solver.
   *  \param doa   Degree of accuracy for quadrature rule to be used
   *  \param verb  Verbose level
   */
  void transfer( const Nodal_data_const &sf, Nodal_data &tf,
		 const Real alpha, Real *t, int *iter, int doa, bool ver);

  /** Compute the nodal load vector
   *  \param sf    Souce data
   *  \param tf    Target data
   *  \param alpha Parameter to control interpolation of 
   *               coordinates between the input meshes
   *  \param doa   Degree of accuracy for quadrature rule to be used
   *  \param verb  Verbose level
   */
  void comp_loads( const Nodal_data_const &sf, Nodal_data &tf,
		   const Real alpha, const int doa, bool verb);
};

//! Specialization for transfering from faces to nodes.
class Transfer_f2n : public Transfer_base {
  typedef Transfer_f2n                Self;
  typedef Transfer_base               Base;

 public:
  //! Constructor.
  Transfer_f2n( RFC_Window_transfer *s, RFC_Window_transfer *t)
    : Base(s, t) {}

  /** The main entry to the data transfer algorithm.
   *  \see Transfer_n2n::transfer
   */
  void transfer( const Facial_data_const &sf, Nodal_data &tf,
		 const Real alpha, Real *tol, int *iter, int doa, 
		 bool verb);

  /** Compute the nodal load vector
   *  \see Transfer_n2n::comp_loads
   */
  void comp_loads( const Facial_data_const &sf, Nodal_data &tf, 
		   const Real alpha, const int order, bool verb);
};

//! Specialization for transfering nodal forces using Farhat's algorithm.
class Interpolator : public Transfer_base {
  typedef Interpolator                  Self;
  typedef Transfer_base                 Base;

public:
  //! Constructor.
  Interpolator( RFC_Window_transfer *s, RFC_Window_transfer *t) 
    : Base(s, t) {}

  /** The main entry to the data transfer algorithm.
   *  \param sf    Souce data
   *  \param tf    Target data
   *  \param verb  Verbose level
   */
  void transfer( const Nodal_data_const &sf, Nodal_data &tf, bool verb);
};

RFC_END_NAME_SPACE

#endif // __TRANSFER_2N_H_


