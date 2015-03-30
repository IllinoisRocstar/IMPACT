//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

/* \file RFC_Window_overlay_IO.C
 * This file implements I/O for RFC_Window_overlay.
 */

#include "RFC_Window_overlay.h"
#include "Overlay_primitives.h"
#include "HDS_accessor.h"
#include "Timing.h"
#include <functional>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

COM_EXTERN_MODULE( SimOUT);

RFC_BEGIN_NAME_SPACE

using  namespace std;

/** Dump out the 0- and 1-features in Tecplot format into files 
 *  <name>f0.plt, <name>f1.plt, respectively. The original mesh, vertex ranks,
 *  face angles, and edge angles are output into file <name>.plt.
 */
void 
RFC_Window_overlay::print_features() {

  std::cout << "Writing feature info " << "..." << std::endl;

  // Construct a temporary window
  std::string wname_fea = name()+"_fea_temp";

  COM_new_window( wname_fea.c_str());
  COM_use_dataitem( (wname_fea+".mesh").c_str(), (name()+".mesh").c_str());

  COM_new_dataitem( (wname_fea+".frank").c_str(), 'n', COM_INTEGER, 1, "");
  COM_new_dataitem( (wname_fea+".face_angle").c_str(), 'n', COM_FLOAT, 1, "");

  if ( verb > 1) {
    COM_new_dataitem( (wname_fea+".turn_angle").c_str(), 'n', COM_FLOAT, 1, "");
    COM_new_dataitem( (wname_fea+".angle_defect").c_str(), 'n', COM_FLOAT, 1, "");
  }

  Pane_set::iterator it=_pane_set.begin(), iend=_pane_set.end();
  for ( it=_pane_set.begin(); it != iend; ++it) {
    RFC_Pane_overlay &pane = (RFC_Pane_overlay &)*it->second;

    int *frank;
    COM_allocate_array( (wname_fea+".frank").c_str(), pane.id(), 
			&(void*&)frank);

    float *face_angle=NULL, *turn_angle=NULL, *angle_defect=NULL;
    COM_allocate_array( (wname_fea+".face_angle").c_str(), pane.id(), 
			&(void*&)face_angle);
    if ( verb>1) {
      COM_allocate_array( (wname_fea+".turn_angle").c_str(), pane.id(), 
			  &(void*&)turn_angle);
      COM_allocate_array( (wname_fea+".angle_defect").c_str(), pane.id(), 
			  &(void*&)angle_defect);
    }

    for (int i=0,s=pane.size_of_nodes(); i<s; ++i) {
      Node vit(&pane,i+1);
      if (vit.is_isolated()) continue;

      Node v = vit.get_primary();
      if ( is_feature_0( v)) {
	if ( _f0_ranks.find(v)==_f0_ranks.end()) {
	  std::cout << "Found rank-0 vertex: " << v.point() << std::endl;
	  frank[i] = 2; // 1;
	}
	else
	  frank[i] = 2; // _f0_ranks[v]+1;
      }
      else
	frank[i] = is_on_feature(v);
	
      // Compute face angle
      face_angle[i] = 2.;
      HEdge h0=v.halfedge_g(), h1=h0;

      if ( h0.destination_g()==v) do {
          face_angle[i] = std::min( face_angle[i], 
                                    cos_face_angle( h1, h1.opposite_g()));
      } while ( (h1=h1.next_g().opposite_g()) != h0);
      else { // v is an edge center for quadratic element
	RFC_assertion( pane.is_quadratic());
	face_angle[i] = cos_face_angle( h1, h1.opposite_g());
      }

      face_angle[i] = acos( face_angle[i])*r2d;

      if ( verb>1) {
	if ( is_feature_0( v) && _f0_ranks[v]>2)
	  turn_angle[i] = 180;
	else if ( v.pane()->get_cos_edge_angle( v)==HUGE_VALF)
	  turn_angle[i] = 0;
	else 
	  turn_angle[i] = acos(v.pane()->get_cos_edge_angle( v))*r2d;
	
	if ( v.pane()->get_angle_defect(v)==HUGE_VALF)
	  angle_defect[i] = 0.;
	else
	  angle_defect[i] = v.pane()->get_angle_defect(v)*r2d;
      }
    }
  }
   
  COM_window_init_done( wname_fea.c_str());

  // Load Rocout
  COM_LOAD_MODULE_STATIC_DYNAMIC( SimOUT, "RFC_OUT");
  int hdl_write = COM_get_function_handle( "RFC_OUT.write_dataitem");

  // Call Rocut with the window to write out the pane.
  int win_all = COM_get_dataitem_handle_const( (wname_fea+".all").c_str());  

  string fname_pre = out_pre + name() + "_fea_";

  int npanes, *paneIDs;
  COM_get_panes( wname_fea.c_str(), &npanes, &paneIDs);

  for ( int i=0; i<npanes; ++i) {
    std::ostringstream ostr;

    ostr << fname_pre;
    ostr.fill('0'); ostr.width(5);
    ostr << paneIDs[i] << ".hdf";

    COM_call_function( hdl_write, ostr.str().c_str(), &win_all, 
		       wname_fea.c_str(), "000", NULL, NULL, &paneIDs[i]);
  }
  COM_free_buffer( &paneIDs);

  // Unload Rocout
  COM_UNLOAD_MODULE_STATIC_DYNAMIC( SimOUT, "RFC_OUT");

  // Delete the temporary window
  COM_delete_window( wname_fea.c_str());
}

/** @addtogroup io Output Routines
 * @{
 */
/** Dump out the strong edges in Tecplot format into file <name>s1.plt. */
void RFC_Window_overlay::
dump_strong_edges( const std::vector<pair<float,HEdge> > &tstrong_edges,
		   const std::vector<pair<float,HEdge> > &rstrong_edges){
  int n=0;
  string buf = out_pre + name() + "s1.plt";

  ofstream os( buf.c_str());
  for (vector<pair<float,HEdge> >::const_iterator it=tstrong_edges.begin();
       it!=tstrong_edges.end(); ++it, ++n) {
    os << "GEOMETRY T=LINE3D C=GREEN"
       << " LT=0.4" << endl << 1 << endl
       << 2 << ' ' << it->second.origin_l().point()
       << ' ' << it->second.destination_l().point()
       << endl;
  }

  if ( !tstrong_edges.empty())
    std::cout << "\tStatistics: Theta-strong-edge angle: MAX =" 
	      << acos(tstrong_edges.front().first)*r2d
	      << " and MIN=" << acos(tstrong_edges.back().first)*r2d
	      << std::endl;

  for (vector<pair<float,HEdge> >::const_iterator it=rstrong_edges.begin();
       it!=rstrong_edges.end(); ++it, ++n) {
    os << "GEOMETRY T=LINE3D C=GREEN"
       << " LT=0.1" << endl << 1 << endl
       << 2 << ' ' << it->second.origin_l().point()
       << ' ' << it->second.destination_l().point()
       << endl;
  }
  if ( !rstrong_edges.empty())
    std::cout << " \tStatistics: r-strong-edge angle: MAX =" 
	      << acos(rstrong_edges.front().first)*r2d
	      << " and MIN=" << acos(rstrong_edges.back().first)*r2d
	      << std::endl;
}

/** @} end of io */

RFC_END_NAME_SPACE


