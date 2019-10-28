//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef _KD_TREE_3_
#define _KD_TREE_3_

#include <cassert>

class KD_tree_3 {
 public:
  KD_tree_3() : _npnts(0), _maxout(0), _tree(0), _bboxes(0), _indices(0) {}

  KD_tree_3(const double *pnts, int np, int maxn = 0)
      : _npnts(0), _maxout(0), _tree(0), _bboxes(0), _indices(0) {
    build(pnts, np, maxn);
  }

  ~KD_tree_3() {
    if (_tree) {
      delete[] _tree;
      _tree = 0;
    }
    if (_bboxes) {
      delete[] _bboxes;
      _bboxes = 0;
    }
    if (_indices) {
      delete[] _indices;
      _indices = 0;
    }
  }

  /* Public interface for building kdtree.
   * Give points should have format equivalent to double[npnts][3].
   * It also takes an optional argument maxn to specify the maximum
   * number of points that can be returned by search.
   */
  void build(const double *pnts, int np, int maxn = 0) {
    assert(np >= 0);
    _npnts = np;
    if (maxn)
      _maxout = maxn;
    else
      _maxout = np;

    if (_npnts) {
      _tree = new int[2 * _npnts - 1];
      _bboxes = new double[12 * _npnts - 6];

      kdtree_build_3(pnts, _npnts);
      /* Compute bboxes in linear time */
      kdtree_bboxes_3(pnts);
    }
  }

  /* Public interface for range search.
   * range is specified as {minx, miny, minz, maxx, maxy, maxz}.
   * If indices is given as second argument, then point to buffer space
   * for indices to the points is returned.
   * This function must be called after build has been called.
   */
  int search(const double range[6], int **indices = 0, int start = 0) {
    assert(_tree);
    if (_npnts == 0) return 0;

    if (!_indices) _indices = new int[_maxout];
    if (indices) *indices = _indices;

    return kdtree_search_3(range, start);
  }

  /* Public interface for range search.
   * range is specified as {minx, miny, minz, maxx, maxy, maxz}.
   * If indices is given as second argument, then point to buffer space
   * for indices to the points is returned.
   * This function must be called after build has been called.
   */
  int search(const double pnt[3], const double tol, int **indices = 0,
             int start = 0) {
    double range[6];

    for (int k = 0; k < 3; ++k) {
      range[k] = pnt[k] - tol;
      range[3 + k] = pnt[k] + tol;
    }

    if (!_indices) _indices = new int[_maxout];
    if (indices) *indices = _indices;

    return kdtree_search_3(range, start);
  }

 protected:
  /*************************************************************
   *
   * FUNCTION: kdtree_build_3
   *
   * Build a kd-tree for a given set of points in 3-D.
   *************************************************************/
  void kdtree_build_3(const double *xs, int npoints);

  /*************************************************************
   *
   * FUNCTION: kdtree_bboxes_3
   *
   * Compute bboxes of nodes of kdtree in linear time.
   *
   * See also  kdtree_search_3, kdtree_build_3
   *************************************************************/
  void kdtree_bboxes_3(const double *xs);

  /*************************************************************
   *
   * FUNCTION: kdtree_search_3
   *
   * Search the k-D tree structure to find points contained whtinin a
   * given range. It returns the number if points found and saves
   * the indices of points into _indices.
   *
   * See also kdtree_build_3, kdtree_bboxes_3
   *************************************************************/
  int kdtree_search_3(const double range[6], int start);

 private:
  int _npnts, _maxout;
  int *_tree;
  double *_bboxes;
  int *_indices;
};

#endif
