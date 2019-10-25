//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** \file assertion.C
 * This file contains the implementation for handling assertion failures.
 * @see com_assertion.h
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "com_assertion.h"

COM_BEGIN_NAME_SPACE

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// standard error handlers
// -----------------------
static void _standard_error_handler(const char *what, const char *expr,
                                    const char *file, int line,
                                    const char *msg) {
  std::cerr << "COM error: " << what << " violation!" << std::endl
            << "Expr: " << expr << std::endl
            << "File: " << file << std::endl
            << "Line: " << line << std::endl;
  if (msg != 0) std::cerr << "Explanation:" << msg << std::endl;
}

// default handler settings
// ------------------------
static Failure_function _error_handler = _standard_error_handler;
static Failure_behaviour _error_behaviour = ABORT;

// failure functions
// -----------------
void assertion_fail(const char *expr, const char *file, int line,
                    const char *msg) {
  extern void printStackBacktrace();

  (*_error_handler)("assertion", expr, file, line, msg);
  printStackBacktrace();  // Print stack backtrace
  switch (_error_behaviour) {
    case ABORT:
      abort();
    case EXIT:
      exit(1);  // EXIT_FAILURE
    case EXIT_WITH_SUCCESS:
      exit(0);  // EXIT_SUCCESS
    case CONTINUE:;
  }
}

Failure_behaviour set_error_behaviour(Failure_behaviour eb) {
  Failure_behaviour result = _error_behaviour;
  _error_behaviour = eb;
  return result;
}

// error handler set functions
// ---------------------------
Failure_function set_error_handler(Failure_function handler) {
  Failure_function result = _error_handler;
  _error_handler = handler;
  return (result);
}

#endif

COM_END_NAME_SPACE
