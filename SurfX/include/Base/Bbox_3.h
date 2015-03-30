//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef RFC_BBOX_3_H
#define RFC_BBOX_3_H

#include "rfc_basic.h"
#include <cmath>

RFC_BEGIN_NAME_SPACE

class Bbox_3 
{ 
public:
  Bbox_3() {
    _min[0] = _min[1] = _min[2] = HUGE_VAL;
    _max[0] = _max[1] = _max[2] = -HUGE_VAL;
  }

  Bbox_3(double x_min, double y_min, double z_min,
	 double x_max, double y_max, double z_max) {
    _min[0]=x_min; _min[1]=y_min; _min[2]=z_min;
    _max[0]=x_max; _max[1]=y_max; _max[2]=z_max;
  }

  template <class Point3>
  explicit Bbox_3( const Point3 &p) {
    _min[0]=p[0]; _min[1]=p[1]; _min[2]=p[2];
    _max[0]=p[0]; _max[1]=p[1]; _max[2]=p[2];
  }

  double  xmin() const { return _min[0]; }
  double  ymin() const { return _min[1]; }
  double  zmin() const { return _min[2]; }
  double  xmax() const { return _max[0]; }
  double  ymax() const { return _max[1]; }
  double  zmax() const { return _max[2]; }

  Bbox_3  operator+(const Bbox_3& b) const {
    Bbox_3 box=*this; box+=b; return box;
  }

  Bbox_3&  operator+=(const Bbox_3& b) {
    _min[0] = std::min(_min[0], b._min[0]);
    _min[1] = std::min(_min[1], b._min[1]);
    _min[2] = std::min(_min[2], b._min[2]);

    _max[0] = std::max(_max[0], b._max[0]);
    _max[1] = std::max(_max[1], b._max[1]);
    _max[2] = std::max(_max[2], b._max[2]);

    return *this;
  }

  // Two bounding boxes are considered to match if for every bound,
  // the difference between the two boxes is smaller than a fraction (eps)
  // of the largest dimension of the two boxes.
  bool do_match(const Bbox_3& bb, double tol) const {
    return (std::abs(xmax()-bb.xmax()) <= tol && 
	    std::abs(xmin()-bb.xmin()) <= tol && 
	    std::abs(ymax()-bb.ymax()) <= tol && 
	    std::abs(ymin()-bb.ymin()) <= tol && 
	    std::abs(zmax()-bb.zmax()) <= tol && 
	    std::abs(zmin()-bb.zmin()) <= tol);
  }

private:
  double  _min[3],_max[3];
};

inline bool do_overlap(const Bbox_3& bb1, const Bbox_3& bb2) {

  return !(bb1.xmax() < bb2.xmin() || bb2.xmax() < bb1.xmin() ||
           bb1.ymax() < bb2.ymin() || bb2.ymax() < bb1.ymin() ||
           bb1.zmax() < bb2.zmin() || bb2.zmax() < bb1.zmin());
}

inline std::ostream&
operator<<(std::ostream &os, const Bbox_3& b) {
  return os << b.xmin() << ' ' << b.ymin() << ' ' << b.zmin() << ' ' 
            << b.xmax() << ' ' << b.ymax() << ' ' << b.zmax();
}

inline std::istream&
operator>>(std::istream &is, Bbox_3& b) {
  double xmin, ymin, zmin, xmax, ymax, zmax;

  is >> xmin >> ymin >> zmin >> xmax >> ymax >> zmax ;
  b = Bbox_3(xmin, ymin, zmin, xmax, ymax, zmax);
  return is;
}

inline bool do_overlap_eps(const Bbox_3& bb1, const Bbox_3& bb2, 
			   const double eps) {
  // check for emptiness ??
  if (bb1.xmax()+eps < bb2.xmin() || bb2.xmax()+eps < bb1.xmin())
    return false;
  if (bb1.ymax()+eps < bb2.ymin() || bb2.ymax()+eps < bb1.ymin())
    return false;
  if (bb1.zmax()+eps < bb2.zmin() || bb2.zmax()+eps < bb1.zmin())
    return false;
  return true;
}

inline bool do_overlap_strict(const Bbox_3& bb1, const Bbox_3& bb2)
{
  // check for emptiness ??
  if (bb1.xmax() <= bb2.xmin() || bb2.xmax() <= bb1.xmin())
    return false;
  if (bb1.ymax() <= bb2.ymin() || bb2.ymax() <= bb1.ymin())
    return false;
  if (bb1.zmax() <= bb2.zmin() || bb2.zmax() <= bb1.zmin())
    return false;
  return true;
}

template <class InIter>
Bbox_3 get_Bbox_3(InIter first, InIter last) {
  Bbox_3  box;

  while (first != last) {
    box += first->bbox();
    ++first;
  }
  return box;
}

RFC_END_NAME_SPACE

#endif // RFC_BBOX_3_H

