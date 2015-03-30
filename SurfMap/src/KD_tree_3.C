//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

//==========================================================
// The implementation of KD_tree_3.
//
// Author: Xiangmin Jiao
// Last modified:   06/12/2010
//==========================================================

#include "KD_tree_3.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <climits>

using namespace std;

/*************************************************************
 *
 * FUNCTION: select_entry
 *
 * Reorders an array for a given integer k, so that the kth entry 
 *  partitions the smaller and larger values after reordering.
 *  It takes an integer k, a real array arr, and an integer array iprm of 
 *  length n with a starting index start, and reorders iprm so that 
 *  arr(iprm(k)) is the kth largest element in the set {arr(iprm(i)),
 *  i=start,..,n}, and in addition we have the partitioning properties:
 *     i<k ==> arr(iprm(i)) <= arr(iprm(k)) and
 *     i>k ==> arr(iprm(i)) >= arr(iprm(k)).
 *************************************************************/
static void  select_entry( const double *arr, int k, int **iprm, 
                           int start, int n, int n2) {
  int start_1, il, ir, itemp, mid;
  int i, j, ia;
  double a;
  
  /* Note: This routine is similar to the Numerical Recipes subroutine */
  /* ``select'', except Numerical Recipes do not have the permutation */
  /*  array IPRM; instead they reorder the array directly. */
  start_1 = start - 1;
  il = 1;
  ir = n;
  for (;;) {
    if (1 + il >= ir) {
      if ((1 + il == ir)&&( arr[ 3 *  (*iprm)[ start_1 - 1 + ir] - 4 + n2] <  
                            arr[ 3 *  (*iprm)[ start_1 - 1 + il] - 4 + n2])) {
        itemp = (*iprm)[ start_1 - 1 + il];
        (*iprm)[ start_1 - 1 + il] = (*iprm)[ start_1 - 1 + ir];
        (*iprm)[ start_1 - 1 + ir] = itemp;
      }
      return;
    }
    else {
      mid = (il + ir) >> 1;
      itemp = (*iprm)[ start_1 - 1 + mid];
      (*iprm)[ start_1 - 1 + mid] = (*iprm)[start_1 + il];
      (*iprm)[start_1 + il] = itemp;
      if ( arr[ 3 *  (*iprm)[ start_1 - 1 + il] - 4 + n2] >  
           arr[ 3 *  (*iprm)[ start_1 - 1 + ir] - 4 + n2]) {
        itemp = (*iprm)[ start_1 - 1 + il];
        (*iprm)[ start_1 - 1 + il] = (*iprm)[ start_1 - 1 + ir];
        (*iprm)[ start_1 - 1 + ir] = itemp;
      }
      if ( arr[ 3 *  (*iprm)[start_1 + il] - 4 + n2] >  
           arr[ 3 *  (*iprm)[ start_1 - 1 + ir] - 4 + n2]) {
        itemp = (*iprm)[start_1 + il];
        (*iprm)[start_1 + il] = (*iprm)[ start_1 - 1 + ir];
        (*iprm)[ start_1 - 1 + ir] = itemp;
      }
      if ( arr[ 3 *  (*iprm)[ start_1 - 1 + il] - 4 + n2] >  
           arr[ 3 *  (*iprm)[start_1 + il] - 4 + n2]) {
        itemp = (*iprm)[start_1 + il];
        (*iprm)[start_1 + il] = (*iprm)[ start_1 - 1 + il];
        (*iprm)[ start_1 - 1 + il] = itemp;
      }
      i = 1 + il;
      j = ir;
      ia = (*iprm)[start_1 + il];
      a = arr[ n2 - 4 + 3 * ia];
      for (;;) {
        i = 1 + i;
        while ( arr[ 3 *  (*iprm)[ start_1 - 1 + i] - 4 + n2] < a) {
          i = 1 + i;
        }
        j = j - 1;
        while ( arr[ 3 *  (*iprm)[ start_1 - 1 + j] - 4 + n2] > a) {
          j = j - 1;
        }
        if (j < i) {
          break;
        }
        itemp = (*iprm)[ start_1 - 1 + i];
        (*iprm)[ start_1 - 1 + i] = (*iprm)[ start_1 - 1 + j];
        (*iprm)[ start_1 - 1 + j] = itemp;
      }
      (*iprm)[start_1 + il] = (*iprm)[ start_1 - 1 + j];
      (*iprm)[ start_1 - 1 + j] = ia;
      if (j >= k) {
        ir = j - 1;
      }
      if (j <= k) {
        il = i;
      }
    }
  }
}

void  KD_tree_3::kdtree_build_3( const double *xs, int npoints) {
  int *points;
  int nextt, itop, node, imn;
  /* size of the stack is bounded by the depth of tree. So 32 is plenty. */
  int imin[32], imax[32], ict[32], istack[32];
  double dimx, dimy, dimz;
  int imx, icut, imd, v;
  int  i;

  /*.... If the number of points is not positive, give an error. */
  assert(npoints > 0);

  /*.... Let the bounding box associated with the root node */
  /*.... [bboxes(1,:)] exactly contain all points. */
  /*.... We use this bounding box to help choose the cutting direction. */
    
  _bboxes[0] = HUGE_VAL;
  _bboxes[1] = HUGE_VAL;
  _bboxes[2] = HUGE_VAL;
  _bboxes[3] = -HUGE_VAL;
  _bboxes[4] = -HUGE_VAL;
  _bboxes[5] = -HUGE_VAL;
  for (i=0; i<npoints; i++) {
    _bboxes[0] = min(xs[3 * i], _bboxes[0]);
    _bboxes[1] = min(xs[1 + 3 * i], _bboxes[1]);
    _bboxes[2] = min(xs[2 + 3 * i], _bboxes[2]);
    _bboxes[3] = max(xs[3 * i], _bboxes[3]);
    _bboxes[4] = max(xs[1 + 3 * i], _bboxes[4]);
    _bboxes[5] = max(xs[2 + 3 * i], _bboxes[5]);
  }

  /*.... If there is only one point, the root node is a leaf. */
  /*.... (Our convention is to set the link corresponding to a leaf */
  /*.... equal to the negative of the unique point contained in */
  /*.... that leaf.)  If the root is a leaf, our work is done. */
  
  if (npoints == 1) {
    _tree[0] = -1;
    return;
  }
  /*.... DIMX, DIMY, DIMZ are equal to the x, y, and z dimensions */
  /*.... of the bounding box corresponding to the current node */
  /*.... under consideration. */
    
  dimx =  _bboxes[3] -  _bboxes[0];
  dimy =  _bboxes[4] -  _bboxes[1];
  dimz =  _bboxes[5] -  _bboxes[2];
  /*.... points will contain a permutation of the integers */
  /*.... {1,...,npoints}.  This permutation will be altered as we */
  /*.... create our balanced binary tree.  NEXTT contains the */
  /*.... address of the next available node to be filled. */
    
  nextt = 2;
  points = new int[npoints];
  for (i=0; i<npoints; i++) {
    points[i] = 1 + i;
  }
  /*.... Use a stack to create the tree.  Put the root node */
  /*.... "1" on the top of the stack (ICRSTACK).  The array */
  /*.... subset of points (i.e., subset of points associated */
  /*.... with this node) is recorded using the arrays */
  /*.... IMIN and IMAX. */
  
  itop = 1;
  istack[0] = 1;
  imin[0] = 1;
  imax[0] = npoints;
  /*.... Finally, we record in ICT the appropriate "cutting */
  /*.... direction" for bisecting the set of points */
  /*.... corresponding to this node.  This direction is either */
  /*.... the x, y, or z direction, depending on which dimension */
  /*.... of the bounding box is largest. */
  
  if (max(dimz, dimy) <= dimx) {
    ict[0] = 1;
  }
  else if (dimy >= dimz) {
    ict[0] = 2;
  }
  else {
    ict[0] = 3;
  }
  /*.... Pop nodes off stack, create children nodes and put them */
  /*.... on stack.  Continue until k-D tree has been created. */

  while (itop > 0) {
    /*.Pop top node off stack. */
    
    node = istack[itop - 1];
    /*.Make this node point to next available node location (NEXTT). */
    /*.This link represents the location of the FIRST CHILD */
    /*.of the node.  The adjacent location (NEXTT+1) */
    /*.is implicitly taken to be the location of the SECOND */
    /*.child of the node. */
    
    _tree[node - 1] = nextt;
    imn = imin[itop - 1];
    imx = imax[itop - 1];
    icut = ict[itop - 1];
    itop = itop - 1;
    /*.Partition point subset associated with this node. */
    /*.Using the appropriate cutting direction, use SELECT_ENTRY to */
    /*.reorder points so that the point with median */
    /*.coordinate is points(imd), while the */
    /*.points {points(i),i<imd} have SMALLER (or equal) */
    /*.coordinates, and the points with */
    /*.{points(i),i>imd} have GREATER coordinates. */
    
    imd = (imn + imx) >> 1;
    select_entry(xs, 1 - imn + imd, &points, imn, 1 - imn + imx, icut);
    /*.If the first child's subset of points is a singleton, */
    /*.the child is a leaf.  Set the child's link to point to the */
    /*.negative of the point number.  Set the child's bounding */
    /*.box to be equal to a box with zero volume located at */
    /*.the coordinates of the point. */

    if (imn == imd) {
      v = points[imn - 1];
      _tree[nextt - 1] = -v;
      nextt = 1 + nextt;
    }
    else {
      /*.In this case, the subset of points corresponding to the */
      /*.first child is more than one point, and the child is */
      /*.not a leaf.  Approximate the bounding box of this child. */

      for (i=0; i<=5; i++) {
        _bboxes[ 6 * nextt - 6 + i] = _bboxes[ 6 * node - 6 + i];
      }
      _bboxes[ 6 * nextt - 4 + icut] = xs[ 3 *  points[imd - 1] - 4 + icut];
      /*...Put the first child onto the stack, noting the */
      /*...associated point subset in IMIN and IMAX, and */
      /*...putting the appropriate cutting direction in ICT. */

      dimx =  _bboxes[ 6 * nextt - 3] -  _bboxes[ 6 * nextt - 6];
      dimy =  _bboxes[ 6 * nextt - 2] -  _bboxes[ 6 * nextt - 5];
      dimz =  _bboxes[ 6 * nextt - 1] -  _bboxes[ 6 * nextt - 4];
      itop = 1 + itop;
      assert (itop <= 32);

      istack[itop - 1] = nextt;
      imin[itop - 1] = imn;
      imax[itop - 1] = imd;
      if ((dimx >= dimy)&&(dimx >= dimz)) {
        ict[itop - 1] = 1;
      }
      else if (dimy >= dimz) {
        ict[itop - 1] = 2;
      }
      else {
        ict[itop - 1] = 3;
      }
      nextt = 1 + nextt;
    }
    /*.If the first child's subset of points is a singleton, */
    /*.the child is a leaf.  Set the child's link to point to the */
    /*.negative of the point number.  Set the child's bounding */
    /*.box to be equal to a box with zero volume located at */
    /*.the coordinates of the point. */

    if (!(1 - imx + imd)) {
      v = points[imx - 1];
      _tree[nextt - 1] = -v;
      nextt = 1 + nextt;
    }
    else {
      /*.. In this case, the subset of points corresponding to the */
      /*...second child is more than one point, and the child is */
      /*...not a leaf.  Approximate the bounding box of this child.         */

      for (i=0; i<=5; i++) {
        _bboxes[ 6 * nextt - 6 + i] = _bboxes[ 6 * node - 6 + i];
      }
      _bboxes[ 6 * nextt - 7 + icut] = xs[ 3 *  points[imd] - 4 + icut];
      /*.. Put the second child onto the stack, noting the */
      /*...associated point subset in IMIN and IMAX, and */
      /*...putting the appropriate cutting direction in ICT. */

      dimx =  _bboxes[ 6 * nextt - 3] -  _bboxes[ 6 * nextt - 6];
      dimy =  _bboxes[ 6 * nextt - 2] -  _bboxes[ 6 * nextt - 5];
      dimz =  _bboxes[ 6 * nextt - 1] -  _bboxes[ 6 * nextt - 4];
      itop = 1 + itop;
      assert (itop <= 32);

      istack[itop - 1] = nextt;
      imin[itop - 1] = 1 + imd;
      imax[itop - 1] = imx;
      if ((dimx >= dimy)&&(dimx >= dimz)) {
        ict[itop - 1] = 1;
      }
      else if (dimy >= dimz) {
        ict[itop - 1] = 2;
      }
      else {
        ict[itop - 1] = 3;
      }
      nextt = 1 + nextt;
    }
  }

  delete [] points;
}

void  KD_tree_3::kdtree_bboxes_3( const double *xs) {
  static int istack[64];
  int n = 2*_npnts-1;
  int itop, node, ind, child, v;
  int i;

  /* size of stack is twice of the depth of tree. So 64 is plenty. */
  for (i=0; i<n; i++) {
    _bboxes[6 * i] = HUGE_VAL;
  }
  itop = 1;
  istack[0] = 1;
  while (itop > 0) {
    node = istack[itop - 1];
    ind = _tree[node - 1];
    if ( _bboxes[ 6 * ind - 6] != HUGE_VAL) {
      assert( _bboxes[6 * ind] != HUGE_VAL);
      /* Pop the node from stack if its children have been visited. */
        
      itop = itop - 1;
      /* Assign bounding box */
        
      _bboxes[ 6 * node - 6] = min(_bboxes[6 * ind], _bboxes[ 6 * ind - 6]);
      _bboxes[ 6 * node - 5] = min(_bboxes[1 + 6 * ind], _bboxes[ 6 * ind - 5]);
      _bboxes[ 6 * node - 4] = min(_bboxes[2 + 6 * ind], _bboxes[ 6 * ind - 4]);
      _bboxes[ 6 * node - 3] = max(_bboxes[3 + 6 * ind], _bboxes[ 6 * ind - 3]);
      _bboxes[ 6 * node - 2] = max(_bboxes[4 + 6 * ind], _bboxes[ 6 * ind - 2]);
      _bboxes[ 6 * node - 1] = max(_bboxes[5 + 6 * ind], _bboxes[ 6 * ind - 1]);
    }
    else {
      /* Vist children */
        
      for (i=0; i<=1; i++) {
        child = ind + i;
        /*.... If child is a leaf, add point to the indices */
          
        if ( _tree[child - 1] < 0) {
          v = - _tree[child - 1];
          _bboxes[ 6 * child - 6] = xs[ 3 * v - 3];
          _bboxes[ 6 * child - 3] = xs[ 3 * v - 3];
          _bboxes[ 6 * child - 5] = xs[ 3 * v - 2];
          _bboxes[ 6 * child - 2] = xs[ 3 * v - 2];
          _bboxes[ 6 * child - 4] = xs[ 3 * v - 1];
          _bboxes[ 6 * child - 1] = xs[ 3 * v - 1];
        }
        else {
          itop = 1 + itop;
          assert(itop <= 64);
          istack[itop - 1] = child;
        }
      }
    }
  }
}
  
int KD_tree_3::kdtree_search_3( const double range[6], int start) {
  int nfound_out, itop, node, ind, child;
  int istack[32];
  bool wholesubtree[32];
  bool wst;
  int i;
  int offset = start-1;

  /* size of the stack is bounded by the depth of tree. So 32 is plenty. */
  /* size of istack is bounded by the depth of tree. So MAXDEPTH is plenty. */
    
  for (i=0; i<=31; i++) {
    wholesubtree[i] = false;
  }
  /*.... If the bounding boxes do not intersect, then return directly. */
    
  nfound_out = 0;
  if (( !_bboxes || _bboxes[2] > range[5]) || ( _bboxes[1] > range[4]) ||
      ( _bboxes[3] < range[0]) || ( _bboxes[0] > range[3]) ||
      ( _bboxes[4] < range[1]) || ( _bboxes[5] < range[2])) {
    return nfound_out;
  }
  /*.... If root node is a leaf, return leaf. */
    
  if ( _tree[0] < 0) {
    nfound_out = 1;
    if (_maxout >= 1) {
      _indices[0] = -_tree[0]+offset;
    }

    return nfound_out;
  }
  itop = 1;
  istack[0] = 1;
  /*.... Traverse (relevant part of) k-D tree using stack */
    
  while (itop > 0) {
    /*.... pop node off of stack */
      
    node = istack[itop - 1];
    ind = _tree[node - 1];
    wst = wholesubtree[itop - 1];
    if (!wst) {
      wst = range[0] <=  _bboxes[ 6 * node - 6] &&
        range[3] >=  _bboxes[ 6 * node - 3] &&
        range[1] <=  _bboxes[ 6 * node - 5] &&
        range[4] >=  _bboxes[ 6 * node - 2] &&
        range[2] <=  _bboxes[ 6 * node - 4] &&
        range[5] >=  _bboxes[ 6 * node - 1];
    }
    itop = itop - 1;
    /*.... check if either child of NODE is a leaf or should be */
    /*.... put on stack. */
    
    for (i=0; i<=1; i++) {
      child = ind + i;
      /*.... If child is a leaf, add point to the indices */

      if ( wst || _bboxes[ 6 * child - 4] <= range[5] &&
           _bboxes[ 6 * child - 5] <= range[4] &&
           _bboxes[ 6 * child - 3] >= range[0] &&
           _bboxes[ 6 * child - 6] <= range[3] &&
           _bboxes[ 6 * child - 2] >= range[1] &&
           _bboxes[ 6 * child - 1] >= range[2]) {
        if ( _tree[child - 1] < 0) {
          nfound_out = 1 + nfound_out;
          if (_maxout >= nfound_out)
            _indices[nfound_out - 1] = -_tree[child - 1]+offset;
        }
        else {
          itop = 1 + itop;
          assert(itop <= 32);
            
          istack[itop - 1] = child;
          wholesubtree[itop - 1] = wst;
        }
      }
    }
  }

  return nfound_out;
}

