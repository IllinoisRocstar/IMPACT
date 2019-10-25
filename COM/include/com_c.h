//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

/** \file com_c.h
 * This file contains the C wrapper for COM_base.
 * All the functions defined here directly correpond to the public
 * member functions of COM_base.
 * @see com_c.C, com_c++.hpp
 */

#ifndef __COM_C_H__
#define __COM_C_H__

#include <string.h>
#include "com_basic.h"
#include "commpi.h"

/* C API of COM. */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOAD_MODULE_WRAPPER(mixed, lower, UPPER)                           \
                                                                           \
  void mixed##_load_module(const char *mod) {                              \
    extern void COM_F_FUNC2(lower##_load_module, UPPER##_LOAD_MODULE)(     \
        const char *mod, long int len);                                    \
                                                                           \
    long int len = strlen(mod);                                            \
    COM_F_FUNC2(lower##_load_module, UPPER##_LOAD_MODULE)                  \
    (mod, len);                                                            \
  }                                                                        \
                                                                           \
  void mixed##_unload_module(const char *mod) {                            \
    extern void COM_F_FUNC2(lower##_unload_module, UPPER##_UNLOAD_MODULE)( \
        const char *mod, long int len);                                    \
                                                                           \
    long int len = strlen(mod);                                            \
    COM_F_FUNC2(lower##_unload_module, UPPER##_UNLOAD_MODULE)              \
    (mod, len);                                                            \
  }

/** \name Initialization and finalization
 * \{
 */
void COM_init(int *argc, char ***argv);
void COM_finalize();
int COM_initialized();
void COM_abort(int ierr);

void COM_set_default_communicator(MPI_Comm comm);
MPI_Comm COM_get_default_communicator();
/**\}*/

/** \name Module management
 *  \{
 */
void COM_load_module(const char *libname, const char *winname);
void COM_unload_module(const char *libname, const char *winname);

/**\}*/

/** \name Window and pane management
 * \{
 */
/* Creation and deletion of a window. str specifies a window's name.
 * The length of a string can be omitted for C/C++ code. For Fortran,
 * the lengths are passed automatically by the compilers.
 * (Same for all other routines.) */
void COM_new_window(const char *w_str, MPI_Comm c);
void COM_delete_window(const char *str);

/* Deregistering a pane from Roccom
 *    w_str is a window's name.
 *    pane_id is the id of a pane which was created before. */
void COM_delete_pane(const char *w_str, const int pane_id);

/* This marks the end of the initialization of a window,
 *    w_str is a window's name. */
void COM_window_init_done(const char *w_str, int pane_changed);

/**\}*/

/** \name DataItem management
 * \{
 */
/** Registering an dataitem type.
 *    wa_str is the dataitem's name in the format "window.dataitem".
 *    loc is the code for the location of the data, which can be
 *             'w' for window dataitem,
 *             'p' for pane dataitem,
 *             'n' for node dataitem, or
 *             'e' for element dataitem.
 *    type is the data type of the dataitem. For C/C++, it can be
 *             COM_CHAR, COM_UNSIGNED_CHAR, COM_BYTE, COM_SHORT,
 *             COM_UNSIGNED_SHORT, COM_INT, COM_UNSIGNED, COM_LONG,
 *             COM_UNSIGNED_LONG, COM_FLOAT0, COM_DOUBLE1, COM_LONG_DOUBLE,
 *             COM_LONG_LONG_INT, or a derived data type.
 *         For Fortran, it can be
 *             COM_CHARACTER, COM_INTEGER, COM_LOGICAL, COM_REAL,
 *             COM_DOUBLE_PRECISION, COM_COMPLEX, COM_DOUBLE_COMPLEX.
 *    size is the length of the dataitem in the given data type */
void COM_new_dataitem(const char *wa_str, const char loc, const int type,
                      int ncomp, const char *unit);

/** Delete an existing dataitem. */
void COM_delete_dataitem(const char *wa_str);

#ifndef C_ONLY

/** Set sizes of for a specific dataitem.
 */
void COM_set_size(const char *wa_str, int pane_id, int size, int ng);

/** Associates an array with an dataitem for a specific pane.
 */
void COM_set_array(const char *wa_str, int pane_id, void *addr, int strd,
                   int cap);

/** Set the lower and upper bounds of an dataitem. */
void COM_set_bounds(const char *wa_str, int pane_id, const void *lbound,
                    const void *ubound);

/** Allocate space for an dataitem on a specific pane and return the
 *  address by setting addr. Allocate for all panes if pane-id is 0,
 *  in which case, do not set addr. */
void COM_allocate_array(const char *wa_str, int pane_id, void **addr, int strd,
                        int cap);

/** Resize an dataitem on a specific pane and return the
 *  address by setting addr. Resize for all panes if pane-id is 0,
 *  in which case, do not set addr. The difference between resize and
 *  allocate is that resize will reallocate memory only if the current
 *  array cannot accomodate the requested capacity. */
void COM_resize_array(const char *wa_str, int pane_id, void **addr, int strd,
                      int cap);

/** Append an array to the end of the dataitem on a specific pane and
 *  return the new address by setting addr.  */
void COM_append_array(const char *wa_str, int pane_id, const void *val,
                      int v_strd, int v_size);

/** Use the subset of panes of another window
 * of which the given pane dataitem has value val. */
void COM_use_dataitem(const char *wname, const char *attr, int with_ghost,
                      const char *ptnname, int val);

/** Clone the subset of panes of another window
 *  of which the given pane dataitem has value val. */
void COM_clone_dataitem(const char *wname, const char *attr, int with_ghost,
                        const char *ptnname, int val);

/** Copy an dataitem onto another. */
void COM_copy_dataitem(const char *wname, const char *attr, int with_ghost,
                       const char *ptnname, int val);

void COM_copy_dataitem_handles(int trg_hdl, int src_hdl, int with_ghost,
                               int ptn_hdl, int val);

/** Deallocate space for an dataitem in a pane, asuming the memory was
 *  allocated allocate_mesh or allocate_dataitem. */
void COM_deallocate_array(const char *wa_str, const int pid);

/** Get the sizes of an dataitem. The opposite of set_size. */
void COM_get_size(const char *wa_str, int pane_id, int *size, int *ng);

/** Get the sizes of an dataitem. The opposite of set_size. */
void COM_get_dataitem(const char *wa_str, char *loc, int *type, int *ncomp,
                      char *unit, int n);
#endif

/** Get the address for an dataitem on a specific pane. */
void COM_get_array(const char *wa_str, int pane_id, void **addr, int *strd,
                   int *cap);

/** Copy an array from an dataitem on a specific pane into a given buffer.*/
void COM_copy_array(const char *wa_str, int pane_id, void *val, int v_strd,
                    int v_size, int offset);

/** Get the lower and upper bounds of an dataitem. */
void COM_get_bounds(const char *wa_str, int pane_id, void *lbound,
                    void *ubound);

/** Check the lower and upper bounds of an dataitem for a specific pane. */
int COM_check_bounds(const char *wa_str, int pane_id);

/** Information retrieval
 * Get the information about an dataitem. The opposite of new_dataitem. */
void COM_get_dataitem(const char *wa_str, char *loc, int *type, int *ncomp,
                      char *unit, int n);
/**\}*/

/** \name Information retrieval
 * \{
 */
void COM_get_communicator(const char *wname, MPI_Comm *comm);

void COM_get_panes(const char *wname, int *npanes, int **pane_ids, int rank);

void COM_get_dataitems(const char *wname, int *na, char **names);

void COM_get_connectivities(const char *wname, int pane_id, int *nc,
                            char **names);

void COM_get_parent(const char *waname, int pane_id, char **parent);

void COM_free_buffer(char **buf);

int COM_get_window_handle(const char *wname);
int COM_get_dataitem_handle(const char *waname);
int COM_get_dataitem_handle_const(const char *waname);
int COM_get_function_handle(const char *wfname);

int COM_get_status(const char *waname, const int pane_id);

/**\}*/

/** \name Function management
 *  \{
 */

/* Associate a function pointer with a name.
 *   The function can take up to seven arguments of void* type
 *   and return no value. */
void COM_set_function(const char *wf_str, Func_ptr func, const char *intents,
                      const COM_Type *types);

void COM_set_member_function(const char *wf_str, Func_ptr func,
                             const char *wa_str, const char *intents,
                             const COM_Type *types);

/* Invoke a function by a name registered to Roccom. */
void COM_call_function(const int wf, int argc, ...);

/* Non-blocking invocation of a function by a name registered to Roccom. */
void COM_icall_function(const int wf, int argc, ...);

void COM_wait(const int id);
int COM_test(const int id);

/**\}*/

/** \name Tracing and profiling tools
 *  \{
 */
void COM_set_verbose(int i);
void COM_set_profiling(int i);
void COM_set_profiling_barrier(int hdl, MPI_Comm comm);
void COM_print_profile(const char *fname, const char *header);
/*\}*/

/** \name Miscellaneous
 *  \{
 */
int COM_get_sizeof(const COM_Type type, int c);
int COM_compatible_types(COM_Type type1, COM_Type type2);

int COM_get_error_code();
/*\}*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __COM_C_H__ */
