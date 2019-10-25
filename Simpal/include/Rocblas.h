//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** \file Rocblas.h
 *  Definition for Rocblas API.
 */
#include <cstdio>
#include "com.h"

USE_COM_NAME_SPACE

class Rocblas {
  typedef unsigned int Size;

 public:
  /// Creates window for Rocblas and registers functions.
  static void init(const std::string &name);

  /// Delete window for Rocblas.
  static void finalize(const std::string &name);

  /// Operation wrapper for addition.
  static void add(const DataItem *x, const DataItem *y, DataItem *z);

  /// Operation wrapper for subtraction.
  static void sub(const DataItem *x, const DataItem *y, DataItem *z);

  /// Operation wrapper for multiplication.
  static void mul(const DataItem *x, const DataItem *y, DataItem *z);

  /// Operation wrapper for limit1.
  static void limit1(const DataItem *x, const DataItem *y, DataItem *z);

  /// Operation wrapper for division.
  static void div(const DataItem *x, const DataItem *y, DataItem *z);

  /// Operation wrapper for addition with y as a scalar pointer.
  static void add_scalar(const DataItem *x, const void *y, DataItem *z,
                         int swap = 0);

  /// Operation wrapper for subtraction with y as a scalar pointer.
  static void sub_scalar(const DataItem *x, const void *y, DataItem *z,
                         int swap = 0);

  /// Operation wrapper for multiplication with y as a scalar pointer.
  static void mul_scalar(const DataItem *x, const void *y, DataItem *z,
                         int swap = 0);

  /// Operation wrapper for division with y as a scalar pointer.
  static void div_scalar(const DataItem *x, const void *y, DataItem *z,
                         int swap = 0);

  /// Operation wrapper for addition with y as a scalar pointer.
  static void maxof_scalar(const DataItem *x, const void *y, DataItem *z,
                           int swap = 0);

  /// Wrapper for dot product.
  static void dot(const DataItem *x, const DataItem *y, DataItem *z,
                  const DataItem *mults = NULL);

  /// Wrapper for 2-norm with z as a scalar pointer.
  static void dot_scalar(const DataItem *x, const DataItem *y, void *z,
                         const DataItem *mults = NULL);

  /// Wrapper for dot product.
  static void dot_MPI(const DataItem *x, const DataItem *y, DataItem *z,
                      const MPI_Comm *comm = NULL,
                      const DataItem *mults = NULL);

  /// Wrapper for 2-norm with z as a scalar pointer.
  static void dot_scalar_MPI(const DataItem *x, const DataItem *y, void *z,
                             const MPI_Comm *comm = NULL,
                             const DataItem *mults = NULL);

  /// Wrapper for 2-norm.
  static void nrm2(const DataItem *x, DataItem *y,
                   const DataItem *mults = NULL);

  /// Wrapper for 2-norm with y as a scalar pointer.
  static void nrm2_scalar(const DataItem *x, void *y,
                          const DataItem *mults = NULL);

  /// Wrapper for 2-norm with MPI.
  static void nrm2_MPI(const DataItem *x, DataItem *y,
                       const MPI_Comm *comm = NULL,
                       const DataItem *mults = NULL);

  /// Wrapper for 2-norm with y as a scalar pointer with MPI.
  static void nrm2_scalar_MPI(const DataItem *x, void *y, const MPI_Comm *comm,
                              const DataItem *mults = NULL);

  /// Wrapper for swap.
  static void swap(DataItem *x, DataItem *y);

  /// Wrapper for copy.
  static void copy(const DataItem *x, DataItem *y);

  /// Generate a random number between 0 and $a$ for each entry in z
  static void rand(const DataItem *a, DataItem *z);

  /// Operation wrapper for copy  (x is a scalar pointer).
  static void copy_scalar(const void *x, DataItem *y);

  /// Generate a random number between 0 and $a$ for each entry in z
  static void rand_scalar(const void *a, DataItem *z);

  /// Wrapper for neg (y=-x).
  static void neg(const DataItem *x, DataItem *y);

  /// Wrapper for sqrt (y=sqrt(x)).
  static void sqrt(const DataItem *x, DataItem *y);

  /// Wrapper for acos (y=acos(x)).
  static void acos(const DataItem *x, DataItem *y);

  /// Wrapper for max.
  static void max_MPI(const DataItem *x, DataItem *y,
                      const MPI_Comm *comm = NULL);

  /// Operation wrapper for max (y is a scalar pointer).
  static void max_scalar_MPI(const DataItem *x, void *y,
                             const MPI_Comm *comm = NULL);

  /// Wrapper for min.
  static void min_MPI(const DataItem *x, DataItem *y,
                      const MPI_Comm *comm = NULL);

  /// Operation wrapper for min (y is a scalar pointer).
  static void min_scalar_MPI(const DataItem *x, void *y,
                             const MPI_Comm *comm = NULL);

  /// Wrapper for sum.
  static void sum_MPI(const DataItem *x, DataItem *y,
                      const MPI_Comm *comm = NULL);

  /// Operation wrapper for sum (y is a scalar pointer).
  static void sum_scalar_MPI(const DataItem *x, void *y,
                             const MPI_Comm *comm = NULL);

  /// Operation wrapper for z = a * x + y.
  static void axpy(const DataItem *a, const DataItem *x, const DataItem *y,
                   DataItem *z);

  /// Operation wrapper for z = a * x + y (a is a scalar pointer).
  static void axpy_scalar(const void *a, const DataItem *x, const DataItem *y,
                          DataItem *z);

 protected:
  ///  Performs the operation:  z = x op y
  template <class FuncType, int ytype>
  static void calc(DataItem *z, const DataItem *x, const void *yin,
                   FuncType opp, bool swap = false);

  ///  Performs the operation:  z = <x, y>
  template <class data_type, int ztype>
  static void calcDot(void *zout, const DataItem *x, const DataItem *y,
                      const MPI_Comm *comm = NULL,
                      const DataItem *mults = NULL);

  ///  Performs the operation  opp(x, y)
  template <class FuncType, int ytype>
  static void gen2arg(DataItem *z, void *yin, FuncType opp);

  ///  Performs the operation:  z = a*x + y
  template <class data_type, int atype>
  static void axpy_gen(const void *a, const DataItem *x, const DataItem *y,
                       DataItem *z);

  /// Chooses which calc function to call based on type of y.
  template <class FuncType>
  static void calcChoose(const DataItem *x, const DataItem *y, DataItem *z,
                         FuncType opp);

  template <class Op>
  static void copy_helper(const DataItem *x, DataItem *z);

  // Function object that implements an assignment.
  template <class T_src, class T_trg>
  struct assn;

  // Function object that implements a random number generator.
  template <class T>
  struct random;

  // Function object that implements a swap.
  template <class T>
  struct swapp;

  // Function object that implements a max operation.
  template <class T>
  struct maxv;

  // Function object that implements a min operation.
  template <class T>
  struct minv;

  // Function object that implements a sum operation.
  template <class T>
  struct sumv;

  // Function object that implements a negation.
  template <class T>
  struct nega;

  // Function object that implements sqrt.
  template <class T>
  struct sqrta;

  // Function object that implements acos.
  template <class T>
  struct acosa;

  // Function object that implements a limit1.
  template <class T>
  struct limit1v;

  // Function object that implements a maxof.
  template <class T>
  struct maxof;

  enum { BLAS_VOID, BLAS_SCALAR, BLAS_VEC, BLAS_SCNE, BLAS_VEC2D };

  template <int attr_type>
  inline static int get_stride(const DataItem *attr);

  template <class data_type, int attr_type, bool is_staggered>
  inline static data_type &getref(data_type *base, const int r, const int c,
                                  const int nc);

  template <class data_type, int attr_type, bool is_staggered>
  inline static const data_type &getref(const data_type *base, const int r,
                                        const int c, const int nc);

  template <class OPint, class OPdbl, int OPMPI>
  inline static void reduce_MPI(const DataItem *x, DataItem *z,
                                const MPI_Comm *comm, int, double);

  template <class OPint, class OPdbl, int OPMPI>
  inline static void reduce_scalar_MPI(const DataItem *x, void *y,
                                       const MPI_Comm *comm, int, double);

  static std::string to_str(int i) {
    char buf[10];
    std::sprintf(buf, "%d", i);
    return buf;
  }
};

template <int attr_type>
inline int Rocblas::get_stride(const DataItem *attr) {
  if (attr_type == BLAS_SCNE || attr_type == BLAS_VEC2D) {
    if (attr->size_of_items() > 1)
      return attr->stride();
    else
      return 0;
  } else if (attr_type == BLAS_VEC)
    return 0;
  else
    return 1;
}

template <class data_type, int attr_type, bool is_staggered>
inline data_type &Rocblas::getref(data_type *base, const int r, const int c,
                                  const int nc) {
  if (attr_type == BLAS_SCNE || (attr_type == BLAS_VEC2D && is_staggered))
    return base[r * nc];
  else if (attr_type == BLAS_VEC2D)
    return base[r * nc + c];
  else if (attr_type == BLAS_VEC)
    return base[c];
  else
    return *base;
}

template <class data_type, int attr_type, bool is_staggered>
inline const data_type &Rocblas::getref(const data_type *base, const int r,
                                        const int c, const int nc) {
  if (attr_type == BLAS_SCNE || (attr_type == BLAS_VEC2D && is_staggered))
    return base[r * nc];
  else if (attr_type == BLAS_VEC2D)
    return base[r * nc + c];
  else if (attr_type == BLAS_VEC)
    return base[c];
  else
    return *base;
}

/// Calls Rocblas initialization function.
extern "C" void Simpal_load_module(const char *name);
extern "C" void Simpal_unload_module(const char *name);
