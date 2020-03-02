#include "RocBlas.h"
#include "com.h"

COM_EXTERN_MODULE(Simpal)

int RocBlas::copy_scalar = 0;
int RocBlas::copy = 0;
int RocBlas::add = 0;
int RocBlas::sub = 0;
int RocBlas::div_scalar = 0;
int RocBlas::limit1 = 0;
int RocBlas::sub_scalar = 0;
int RocBlas::axpy_scalar = 0;
int RocBlas::mul = 0;
int RocBlas::div = 0;
int RocBlas::neg = 0;
int RocBlas::axpy = 0;
int RocBlas::nrm2 = 0;
int RocBlas::mul_scalar = 0;
int RocBlas::max_scalar_MPI = 0;
int RocBlas::min_scalar_MPI = 0;
int RocBlas::sum_scalar_MPI = 0;
int RocBlas::nrm2_scalar_MPI = 0;
int RocBlas::maxof_scalar = 0;

void RocBlas::initHandles() {
  copy_scalar = COM_get_function_handle("BLAS.copy_scalar");
  sub_scalar = COM_get_function_handle("BLAS.sub_scalar");
  copy = COM_get_function_handle("BLAS.copy");
  add = COM_get_function_handle("BLAS.add");
  sub = COM_get_function_handle("BLAS.sub");
  axpy = COM_get_function_handle("BLAS.axpy");
  limit1 = COM_get_function_handle("BLAS.limit1");
  mul = COM_get_function_handle("BLAS.mul");
  div = COM_get_function_handle("BLAS.div");
  neg = COM_get_function_handle("BLAS.neg");
  nrm2 = COM_get_function_handle("BLAS.nrm2");
  div_scalar = COM_get_function_handle("BLAS.div_scalar");
  axpy_scalar = COM_get_function_handle("BLAS.axpy_scalar");
  mul_scalar = COM_get_function_handle("BLAS.mul_scalar");
  max_scalar_MPI = COM_get_function_handle("BLAS.max_scalar_MPI");
  min_scalar_MPI = COM_get_function_handle("BLAS.min_scalar_MPI");
  sum_scalar_MPI = COM_get_function_handle("BLAS.sum_scalar_MPI");
  nrm2_scalar_MPI = COM_get_function_handle("BLAS.nrm2_scalar_MPI");
  maxof_scalar = COM_get_function_handle("BLAS.maxof_scalar");
}

void RocBlas::init() {
  COM_LOAD_MODULE_STATIC_DYNAMIC(Simpal, "BLAS");
  initHandles();
}
