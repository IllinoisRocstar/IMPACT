//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/* \file RFC_Window_overlay_fea.C
 * This file implements the feature-detection algorithm using url-thresholding.
 */

#include "HDS_accessor.h"
#include "Overlay_primitives.h"
#include "RFC_Window_overlay.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include "Timing.h"

RFC_BEGIN_NAME_SPACE

using namespace std;

static HDS_accessor acc;
const float RFC_Window_overlay::r2d = 180. / 3.1415926535;

/** @addtogroup fdp Feature-Detection Primitives
 *  @{
 */
/** Read in the control file for feature detection. The control file
 *  should be named as <window name>.fc. It should contain five lines.
 *  The first three lines correspond to the parameters for face angle,
 *  angle defect, and edge angle, respectively. The fourth line controls
 *  filteration rules, including the minimum edge length for
 *  open-ended 1-features, whether to apply long-falseness checking with
 *  strong endedness, and whether whether to snap the feature onto the
 *  reference mesh. The last line controls verbose level.
 *  A sample control file (default) is:

 0.76604444 0.98480775  3 0.98480775
 1.3962634  0.314159265 3
 0.17364818 0.96592583  3
 6 1 0 0
 1

 * For RSRM dataset, use the following control files:
 0.5 0.6 3 0.17365  # cos(face_angle_ub) cos(face_angle_lb) face_angle_r
 cos(weak_end) 1.39626 0.314159 3 # angle_defect_ub angle_defect_lb
 angle_defect_r 0 0.5 3            # cos(turn_angle_ub) cos(turn_angle_lb)
 turn_angle_r 6 1 1 0            # min-length-of-ridge long-falseness-rule
 strong-ended-check snapping 1                  # verbose level

 * If the control file is missing, then the default values (above)
 * will be used. These default values should work for most cases.
 * For some extreme cases, adjusting the signal-to-noise ratios (the
 * last parameters of the first three lines) and the minimum edge
 * (line 4) should suffice.
 */
void RFC_Window_overlay::init_feature_parameters() {
  std::string fname = string(name()) + ".fc";
  std::ifstream f(fname.c_str());

  if (f) {
    std::string buf;
    std::cout << "Reading in parameters from file " << fname << "..."
              << std::flush;
    f >> _cos_uf >> _cos_lf >> _rf >> _cos_weakend;
    getline(f, buf);
    f >> _ud >> _ld >> _rd;
    getline(f, buf);
    f >> _cos_ue >> _cos_le >> _re;
    getline(f, buf);
    f >> _min_1f_len >> _long_falseness_check >> _strong_ended >>
        _snap_on_features;
    getline(f, buf);
    f >> verb;
    getline(f, buf);

    RFC_assertion(_cos_uf >= 0 && _cos_uf <= 1 && _cos_lf <= 1 &&
                  _cos_lf >= _cos_uf);
    RFC_assertion(_rf == 0 || _rf >= 1);
    RFC_assertion(_ud >= 0 && _ud <= 180 / r2d && _ld >= 0 && _ld <= _ud);
    RFC_assertion(_rd == 0 || _rd >= 1);
    RFC_assertion(_cos_ue >= 0 && _cos_ue <= 1 && _cos_le <= 1 &&
                  _cos_le >= _cos_ue);
    RFC_assertion(_re == 0 || _re >= 1);
    RFC_assertion(_min_1f_len >= 1);
    std::cout << "Done" << std::endl;
  } else {
    // Use default parameters and write the parameters into the file
    _cos_uf = cos(50 / r2d);
    _cos_lf = cos(15 / r2d);
    _rf = 3;
    _cos_weakend = _cos_lf;
    _ud = 60 / r2d;
    _ld = 18 / r2d;
    _rd = 3;
    _cos_ue = cos(60 / r2d);
    _cos_le = cos(30 / r2d);
    _re = 3;
    _min_1f_len = 6;
    _long_falseness_check = true;
    _strong_ended = true;
    _snap_on_features = false;
    verb = 1;
  }
}

/** Compute the cosine of the face angle (dihedral angle) at an
 *  edge. When the face angle was computed the first time, it is saved and
 *  subsequent calls for the same edge will simply return the stored value.
 *  The face angle is defined to be pi if the edge is on the border.
 */
float RFC_Window_overlay::cos_face_angle(const HEdge &h) {
  // Precondition: Face angles are initialized to HUGE_VALF.
  RFC_Pane_overlay *p1 = h.pane();
  float &t = p1->get_cos_face_angle(h);
  if (t < HUGE_VALF) return t;
  assert(h.is_border_g());

  t = -1;
  return t;
}

/** Compute the cosine of the face angle (dihedral angle) at an
 *  edge. When the face angle was computed the first time, it is saved and
 *  subsequent calls for the same edge will simply return the stored value.
 *  The face angle is defined to be pi if the edge is on the border.
 */
float RFC_Window_overlay::cos_face_angle(const HEdge &h, const HEdge &hopp) {
  // Precondition: Face angles are initialized to HUGE_VALF.
  RFC_Pane_overlay *p1 = h.pane();
  float &t = p1->get_cos_face_angle(h);
  if (t < HUGE_VALF) return t;

  if (h.is_border_g() || hopp.is_border_g())
    t = -1;
  else {
    Overlay_primitives op;

    Vector_3 v1 = op.get_face_normal(h, Point_2(0.5, 0));
    Vector_3 v2 = op.get_face_normal(hopp, Point_2(0.5, 0));

    t = std::min(v1 * v2 / sqrt((v1 * v1) * (v2 * v2)), 1.);
    RFC_Pane_overlay *p2 = hopp.pane();
    if (p1 != p2) p2->get_cos_face_angle(hopp) = t;
  }
  return t;
}

/** Compute the cosine of the edge angle at a vertex between two incident
 *  feature edges.
 */
float RFC_Window_overlay::cos_edge_angle(const HEdge &h1, const HEdge &h2) {
  Vector_3 v1 = get_tangent(h1);
  Vector_3 v2 = get_tangent(h2);

  float d =
      std::max(-1., std::min((v1 * v2) / sqrt((v1 * v1) * (v2 * v2)), 1.));
  if (h1.destination_g() == h2.destination_g() ||
      h1.origin_g() == h2.origin_g())
    return -d;
  else
    return d;
}

/** Compute the cosine of the edge angle at a vertex in a given feature curve.
 *  It saves the solution by associating it with the vertex. When the routine
 *  is invoked on the same vertex next time, it will return the saved value
 *  if the vertex is not at the end of the feature curve.
 */
float RFC_Window_overlay::cos_edge_angle(const Feature_1 &f1,
                                         Feature_1::const_iterator it,
                                         bool isloop) {
  Node v;
  if (!isloop) {
    if (it == f1.begin()) {
      v = (*it).origin_g();
      v.pane()->get_cos_edge_angle(v) = HUGE_VALF;
      return -1;
    } else if (it == f1.end()) {
      v = (f1.back()).destination_g();
      v.pane()->get_cos_edge_angle(v) = HUGE_VALF;
      return -1.;
    }
  }

  if (it == f1.begin() || it == f1.end()) {
    v = f1.front().origin_g();
    float &t = v.pane()->get_cos_edge_angle(v);
    if (t == HUGE_VALF) t = cos_edge_angle(f1.front(), f1.back());
    return t;
  } else {
    v = it->origin_g();
    float &t = v.pane()->get_cos_edge_angle(v);
    if (t == HUGE_VALF) {
      Feature_1::const_iterator ip = it;
      --ip;
      t = cos_edge_angle(*it, *ip);
    }
    return t;
  }
}

/** Compute the angle defect of a vertex. The angle defect at v is defined as:
 *  1. the difference between 2*pi and the sum of its angles in incident faces
 *     if v is not a border vertex;
 *  2. the difference between pi and the sum of its angles in incident faces
 *     if v is a border vertex.
 *  When the angle defect was computed for a vertex for the first time, it is
 *  saved, and subsequent calls for the same vertex will return saved value.
 */
float RFC_Window_overlay::comp_angle_defect(const Node &v) {
  const float pi = 3.1415926535;

  // Precondition: d was initialized to HUGE_VALF
  float &t = v.pane()->get_angle_defect(v), d = t;
  if (d < HUGE_VALF) return d;

  float angle_sum = 0.;

  HEdge h = v.halfedge_g(), h0 = h;
  bool is_border = false;
  do {
    if (!h.is_border_g())
      angle_sum += acos(-cos_edge_angle(h, h.next_l()));
    else
      is_border = true;
  } while ((h = acc.get_next_around_destination(h)) != h0);

  return t = abs((2 - is_border) * pi - angle_sum);
}

template <class T>
T squares(const T &t) {
  return t * t;
}

/** Determine whether a vertex is strong (either theta-strong or
 *  relatively strong) in angle defect.
 */
bool RFC_Window_overlay::is_strong_ad(const Node &v) {
  // Determine whether the edge is theta-strong
  float d = comp_angle_defect(v);
  if (d >= _ud) {
    return true;
  } else if (d >= _ld) {
    float max_ad = 0;
    HEdge h0 = v.halfedge_g(), h1 = h0;
    // Loop through incident edges of v
    do {
      float a = comp_angle_defect(h1.origin_g());
      max_ad = std::max(max_ad, a);
    } while ((h1 = acc.get_next_around_destination(h1)) != h0);

    if (d >= _rd * max_ad) return true;
  }
  return false;
}

/** Determine whether a vertex is relatively strong in edge angle
 *  within a give feature.
 */
bool RFC_Window_overlay::is_rstrong_ea(const Feature_1 &f1,
                                       Feature_1::const_iterator hprev,
                                       Feature_1::const_iterator hnext,
                                       float cos_ea, bool isloop) {
  if (cos_ea > _cos_le) return false;
  float cos_max = std::min(cos_edge_angle(f1, hprev, isloop),
                           cos_edge_angle(f1, hnext, isloop));
  return acos(cos_ea) >= _re * acos(cos_max);
}

/** @} end of fdp */

/** @addtogroup mf1 Manipulation of Strong Curves
 * @{
 */
/** Merge two feature curves into one at vertex v. After the operation,
 *  f1 becomes the union of f1 and f2, and f2 becomes empty.
 */
void RFC_Window_overlay::merge_features_1(const Node &v, Feature_1 &f1,
                                          Feature_1 &f2) {
  RFC_assertion(!f1.empty() && f1 != f2);
  if (f2.empty()) return;

  Node dst, src;

  if ((dst = f1.back().destination_g()) == f2.front().origin_g() && v == dst)
    f1.splice(f1.end(), f2);
  else if ((src = f1.front().origin_g()) == f2.back().destination_g() &&
           v == src) {
    f2.splice(f2.end(), f1);
    f1.swap(f2);
  } else if (dst == f2.back().destination_g() && v == dst) {
    while (!f2.empty()) {
      f1.push_back(f2.back().opposite_g());
      f2.pop_back();
    }
  } else {
    RFC_assertion(src == f2.front().origin_g() && v == src);
    while (!f2.empty()) {
      f1.push_front(f2.front().opposite_g());
      f2.pop_front();
    }
  }
  RFC_assertion(f2.empty());
}

/** Determine whether a curve is false strong.
 */
bool RFC_Window_overlay::check_false_strong_1(Feature_1 &f1) {
  if (cos_face_angle(f1.front()) == -1) return false;

  Node src = f1.front().origin_g();
  Node dst = f1.back().destination_g();

  // is a loop or strong at both ends
  if (src == dst ||
      (is_on_feature(src) && is_on_feature(dst) &&
       (!_f0_ranks.empty() || is_feature_0(src) || is_feature_0(dst))) ||
      (cos_face_angle(f1.front()) < 0 && cos_face_angle(f1.back()) < 0))
    return false;

  // Is it weak ended? If so, pop out the weak edges
  if (_strong_ended) {
    if (!f1.empty())
      for (;;) {
        HEdge h = f1.front(), hopp = h.opposite_g();

        if (is_on_feature(src) || cos_face_angle(h, hopp) <= _cos_uf ||
            is_strong_ad(src))
          break;
        else {
          h.pane()->unset_strong_edge(h);
          hopp.pane()->unset_strong_edge(hopp);
          f1.pop_front();
        }
        if (!f1.empty())
          src = f1.front().origin_g();
        else
          return true;
      }

    if (!f1.empty())
      for (;;) {
        HEdge h = f1.back();
        HEdge hopp = h.opposite_g();

        if (is_on_feature(dst) || cos_face_angle(h, hopp) <= _cos_uf ||
            is_strong_ad(src))
          break;
        else {
          h.pane()->unset_strong_edge(h);
          hopp.pane()->unset_strong_edge(hopp);
          f1.pop_back();
        }
        if (!f1.empty())
          dst = f1.back().destination_g();
        else
          return true;
      }
  }

  // is it too short?
  if (_min_1f_len) {
    int c = !(is_strong_ad(src) || is_feature_0(src)) +
            !(is_strong_ad(dst) || is_feature_0(dst));
    if (int(f1.size()) <= c * _min_1f_len) return true;
  }

  // is it too close to another features?
  if (_long_falseness_check) {
    if (_f_list_1.empty() || ((is_strong_ad(src) || is_feature_0(src)) &&
                              (is_strong_ad(dst) || is_feature_0(dst))))
      return false;
    for (Feature_1::const_iterator it = ++f1.begin(); it != f1.end(); ++it) {
      Node v = it->origin_g();
      HEdge h0 = v.halfedge_g(), h1 = h0;
      // Loop through incident edges of v
      do {
        if (is_on_feature(h1.origin_g())) {
          v = Node();
          break;
        }
      } while ((h1 = acc.get_next_around_destination(h1)) != h0);
      if (v.pane() == NULL)
        continue;
      else
        return false;
    }
    return true;
  }
  return false;
}

/** Subdivide a feature curve by splitting it at 0-features.
 *  Some false strong edges may be filtered out during this step.
 */
void RFC_Window_overlay::subdiv_feature_curve(const Feature_1 &f1,
                                              Feature_list_1 &new_flist,
                                              int &dropped) {
  // We loop through all vertices in the curve to locate the 0-features
  list<Feature_1::const_iterator> divs;
  Node src = f1.front().origin_g();
  Node trg = f1.back().destination_g();
  bool isloop = (src == trg);

  // A 1-feature is dangling if one of its end point is a terminus
  bool isdangling = !isloop && (_f0_ranks.find(src) == _f0_ranks.end() ||
                                _f0_ranks.find(trg) == _f0_ranks.end());

  float cos_min_fa_before = 1, cos_min_fa_after = 1;
  Feature_1::const_iterator f1end = f1.end(), hfirst = f1end, hlast = f1end;

  for (Feature_1::const_iterator hprev = f1.begin(), hi = ++f1.begin(),
                                 hnext = hi;
       hi != f1end; hprev = hi, hi = hnext) {
    ++hnext;
    Node v = hi->origin_g();
    if (is_feature_0(v)) {
      RFC_assertion(hi == f1.begin());
      continue;
    }

    float cos_ea = cos_edge_angle(f1, hi, isloop);
    if (cos_ea <= _cos_ue || is_rstrong_ea(f1, hprev, hnext, cos_ea, isloop)) {
      divs.push_back(hi);
      // When breaking loops, mark breakpoints as corners
      set_feature_0(v);
    } else if (isdangling) {
      // Mark transitions between strong and week edges for danging 1-features
      // as 0-feature
      float dpre = cos_face_angle(*hprev, hprev->opposite_g());
      float d = cos_face_angle(*hi, hi->opposite_g());

      if (hfirst == f1end && dpre < cos_min_fa_before) cos_min_fa_before = dpre;

      if (std::min(dpre, d) <= _cos_weakend &&
          std::max(dpre, d) > _cos_weakend) {
        if (hfirst == f1end) hfirst = hi;
        hlast = hi;
        cos_min_fa_after = 1.;
      }

      if (d < cos_min_fa_before) cos_min_fa_after = d;
    }
  }

  if (isdangling && divs.empty()) {
    if (hfirst != f1end && cos_min_fa_before > _cos_weakend &&
        _f0_ranks.find(src) == _f0_ranks.end()) {
      divs.push_back(hfirst);
      set_feature_0(hfirst->origin_g());
    }

    if (hlast != f1end && hlast != hfirst && cos_min_fa_after > _cos_weakend &&
        _f0_ranks.find(trg) == _f0_ranks.end()) {
      divs.push_back(hlast);
      set_feature_0(hlast->origin_g());
    }
  }

  if (isloop) {
    // Finally, we process end vertices
    Feature_1::const_iterator hi = f1.begin();
    float cos_ea = cos_edge_angle(f1, hi, isloop);
    if (cos_ea <= _cos_ue ||
        is_rstrong_ea(f1, f1end, ++f1.begin(), cos_ea, isloop)) {
      // When breaking loops, mark breakpoints as corners
      divs.push_back(f1end);
      set_feature_0(hi->origin_g());
    }
  } else
    divs.push_back(f1end);

  // Now subdivide the curve into sub-curves
  Feature_list_1 subcur;
  if (divs.size() == 0)
    subcur.push_back(f1);
  else {
    subcur.push_back(Feature_1(f1.begin(), divs.front()));
    list<Feature_1::const_iterator>::const_iterator dit = divs.begin();
    for (list<Feature_1::const_iterator>::const_iterator dinext = dit;
         ++dinext != divs.end(); dit = dinext)
      subcur.push_back(Feature_1(*dit, *dinext));

    if (isloop) {
      Feature_1 &newf = subcur.front();
      newf.insert(newf.begin(), *dit, f1end);
    }
  }

  // Check the sub-curves to filter out false-strongness.
  for (Feature_list_1::iterator sit = subcur.begin(); sit != subcur.end();
       ++sit) {
    if (check_false_strong_1(*sit)) {
      for (Feature_1::const_iterator i = sit->begin(); i != sit->end(); ++i) {
        i->pane()->unset_strong_edge(*i);
        ++dropped;
        HEdge hopp = i->opposite_g();
        hopp.pane()->unset_strong_edge(hopp);
      }
    } else {
      // Set all vertices on-feature
      set_on_feature(sit->front().origin_g());
      for (Feature_1::const_iterator i = sit->begin(); i != sit->end(); ++i)
        set_on_feature(i->destination_g());

      Node src = sit->front().origin_g();
      Node dst = sit->back().destination_g();

      if (src != dst) {
        set_feature_0(src);
        set_feature_0(dst);
      }
      new_flist.push_back(Feature_1());
      new_flist.back().swap(*sit);
    }
  }
}

/** Remove a false strong curve. */
void RFC_Window_overlay::remove_feature_1(Feature_1 &f) {
  for (Feature_1::iterator it = f.begin(), iend = f.end(); it != iend; ++it) {
    unset_strong_edge(*it);
    unset_strong_edge(it->opposite_g());
  }

  for (Feature_1::iterator it = f.begin(), iend = f.end(); it != iend; ++it) {
    HEdge k = *it, kopp = k.opposite_g();
    RFC_assertion(!k.pane()->_f_n_index.empty());
    do {
      HEdge h0 = k, h = h0;
      do {
        RFC_assertion(!is_feature_1(h));
        RFC_Pane_overlay &pane = *h.pane();
        pane._f_n_index[pane.get_index(h)] = -1;
      } while ((h = acc.get_next_around_destination(h)) != h0);
    } while ((k = (k == kopp) ? *it : kopp) != *it);
  }
}

/** @} end of mf1 */

/** @addtogroup mf0 Manipulation of Strong Vertices.
 *  @{
 */
/** Identify the 0-features.
 */
void RFC_Window_overlay::identify_features_0() {
  Feature_list_1 new_flist;
  _f_list_1.swap(new_flist);

  Feature_list_1::iterator flit;
  // Split feature curves at known feature vertices
  for (flit = new_flist.begin(); flit != new_flist.end(); ++flit) {
    Feature_list_1 subf;
    Feature_1::iterator fit = flit->begin(), fprev = fit;
    for (++fit; fit != flit->end(); ++fit) {
      if (is_feature_0(fit->origin_g())) {
        subf.push_back(Feature_1(fprev, fit));
        fprev = fit;
      }
    }

    if (is_feature_0(flit->begin()->origin_g()) || subf.empty()) {
      subf.push_back(Feature_1(fprev, fit));
    } else {
      Feature_1 &newf = subf.front();
      newf.insert(newf.begin(), fprev, flit->end());
    }

    // Assign the ranks and mark the ones with rank 2.
    for (Feature_list_1::iterator sit = subf.begin(); sit != subf.end();
         ++sit) {
      _f_list_1.push_back(*sit);
    }
  }

  // Determine the ranks of the vertices
  RFC_assertion(_f0_ranks.empty());
  for (flit = _f_list_1.begin(); flit != _f_list_1.end(); ++flit) {
    Node src = flit->front().origin_g();
    Node dst = flit->back().destination_g();

    map<Node, int>::iterator i = _f0_ranks.find(src);
    if (i == _f0_ranks.end())
      _f0_ranks.insert(make_pair(src, -1 - (src == dst)));
    else
      i->second = abs(i->second) + 1 + (src == dst);
    if (src != dst) {
      if ((i = _f0_ranks.find(dst)) == _f0_ranks.end())
        _f0_ranks.insert(make_pair(dst, -1));
      else
        i->second = abs(i->second) + 1;
    }
  }

  // Determine the connectivity at rank-2 vertices.
  std::map<Node, std::vector<Feature_1 *> > turn_maps;
  for (flit = _f_list_1.begin(); flit != _f_list_1.end(); ++flit) {
    // Assign the ranks and mark the ones with rank 2.
    Node src = flit->front().origin_g();
    map<Node, int>::iterator i = _f0_ranks.find(src);
    if (i != _f0_ranks.end() && abs(i->second) == 2)
      turn_maps[src].push_back(&*flit);

    Node dst = flit->back().destination_g();
    i = _f0_ranks.find(dst);
    if (i != _f0_ranks.end() && abs(i->second) == 2)
      turn_maps[dst].push_back(&*flit);
  }

  if (!turn_maps.empty()) {
    std::map<Node, std::vector<Feature_1 *> >::iterator i;
    RFC_assertion_code(for (i = turn_maps.begin(); i != turn_maps.end(); ++i)
                           RFC_assertion(i->second.size() == 2));

    new_flist.clear();
    // Merge at weak rank-2 vertices
    for (flit = _f_list_1.begin(); flit != _f_list_1.end(); ++flit) {
      if (flit->empty()) continue;
      Feature_1 &f1 = *flit;

      bool modified = false;
      Node src = f1.front().origin_g();
      Node dst = f1.back().destination_g();

      if (src == dst) {
        if ((i = turn_maps.find(src)) != turn_maps.end()) {
          turn_maps.erase(i);
          if (!is_strong_ad(src)) {
            src.pane()->unset_feature_0(src);
            _f0_ranks.erase(_f0_ranks.find(src));
          } else {
            src.pane()->set_feature_0(src);
            _f0_ranks[src] = 2;
          }
        }
      } else
        for (int c = 0; c < 2; ++c) {
          for (;;) {
            Node v = (c ? f1.back().destination_g() : f1.front().origin_g());
            if ((i = turn_maps.find(v)) == turn_maps.end()) break;
            Feature_1 *f2;
            if ((f2 = i->second[0]) == &f1) f2 = i->second[1];
            turn_maps.erase(i);
            if (is_feature_0(v)) {
              v.pane()->unset_feature_0(v);
              _f0_ranks.erase(_f0_ranks.find(v));
            }
            if (f2 == &f1) break;
            merge_features_1(v, f1, *f2);

            Node u;
            if ((u = f1.back().destination_g()) != v &&
                (i = turn_maps.find(u)) != turn_maps.end()) {
              if (i->second[0] == f2) i->second[0] = &f1;
              if (i->second[1] == f2) i->second[1] = &f1;
            } else if ((u = f1.front().origin_g()) != v &&
                       (i = turn_maps.find(u)) != turn_maps.end()) {
              if (i->second[0] == f2) i->second[0] = &f1;
              if (i->second[1] == f2) i->second[1] = &f1;
            }
            modified = true;
          }
        }

      // Split at strong rank-2 vertices.
      if (modified) {
        Feature_list_1 subf;
        int dropped = 0;
        subdiv_feature_curve(f1, subf, dropped);
        RFC_assertion(dropped == 0);
        f1.clear();
        for (Feature_list_1::iterator it = subf.begin(); it != subf.end();
             ++it) {
          new_flist.push_back(Feature_1());
          Node v = it->front().origin_g();
          if (is_feature_0(v) && _f0_ranks.find(v) == _f0_ranks.end())
            _f0_ranks[v] = 2;
          new_flist.back().swap(*it);
        }
      } else {
        new_flist.push_back(Feature_1());
        new_flist.back().swap(f1);
      }
    }
    _f_list_1.swap(new_flist);
  }
  RFC_assertion(turn_maps.empty());

  // Fill 0-feature list
  for (map<Node, int>::iterator it = _f0_ranks.begin(); it != _f0_ranks.end();
       ++it) {
    // Adjust the rank for termini
    if (it->second == -1 && is_strong_ad(it->first)) it->second = 1;

    RFC_assertion(is_feature_0(it->first));
    _f_list_0.push_back(Feature_0(it->first));
  }
  _f_list_0.sort();
}

/** Remove the given 0-feature from the list */
RFC_Window_overlay::Feature_list_0::iterator
RFC_Window_overlay::remove_feature_0(Feature_list_0::iterator i) {
  Node v = i->vertex();
  v.pane()->unset_feature_0(v);
  return _f_list_0.erase(i);
}

/** @} end of mf0 */

/** @defgroup pifd Public Interface for Feature Detection
 * @{
 */
/** The main entry of feature detection. */
void RFC_Window_overlay::detect_features() {
  int size_edges = 0, dropped = 0;

  // Initializing data arrays.
  Pane_set::iterator it = _pane_set.begin(), iend = _pane_set.end();
  for (; it != iend; ++it) {
    RFC_Pane_overlay &pane = (RFC_Pane_overlay &)*it->second;
    int num_hedgs = 4 * pane.size_of_faces() + pane.size_of_border_edges();
    int num_verts = pane.size_of_nodes();
    size_edges += num_hedgs / 2;

    pane.init_face_angle();
    pane.init_angle_defect();
    pane.init_edge_angle();
    pane._is_f_1.clear();
    pane._is_f_1.resize(num_hedgs, false);
    pane._is_f_0.clear();
    pane._is_f_0.resize(num_verts, false);
    pane._is_on_f.clear();
    pane._is_on_f.resize(num_verts, false);
  }
  std::vector<pair<float, HEdge> > tstrong_edges, rstrong_edges;
  tstrong_edges.reserve(size_edges / 10);
  rstrong_edges.reserve(size_edges / 10);

  float t0 = get_wtime(), totaltime = 0;
  // loop through all panes and primary halfedges in each pane
  for (it = _pane_set.begin(); it != iend; ++it) {
    RFC_Pane_overlay &pane = (RFC_Pane_overlay &)*it->second;

    // Detect theta-strong edges
    for (int i = 0, s = pane.size_of_faces(); i < s; ++i) {
      HEdge h(&pane, Edge_ID(i + 1, 0)), h0 = h;

      do {
        HEdge lopp = h.opposite_l();

        if (lopp.is_border_l() || h.id() < lopp.id()) {
          HEdge hopp = h.opposite_g();
          if (pane.id() <= hopp.pane()->id()) {
            // Determine whether the edge is theta-strong
            float d = cos_face_angle(h, hopp);

            if (d < _cos_uf) tstrong_edges.push_back(make_pair(d, h));
          }
        }
      } while ((h = h.next_l()) != h0);
    }
  }
  float t1 = get_wtime();
  totaltime += t1 - t0;
  if (verb >= 3) {
    std::cout << "\tIdentified " << tstrong_edges.size()
              << " theta-strong edges with Theta=" << acos(_cos_uf) * r2d
              << " in " << t1 - t0 << " sec." << std::endl;
    t0 = get_wtime();
  }

  sort(tstrong_edges.begin(), tstrong_edges.end());
  t1 = get_wtime();
  totaltime += t1 - t0;
  if (verb >= 3) {
    std::cout << "\tSorted theta-strong edges in " << t1 - t0 << " sec."
              << std::endl;
  }

  vector<std::pair<float, HEdge> > iedges;
  iedges.reserve(16);
  _f_list_1.clear();

  // Sort the strong edges into strong curves
  unmark_alledges();
  float min_fa_r = HUGE_VALF;
  t0 = get_wtime();
  for (vector<pair<float, HEdge> >::iterator it = tstrong_edges.begin(),
                                             iend = tstrong_edges.end();
       it != iend; ++it) {
    HEdge h = it->second;
    if (acc.marked(h)) continue;
    HEdge hopp = h.opposite_g();

    // Create a new list
    Feature_1 f1;
    f1.push_back(h);

    // Mark the edge and its opposite
    h.pane()->set_strong_edge(h);
    acc.mark(h);
    hopp.pane()->set_strong_edge(hopp);
    acc.mark(hopp);

    Node src = f1.front().origin_g();
    Node dst = f1.back().destination_g();

    // Traverse forwards and backwards along the curve, and append an
    // r-strong edge to the curve until we reach a non-strong curve.
    for (int c = 0; c < 2; ++c) {
      for (;;) {
        HEdge h = ((c == 0) ? f1.back() : f1.front());
        if (src == dst || (c == 0 && is_on_feature(dst)) ||
            (c && is_on_feature(src)))
          break;

        // Get the unmarked 1-feature with minimum angle
        iedges.clear();
        HEdge h0 = h.opposite_g();
        float d0 = cos_face_angle(h, h0);
        pair<float, HEdge> t0(d0, (c == 0) ? h0 : h);
        pair<float, HEdge> cos_max(HUGE_VALF, HEdge());

        HEdge h1 = acc.get_next_around_origin(t0.second);
        do {
          HEdge h1o = h1.opposite_g();
          float d = cos_face_angle(h1, h1o);
          pair<float, HEdge> t(d, h1);
          iedges.push_back(t);

          if (t < cos_max) cos_max = t;
        } while ((h1 = acc.get_next_around_origin(h1)) != t0.second);

        bool is_strong = true;
        const Vector_3 v1 = get_tangent(t0.second);
        if (cos_max.first > _cos_uf && iedges.size() > 1) {
          for (int i = 0, s = iedges.size(); i < s; ++i) {
            const Vector_3 v2 = get_tangent(iedges[i].second);
            const Real t = std::max(-v1 * v2 / sqrt((v1 * v1) * (v2 * v2)), 0.);
            iedges[i].first = acos(iedges[i].first) * t;
          }
          sort(iedges.rbegin(), iedges.rend());
          cos_max = iedges[0];
          if (cos(cos_max.first) > _cos_lf) break;

          is_strong = cos_max.first >= iedges[1].first * _rf ||
                      is_on_feature(cos_max.second.destination_g()) ||
                      is_strong_ad(cos_max.second.destination_g());
          min_fa_r = std::min(min_fa_r, cos_max.first / iedges[1].first);
        }
        const Vector_3 v2 = get_tangent(cos_max.second);
        float a = -v1 * v2 / sqrt((v1 * v1) * (v2 * v2));
        if (a < _cos_ue || a < cos_face_angle(cos_max.second)) break;

        rstrong_edges.push_back(
            make_pair(cos_face_angle(cos_max.second), cos_max.second));

        h = cos_max.second;
        if (acc.marked(h)) break;
        HEdge hopp = h.opposite_g();

        acc.mark(h);
        h.pane()->set_strong_edge(h);
        acc.mark(hopp);
        hopp.pane()->set_strong_edge(hopp);
        if (c == 0) {
          f1.push_back(h);
          dst = h.destination_g();
        } else {
          f1.push_front(hopp);
          src = hopp.origin_g();
        }
        if (!is_strong) break;
      }
    }

    Feature_list_1 flist;
    subdiv_feature_curve(f1, flist, dropped);
    _f_list_1.splice(_f_list_1.end(), flist);
  }

  t1 = get_wtime();
  totaltime += t1 - t0;
  if (verb >= 3) {
    std::cout << "\tIdentified " << rstrong_edges.size()
              << " r-strong edges with r=" << _rf << " in " << t1 - t0
              << " sec." << std::endl;
    sort(rstrong_edges.begin(), rstrong_edges.end());
    dump_strong_edges(tstrong_edges, rstrong_edges);
    t0 = get_wtime();
  }

  identify_features_0();

  totaltime += t1 - t0;
  if (verb >= 1) {
    std::cout << "\tFound " << _f_list_1.size() << " ridges and "
              << _f_list_0.size() << " corners and dropped " << dropped
              << " false-strong edges.\n"
              << "\tDone in " << totaltime << " sec." << std::endl;
#ifndef DEBUG
    if (verb >= 2)
#endif
      print_features();
  }

  // Compute the bounding boxes of the 1-features
  for (Feature_list_1::iterator it = _f_list_1.begin(); it != _f_list_1.end();
       ++it) {
    it->bbox += Bbox_3(it->front().origin_g().point());
    for (Feature_1::iterator hi = it->begin(), hiend = it->end(); hi != hiend;
         ++hi) {
      it->bbox += Bbox_3(hi->destination_g().point());
    }
  }

  // Finalization
  unmark_alledges();

  free_vector(tstrong_edges);
  rstrong_edges.clear();
  for (it = _pane_set.begin(); it != _pane_set.end(); ++it) {
    RFC_Pane_overlay &pane = (RFC_Pane_overlay &)*it->second;

    free_vector(pane._fd_1);
    free_vector(pane._ad_0);
    free_vector(pane._ea_0);
  }
}

/** @} end of pifd */

RFC_END_NAME_SPACE
