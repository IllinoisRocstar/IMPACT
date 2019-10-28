//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** \file com_assertion.h
 *   This file contains a set of routines for error assertion.
 * @see assertion.C
 */

#ifndef __COM_ASSERTION_H
#define __COM_ASSERTION_H

#include "com_basic.h"

// COM_BEGIN_NAME_SPACE
namespace COM {
///  Behavior of failures.
enum Failure_behaviour { ABORT, EXIT, EXIT_WITH_SUCCESS, CONTINUE };

/// Function type for error handlers.
typedef void (*Failure_function)(const char *, const char *, const char *, int,
                                 const char *);
/// Default error handler.
void assertion_fail(const char *, const char *, int, const char *);

/// Controls the behavior when an assertion fails.
Failure_behaviour set_error_behaviour(Failure_behaviour eb);
/// Sets the handler for assertion-failures.
Failure_function set_error_handler(Failure_function handler);

/** Error checking utility similar to the assert macro of the C language.
 *  If NDEBUG is not defined, then it will invoke the error handler set by
 *  set_error_handler, and then perform the action according to the
 *  Failure_behaviour set by set_error_behaviour. The default error handler
 *  is to print the assertion expression, file name and line number, and
 *  the default error behavior is to abort. If NDEBUG is defined, this
 *  macro has no effect.
 */
#ifndef NDEBUG
#define COM_assertion(EX) \
  ((EX) ? ((void)0) : ::COM::assertion_fail(#EX, __FILE__, __LINE__, 0))
#define COM_assertion_msg(EX, msg) \
  ((EX) ? ((void)0) : ::COM::assertion_fail(#EX, __FILE__, __LINE__, msg))
#else
#define COM_assertion(EX) ((void)0)
#define COM_assertion_msg(EX, msg) ((void)0)
#endif

}  // namespace COM

#endif  // __COM_ASSERTION_H
