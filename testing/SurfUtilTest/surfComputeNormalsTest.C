//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include "com.h"
#include "gtest/gtest.h"
#include "surfbasic.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

// ==== Routines for creating mesh information
void init_structured_mesh(double coors[][3], int nrow, int ncol, int rank,
                          int nproc, int check = 1) {
  // consider the processors as a 2*(nproc/2) grid
  int proc_col = nproc;
  if (nproc % 2 == 0) {
    proc_col = nproc / 2;
  } else {
    proc_col = nproc;
  }
  int row = rank / proc_col, col = rank % proc_col;

  const double width = 300. / (nrow - 1), length = 300. / (ncol - 1);

  double x0 = col * 300, y0 = row * 300, y1 = y0;
  for (int i = 0; i < nrow; ++i) {
    double x1 = x0;
    for (int j = 0; j < ncol; ++j) {
      if (check < 0) {
        assert(coors[i * ncol + j][0] == x1 && coors[i * ncol + j][1] == y1 &&
               coors[i * ncol + j][2] == 0.5);
      } else {
        coors[i * ncol + j][0] = x1 * check;
        coors[i * ncol + j][1] = y1 * check;
        coors[i * ncol + j][2] = 0.5;
      }
      x1 += length;
    }
    y1 += width;
  }
}

void init_unstructure_mesh(double coors[][3], int elmts[][4], int nrow,
                           int ncol, int rank, int nproc, int check = 1) {
  // consider the processors as a 2*(nproc/2) grid
  int proc_col = nproc;
  if (nproc % 2 == 0) {
    proc_col = nproc / 2;
  } else {
    proc_col = nproc;
  }

  int row = rank / proc_col, col = rank % proc_col;

  const double width = 300. / (nrow - 1), length = 300. / (ncol - 1);

  double x0 = col * 300, y0 = row * 300, y1 = y0;
  for (int i = 0; i < nrow; ++i) {
    double x1 = x0;
    for (int j = 0; j < ncol; ++j) {
      if (check < 0) {
        assert(coors[i * ncol + j][0] == x1 && coors[i * ncol + j][1] == y1 &&
               coors[i * ncol + j][2] == 0.0);
      } else {
        coors[i * ncol + j][0] = x1 * check;
        coors[i * ncol + j][1] = y1 * check;
        coors[i * ncol + j][2] = 0.0;
      }
      x1 += length;
    }
    y1 += width;
  }

  // computes elmts
  for (int i = 0; i < nrow - 1; ++i) {
    for (int j = 0; j < ncol - 1; ++j) {
      if (check < 0) {
        assert(elmts[i * (ncol - 1) + j][0] == i * ncol + j + 1 &&
               elmts[i * (ncol - 1) + j][1] == i * ncol + j + ncol + 1 &&
               elmts[i * (ncol - 1) + j][2] == i * ncol + j + ncol + 2 &&
               elmts[i * (ncol - 1) + j][3] == i * ncol + j + 2);
      } else {
        elmts[i * (ncol - 1) + j][0] = check * i * ncol + j + 1;
        elmts[i * (ncol - 1) + j][1] = check * i * ncol + j + ncol + 1;
        elmts[i * (ncol - 1) + j][2] = check * i * ncol + j + ncol + 2;
        elmts[i * (ncol - 1) + j][3] = check * i * ncol + j + 2;
      }
    }
  }
}

int get_comm_rank(MPI_Comm comm) {
  int rank;
#ifndef NDEBUG
  int ierr =
#endif
      MPI_Comm_rank(comm, &rank);
  COM_assertion(ierr == 0);
  return rank;
}

int get_comm_size(MPI_Comm comm) {
  int size;
#ifndef NDEBUG
  int ierr =
#endif
      MPI_Comm_size(comm, &size);
  COM_assertion(ierr == 0);
  return size;
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}

TEST(SurfParallelTest, ComputeNormals) {
  const int nproc = 4;
  const int nrow = 40, ncol = 30;
  const int num_nodes = nrow * ncol;
  const int num_elmts = (nrow - 1) * (ncol - 1);

  double coors_s[nproc][num_nodes][3];
  int elmts[nproc][num_elmts][4];

  MPI_Init(&ARGC, &ARGV);
  COM_init(&ARGC, &ARGV);

  MPI_Comm comm = MPI_COMM_WORLD;
  const int comm_rank = get_comm_rank(comm);
  const int comm_size = get_comm_size(comm);

  int vb = (ARGC > 1) ? atoi(ARGV[1]) : 1;
  if (comm_rank == 0) COM_set_verbose(vb);

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));

  if (comm_rank == 0) std::cout << "Creating window \"unstr\"" << std::endl;
  ASSERT_NO_THROW(COM_new_window("unstr"));
  ASSERT_NO_THROW(COM_new_dataitem("unstr.normals", 'n', COM_DOUBLE, 3, "m"));

  for (int pid = 0; pid < nproc; ++pid)
    if (pid % comm_size == comm_rank) {
      init_unstructure_mesh(coors_s[pid], elmts[pid], nrow, ncol, pid, nproc);
      ASSERT_NO_THROW(COM_set_size("unstr.nc", pid + 1, num_nodes));
      ASSERT_NO_THROW(COM_set_array("unstr.nc", pid + 1, &coors_s[pid][0][0]));
      ASSERT_NO_THROW(COM_set_size("unstr.:q4:", pid + 1, num_elmts));
      ASSERT_NO_THROW(COM_set_array("unstr.:q4:", pid + 1, &elmts[pid][0][0]));
    }
  ASSERT_NO_THROW(COM_resize_array("unstr.data"));
  ASSERT_NO_THROW(COM_window_init_done("unstr"));

  int mesh = COM_get_dataitem_handle_const("unstr.mesh");
  ASSERT_NE(-1, mesh) << "const Dataitem unstr.mesh was not found!\n";
  int normals = COM_get_dataitem_handle("unstr.normals");
  ASSERT_NE(-1, normals) << "Dataitem unstr normals was not found!\n";

  ASSERT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfUtil, "SURF"));
  int SURF_init = COM_get_function_handle("SURF.initialize");
  ASSERT_NE(-1, SURF_init) << "Function SURF.initialize was not found!\n";
  ASSERT_NO_THROW(COM_call_function(SURF_init, &mesh));

  int SURF_normal = COM_get_function_handle("SURF.compute_normals");
  ASSERT_NE(-1, SURF_normal)
      << "Function SURF.compute_normals was not found!\n";
  ASSERT_NO_THROW(COM_call_function(SURF_normal, &mesh, &normals));

  if (comm_rank == 0) std::cout << "Output normals into file..." << std::endl;
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  int unstr_all = COM_get_dataitem_handle("unstr.all");
  ASSERT_NE(-1, OUT_write) << "Function OUT.write_dataitem was not found!\n";
  ASSERT_NE(-1, unstr_all) << "Dataitem unstr.all was not found!\n";

  ASSERT_NO_THROW(COM_call_function(OUT_write, "SurfUtilParallelUnstruc_all",
                                    &unstr_all, "unstr", "000"));
  ASSERT_NO_THROW(COM_delete_window("unstr"));

  // Test structured mesh
  double coors_f[nproc][nrow * ncol][3];

  if (comm_rank == 0) std::cout << "Creating window \"str\"" << std::endl;
  ASSERT_NO_THROW(COM_new_window("str"));
  ASSERT_NO_THROW(COM_new_dataitem("str.normals", 'n', COM_DOUBLE, 3, "m/s"));

  for (int pid = 0; pid < nproc; ++pid)
    if (pid % comm_size == comm_rank) {
      init_structured_mesh(coors_f[pid], nrow, ncol, pid, nproc);

      ASSERT_NO_THROW(COM_set_size("str.nc", pid + 1, nrow * ncol));
      ASSERT_NO_THROW(COM_set_array("str.nc", pid + 1, &coors_f[pid][0][0]));
      int dims[2] = {ncol, nrow};  // Use Fortran convention
      ASSERT_NO_THROW(COM_set_array("str.:st2:", pid + 1, &dims[0]));
    }

  ASSERT_NO_THROW(COM_resize_array("str.data"));
  ASSERT_NO_THROW(COM_window_init_done("str"));

  mesh = COM_get_dataitem_handle("str.mesh");
  normals = COM_get_dataitem_handle("str.normals");
  ASSERT_NE(-1, mesh) << "Dataiem str.mesh was not found!\n";
  ASSERT_NE(-1, normals) << "Dataitem str.normals was not found!\n";

  ASSERT_NO_THROW(COM_call_function(SURF_init, &mesh));
  ASSERT_NO_THROW(COM_call_function(SURF_normal, &mesh, &normals));

  if (comm_rank == 0) std::cout << "Output normals into file..." << std::endl;
  int str_all = COM_get_dataitem_handle("str.all");
  ASSERT_NE(-1, str_all) << "Dataitem str.all was not found!\n";

  ASSERT_NO_THROW(COM_call_function(OUT_write, "SurfUtilParallelStruc_all",
                                    &str_all, "str", "000"));
  ASSERT_NO_THROW(COM_delete_window("str"));

  // COM_unload_module("Rocout");   // Unload Rocout

  COM_finalize();
  MPI_Finalize();
}
