//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "com.h"
#include "Tuple.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cassert>


COM_EXTERN_MODULE( SurfX);
COM_EXTERN_MODULE( SimOUT);

using namespace std;
USE_RFC_NAME_SPACE

int main(int argc, char **argv) {
  COM_init( &argc, &argv);

  if ( argc < 5) {
    std::cout << "Usage: " << argv[0] << " br, bc, gr, gc" << std::endl;
    exit(-1);
  }

  MPI_Comm comm = MPI_COMM_WORLD;
  const int comm_rank=0;
  const int comm_size=1;
 
  const int b_row=atoi(argv[1]), b_col=atoi(argv[2]);
  const int g_row=atoi(argv[3]), g_col=atoi(argv[4]);

  const double g_row_max=double(g_row-1.)/2., g_col_max=double(g_col-1.)/2.;
  const double b_row_max=double(b_row-1.)/2., b_col_max=double(b_col-1.)/2.;

  vector<Point_3>  g_pnts(g_row*g_col);
  vector<Four_tuple<int> >  g_elems((g_row-1)*(g_col-1));

  vector<Point_3>  b_pnts(b_row*b_col);
  vector<Four_tuple<int> >  b_elems((b_row-1)*(b_col-1));

  const double pi_by_2 = asin(1.);
  for (int i=0; i<g_row; ++i)
    for (int j=0; j<g_col; ++j) {
      double sin_1 = sin((i+g_row_max)/(g_row-1)*pi_by_2);
      double sin_2 = sin((j+g_col_max)/(g_col-1)*pi_by_2);
      double cos_1 = cos((i+g_row_max)/(g_row-1)*pi_by_2);
      double cos_2 = cos((j+g_col_max)/(g_col-1)*pi_by_2);

      g_pnts[i*g_col+j] = Point_3( cos_1*sin_2, cos_2, sin_1*sin_2);
    }

  for (int i=0; i<g_row-1; ++i)
    for (int j=0; j<g_col-1; ++j)
      g_elems[i*(g_col-1)+j] = Four_tuple<int>(i*g_col+j+1, (i+1)*g_col+j+1,
					       (i+1)*g_col+j+2, i*g_col+j+2);

  for (int i=0; i<b_row; ++i)
    for (int j=0; j<b_col; ++j) {
      double sin_1 = sin((i+b_row_max)/(b_row-1)*pi_by_2);
      double sin_2 = sin((j+b_col_max)/(b_col-1)*pi_by_2);
      double cos_1 = cos((i+b_row_max)/(b_row-1)*pi_by_2);
      double cos_2 = cos((j+b_col_max)/(b_col-1)*pi_by_2);


      b_pnts[i*b_col+j] = Point_3( cos_1*sin_2, cos_2, sin_1*sin_2);
    }

  for (int i=0; i<b_row-1; ++i)
    for (int j=0; j<b_col-1; ++j)
      b_elems[i*(b_col-1)+j] = Four_tuple<int>(i*b_col+j+1, (i+1)*b_col+j+1,
					       (i+1)*b_col+j+2, i*b_col+j+2);

  vector<Point_3>  g_coor = g_pnts, b_coor = b_pnts;

  COM_set_verbose( 1);
  COM_set_profiling( 1);

  COM_LOAD_MODULE_STATIC_DYNAMIC( SurfX, "RFC");

  COM_new_window("quad1");
  COM_new_dataitem("quad1.coor", 'n', COM_DOUBLE, 3, "m");
  if ( comm_rank==0) {
    COM_set_size( "quad1.nc", 1, b_pnts.size());
    COM_set_array( "quad1.nc", 1, &b_pnts[0]);
    COM_set_size( "quad1.:q4:", 1, b_elems.size());
    COM_set_array( "quad1.:q4:", 1, &b_elems[0]);
    COM_set_array( "quad1.coor", 1, &b_coor[0]);
  }
  COM_window_init_done( "quad1");

  COM_new_window("quad2");
  COM_new_dataitem("quad2.coor", 'n', COM_DOUBLE, 3, "m");
  if ( comm_rank==0) {
    COM_set_size( "quad2.nc", 1, g_pnts.size());
    COM_set_array( "quad2.nc", 1, &g_pnts[0]);
    COM_set_size( "quad2.:q4:", 1, g_elems.size());
    COM_set_array( "quad2.:q4:", 1, &g_elems[0]);
    COM_set_array("quad2.coor", 1, &g_coor[0]);
  }
  COM_window_init_done( "quad2");

  int quad1_mesh = COM_get_dataitem_handle( "quad1.mesh");
  int quad2_mesh = COM_get_dataitem_handle( "quad2.mesh");
  int RFC_clear = COM_get_function_handle( "RFC.clear_overlay");
  int RFC_write = COM_get_function_handle( "RFC.write_overlay");

  const char *format="CGNS";
  if ( comm_size==1) {
    int RFC_overlay = COM_get_function_handle( "RFC.overlay");

    COM_call_function( RFC_overlay, &quad1_mesh, &quad2_mesh);

    COM_call_function( RFC_write, &quad1_mesh, &quad2_mesh,
		       "quad1", "quad2", format);
    COM_call_function( RFC_clear, "quad1", "quad2");
  }

  int RFC_read = COM_get_function_handle( "RFC.read_overlay");
  COM_call_function( RFC_read, &quad1_mesh, &quad2_mesh, &comm,
		     "quad1", "quad2", format);

  if ( argc > 5) {
    char prefix1[100], prefix2[100];

    std::sprintf(prefix1, "quad1_coor%d", comm_rank);
    std::sprintf(prefix2, "quad2_coor%d", comm_rank);

    COM_LOAD_MODULE_STATIC_DYNAMIC( SimOUT, "OUT");
    int OUT_write = COM_get_function_handle( "OUT.write_dataitem");
    int OUT_set = COM_get_function_handle( "OUT.set_option");
    int quad1_coor = COM_get_dataitem_handle( "quad1.coor");
    int quad2_coor = COM_get_dataitem_handle( "quad2.coor");

    COM_call_function( OUT_write, prefix1, &quad1_coor, "quad1", "000");
    COM_call_function( OUT_write, prefix2, &quad2_coor, "quad2", "000");

    int RFC_transfer = COM_get_function_handle( "RFC.least_squares_transfer");
    COM_call_function( RFC_transfer, &quad1_coor, &quad2_coor);
    COM_call_function( RFC_transfer, &quad2_coor, &quad1_coor);

    COM_call_function( OUT_set, "mode", "a");
    COM_call_function( OUT_write, prefix1, &quad1_coor, "quad1", "001");
    COM_call_function( OUT_write, prefix2, &quad2_coor, "quad2", "001");

    COM_call_function( RFC_clear, "quad1", "quad2");
    // COM_UNLOAD_MODULE_STATIC_DYNAMIC( Rocout, "OUT");
  }

  COM_delete_window( "quad1");
  COM_delete_window( "quad2");

  COM_print_profile( "", "");

  COM_finalize();
}


