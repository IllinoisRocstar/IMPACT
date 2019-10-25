//
// Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information
//

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include "com.h"
#include "gtest/gtest.h"
#include "surfbasic.h"

COM_EXTERN_MODULE(SurfUtil)
COM_EXTERN_MODULE(SimOUT)

// Global variables used to pass arguments to the tests
char **ARGV;
int ARGC;

struct Four_tuple {
  Four_tuple() {}
  Four_tuple(int a, int b, int c, int d) : v1(a), v2(b), v3(c), v4(d) {}
  int v1, v2, v3, v4;
};

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}

TEST(SurfUtilTests, QuadNormals) {
  COM_init(&ARGC, &ARGV);

  /* if ( argc < 3) {
    std::cout << "Usage: " << argv[0] << " nrow ncol" << std::endl;
    exit(-1);
  } */

  COM_set_verbose(1);

  const int n_row = atoi(ARGV[1]), n_col = atoi(ARGV[2]);

  const double row_max = static_cast<double>(n_row - 1.) / 2.,
               col_max = static_cast<double>(n_col - 1.) / 2.;

  std::vector<SURF::Point_3<double> > pnts(n_row * n_col);
  std::vector<Four_tuple> elems((n_row - 1) * (n_col - 1));

  const double pi_by_2 = asin(1.);

  for (int i = 0; i < n_row; ++i)
    for (int j = 0; j < n_col; ++j) {
      double sin_1 = sin((i + row_max) / (n_row - 1) * pi_by_2);
      double sin_2 = sin((j + col_max) / (n_col - 1) * pi_by_2);
      double cos_1 = cos((i + row_max) / (n_row - 1) * pi_by_2);
      double cos_2 = cos((j + col_max) / (n_col - 1) * pi_by_2);

      pnts[i * n_col + j] =
          SURF::Point_3<double>(cos_1 * sin_2, cos_2, sin_1 * sin_2);
    }

  for (int i = 0; i < n_row - 1; ++i)
    for (int j = 0; j < n_col - 1; ++j)
      elems[i * (n_col - 1) + j] =
          Four_tuple(i * n_col + j + 1, (i + 1) * n_col + j + 1,
                     (i + 1) * n_col + j + 2, i * n_col + j + 2);
  std::vector<SURF::Vector_3<double> > nrms(pnts.size());

  std::cout << "Creating window \"quad1\"..." << std::endl;

  ASSERT_NO_THROW(COM_new_window("quad1"));
  ASSERT_NO_THROW(COM_new_dataitem("quad1.normals", 'n', COM_DOUBLE, 3, "m"));
  ASSERT_NO_THROW(COM_new_dataitem("quad1.evals", 'e', COM_DOUBLE, 3, "m"));
  ASSERT_NO_THROW(COM_new_dataitem("quad1.nvals", 'n', COM_DOUBLE, 3, "m"));

  ASSERT_NO_THROW(COM_set_size("quad1.nc", 1, pnts.size()));
  ASSERT_NO_THROW(COM_set_array("quad1.nc", 1, &pnts[0]));
  ASSERT_NO_THROW(COM_set_size("quad1.:q4:", 1, elems.size()));
  ASSERT_NO_THROW(COM_set_array("quad1.:q4:", 1, &elems[0]));

  ASSERT_NO_THROW(COM_set_array("quad1.normals", 1, &nrms[0]));
  ASSERT_NO_THROW(COM_resize_array("quad1.evals"));
  ASSERT_NO_THROW(COM_resize_array("quad1.nvals"));
  ASSERT_NO_THROW(COM_window_init_done("quad1"));

  double *evals_p;
  ASSERT_NO_THROW(COM_get_array("quad1.evals", 1, &(void *&)evals_p));
  for (int i = 0, n = 3 * elems.size(); i < n; ++i) evals_p[i] = 1.;

  int mesh = COM_get_dataitem_handle_const("quad1.mesh");
  int normals = COM_get_dataitem_handle("quad1.normals");
  int evals = COM_get_dataitem_handle("quad1.evals");
  int nvals = COM_get_dataitem_handle("quad1.nvals");
  ASSERT_NE(-1, mesh) << "Dataitem quad1.mesh was not found!\n";
  ASSERT_NE(-1, normals) << "Dataitem quad1.normals was not found!\n";
  ASSERT_NE(-1, evals) << "Dataitem quad1.evals was not found!\n";
  ASSERT_NE(-1, nvals) << "Dataitem quad1.nvals was not found!\n";

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfUtil, "SURF"));
  int SURF_init = COM_get_function_handle("SURF.initialize");
  ASSERT_NE(-1, SURF_init) << "Function SURF.initialize was not found!\n";
  ASSERT_NO_THROW(COM_call_function(SURF_init, &mesh));

  int SURF_normal = COM_get_function_handle("SURF.compute_normals");
  ASSERT_NE(-1, SURF_normal)
      << "Function SURF.compute_normals was not found!\n";
  ASSERT_NO_THROW(COM_call_function(SURF_normal, &mesh, &normals));

  int SURF_e2n = COM_get_function_handle("SURF.elements_to_nodes");
  ASSERT_NE(-1, SURF_e2n) << "Function SURF.elements_to_nodes was not found!\n";
  int scheme = SURF::E2N_ANGLE;
  ASSERT_NO_THROW(COM_call_function(SURF_e2n, &evals, &nvals, &mesh, &scheme));

  std::cout << "Output normals into file..." << std::endl;

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  int quad1_all = COM_get_dataitem_handle("quad1.all");
  ASSERT_NE(-1, OUT_write) << "Function OUT.write_dataitem was not found!\n";
  ASSERT_NE(-1, quad1_all) << "Dataitem quad1.all was not found!\n";

  char prefix1[100];
  sprintf(prefix1, "SurfUtilQuadNormals");
  ASSERT_NO_THROW(
      COM_call_function(OUT_write, prefix1, &quad1_all, "quad1", "000"));

  COM_delete_window("quad1");

  COM_finalize();
}
