//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <cassert>
#include "commpi.h"
#ifdef TRACEBACK
#include <execinfo.h> /* for backtrace (GNU glibc header) */
#endif
#include "rfc_basic.h"

RFC_BEGIN_NAME_SPACE

void printStackBacktrace() {
#ifdef TRACEBACK
  const int max_stack = 64;
  void* stackPtrs[max_stack];

  int levels = backtrace(stackPtrs, max_stack);
  char** symbols = backtrace_symbols(stackPtrs, levels);

  const int nSkip = 2;
  std::printf("\nStack Backtrace:\n");
  for (int i = nSkip; i < levels; i++) {
    std::printf(" [%d] %s\n", i - nSkip, symbols[i]);
  }
#endif
}

// standard error handlers
static void _standard_error_handler(const char* what, const char* expr,
                                    const char* file, int line,
                                    const char* msg) {
  std::cerr << "Rocface error: " << what << " violation!" << std::endl
            << "Expr: " << expr << std::endl
            << "File: " << file << std::endl
            << "Line: " << line << std::endl;
  if (msg != 0) std::cerr << "Explanation:" << msg << std::endl;
}

// failure functions
void assertion_fail(const char* expr, const char* file, int line,
                    const char* msg) {
  _standard_error_handler("assertion", expr, file, line, msg);
  printStackBacktrace();

  if (COMMPI_Initialized()) MPI_Abort(MPI_COMM_WORLD, -1);
  abort();
}

void precondition_fail(const char* expr, const char* file, int line,
                       const char* msg) {
  _standard_error_handler("precondition", expr, file, line, msg);
  printStackBacktrace();

  if (COMMPI_Initialized()) MPI_Abort(MPI_COMM_WORLD, -1);
  abort();
}

RFC_END_NAME_SPACE
