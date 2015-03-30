//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

/** \file com_c++.hpp
 * Contains the wrapper routines for C++ binding of COM API.
 * All the functions are inlined and hence have smaller runtime
 * overhead compared with the C binding.
 * @see com_c.h
 */

#ifndef __COM_CPP_H__
#define __COM_CPP_H__

#ifndef __cplusplus
#error  This header file is for C++. Include "com_c.h" instead for C.
#endif

#include "COM_base.hpp"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef C_ONLY
#define inline extern "C" /* Disable inlining */
#endif

//==============================================================
// C++ API
// Initialization and finalization
//=============================================================
inline void COM_init( int *argc, char ***argv) 
{ COM::COM_base::init( argc, argv); }
inline void COM_finalize() 
{ COM::COM_base::finalize(); }

inline int COM_initialized()
{ return COM::COM_base::initialized(); }
inline void COM_abort( int ierr) 
{ COM::COM_base::abort( ierr); }

inline void COM_set_default_communicator( MPI_Comm comm)
{ COM::COM_base::set_default_communicator( comm); }
inline MPI_Comm COM_get_default_communicator()
{ return COM::COM_base::get_default_communicator(); }

//================================================================
//================== Load and unload modules =====================
//================================================================
inline void COM_load_module( const char *libname, const char *winname) 
{ COM_get_com()->load_module( libname, winname); }

inline void COM_unload_module( const char *libname, const char *winname=NULL) 
{ COM_get_com()->unload_module( libname, winname?winname:""); }

inline void COM_close_module( const char *libname, const char *winname=NULL) 
{ COM_get_com()->unload_module( libname, winname?winname:"", 0); }

// ======== Basic API for registering mesh and field data
// Creation and deletion of a window. str specifies a window's name.
inline void COM_new_window( const char *wname, MPI_Comm c=MPI_COMM_NULL)
{ COM_get_com()->new_window(wname, c); }

#if !defined(C_ONLY)
inline void COM_new_window( const std::string wname, MPI_Comm c=MPI_COMM_NULL)
{ COM_get_com()->new_window(wname.c_str(), c); }
#endif

inline void COM_delete_window( const char *wname)
{ COM_get_com()->delete_window(wname); }

#if !defined(C_ONLY)
inline void COM_delete_window( const std::string wname)
{ COM_get_com()->delete_window(wname.c_str()); }
#endif

inline void COM_window_init_done( const char *w_str, int pane_changed=true)
{ COM_get_com()->window_init_done( w_str, pane_changed); }

#if !defined(C_ONLY)
inline void COM_window_init_done( const std::string w_str, int pane_changed=true)
{ COM_get_com()->window_init_done( w_str.c_str(), pane_changed); }
#endif

inline void COM_delete_pane( const char *str,  int pid)
{ COM_get_com()->delete_pane( str, pid) ; }

#if !defined(C_ONLY)
inline void COM_delete_pane( const std::string str,  int pid)
{ COM_get_com()->delete_pane( str.c_str(), pid) ; }
#endif

inline void COM_new_dataitem( const char *wa_str, const char loc,
			       const int type, int ncomp, const char *unit)
{ COM_get_com()->new_dataitem( wa_str, loc, type, ncomp, unit); }

#if !defined(C_ONLY)
inline void COM_new_dataitem( const std::string wa_str, const char loc,
			       const int type, int ncomp, const char *unit)
{ COM_get_com()->new_dataitem( wa_str.c_str(), loc, type, ncomp, unit); }
#endif

inline void COM_delete_dataitem( const char *wa_str)
{ COM_get_com()->delete_dataitem( wa_str); }

#if !defined(C_ONLY)
inline void COM_delete_dataitem( const std::string wa_str)
{ COM_get_com()->delete_dataitem( wa_str.c_str()); }
#endif

inline void COM_set_size( const char *wa_str, int pane_id, int size, int ng=0)
{ COM_get_com()->set_size( wa_str, pane_id, size, ng); }

#if !defined(C_ONLY)
inline void COM_set_size( const std::string wa_str, int pane_id, int size, int ng=0)
{ COM_get_com()->set_size( wa_str.c_str(), pane_id, size, ng); }

template <class Type>
inline void COM_set_object( const char *wa_str, int pane_id, Type *addr) {
  COM_assertion_msg( addr && COM_Object(*addr).validate_object()==0, 
		     "Invalid casting");

  COM_get_com()->set_object( wa_str, pane_id, addr, (COM_Object*)(addr)); 
}

template <class Type>
inline void COM_get_object( const char *wa_str, int pane_id, Type **addr) 
{ COM_get_com()->get_object( wa_str, pane_id, (void**)addr); }
#endif

inline void COM_set_array( const char *wa_str, int pane_id, 
			   void *addr, int strd=0, int cap=0)
{ COM_get_com()->set_array( wa_str, pane_id, addr, strd, cap); }

inline void COM_set_array_const( const char *wa_str, int pane_id, 
				 const void *addr, int strd=0, int cap=0)
{ COM_get_com()->set_array( wa_str, pane_id, 
			       const_cast<void*>(addr), strd, cap, true); }

#ifndef C_ONLY
inline void COM_set_array( const std::string wa_str, int pane_id, 
			   void *addr, int strd=0, int cap=0)
{ COM_get_com()->set_array( wa_str.c_str(), pane_id, addr, strd, cap); }

inline void COM_set_array_const( const std::string wa_str, int pane_id, 
				 const void *addr, int strd=0, int cap=0)
{ COM_get_com()->set_array( wa_str.c_str(), pane_id, 
			       const_cast<void*>(addr), strd, cap, true); }
#endif

inline void COM_set_bounds( const char *wa_str, int pane_id, 
			    const void *lbound, const void *ubound)
{ COM_get_com()->set_bounds( wa_str, pane_id, lbound, ubound); }

#ifndef C_ONLY
inline void COM_set_bounds( const std::string wa_str, int pane_id, 
			    const void *lbound, const void *ubound)
{ COM_get_com()->set_bounds( wa_str.c_str(), pane_id, lbound, ubound); }

#define COM_set_bounds_prototype(type) \
inline void COM_set_bounds( const char *wa_str, int pane_id, \
			    type lbound, type ubound) \
{ COM_get_com()->set_bounds( wa_str, pane_id, &lbound, &ubound); }

COM_set_bounds_prototype(char)
COM_set_bounds_prototype(int)
COM_set_bounds_prototype(float)
COM_set_bounds_prototype(double)
#endif

inline void COM_allocate_array( const char *wa_str, int pane_id=0, 
				void **addr=NULL, int strd=0, int cap=0)
{ COM_get_com()->allocate_array( wa_str, pane_id, addr, strd, cap); }

inline void COM_resize_array( const char *wa_str, int pane_id=0, 
			      void **addr=NULL, int strd=-1, int cap=0)
{ COM_get_com()->resize_array( wa_str, pane_id, addr, strd, cap); }

#ifndef C_ONLY
inline void COM_allocate_array( const std::string wa_str, int pane_id=0, 
				void **addr=NULL, int strd=0, int cap=0)
{ COM_get_com()->allocate_array( wa_str.c_str(), pane_id, addr, strd, cap); }

inline void COM_resize_array( const std::string wa_str, int pane_id=0, 
			      void **addr=NULL, int strd=-1, int cap=0)
{ COM_get_com()->resize_array( wa_str.c_str(), pane_id, addr, strd, cap); }
#endif

inline void COM_append_array( const char *wa_str, int pane_id,
			      const void *val, int v_strd, int v_size)
{ COM_get_com()->append_array( wa_str, pane_id, val, v_strd, v_size); }

#ifndef C_ONLY
inline void COM_append_array( const std::string wa_str, int pane_id,
			      const void *val, int v_strd, int v_size)
{ COM_get_com()->append_array( wa_str.c_str(), pane_id, val, v_strd, v_size); }
#endif

inline void COM_use_dataitem( const char *wname, const char *attr,
			       int wg=1, const char *ptnname=0, int val=0) 
{ COM_get_com()->use_dataitem(wname, attr, wg, ptnname, val); }

#ifndef C_ONLY
inline void COM_use_dataitem( const std::string wname, const std::string attr,
			       int wg=1, const char *ptnname=0, int val=0) 
{ COM_get_com()->use_dataitem(wname.c_str(), attr.c_str(), wg, ptnname, val); }
#endif

inline void COM_clone_dataitem( const char *wname, const char *attr,
				 int wg=1, const char *ptnname=0, int val=0) 
{ COM_get_com()->clone_dataitem(wname, attr, wg, ptnname, val); }

#ifndef C_ONLY
inline void COM_clone_dataitem( const std::string wname, const std::string attr,
				 int wg=1, const char *ptnname=0, int val=0) 
{ COM_get_com()->clone_dataitem(wname.c_str(), attr.c_str(), wg, ptnname, val); }
#endif

inline void COM_copy_dataitem( const char *wname, const char *attr,
				int wg=1, const char *ptnname=0, int val=0) 
{ COM_get_com()->copy_dataitem(wname, attr, wg, ptnname, val); }

#ifndef C_ONLY
inline void COM_copy_dataitem( const std::string wname, const std::string attr,
				int wg=1, const char *ptnname=0, int val=0) 
{ COM_get_com()->copy_dataitem(wname.c_str(), attr.c_str(), wg, ptnname, val); }

inline void COM_copy_dataitem
( int trg_hdl, int src_hdl, int wg=1, int ptn_hdl=0, int val=0) 
{ COM_get_com()->copy_dataitem(trg_hdl, src_hdl, wg, ptn_hdl, val); }
#else
inline void COM_copy_dataitem_handles
( int trg_hdl, int src_hdl, int wg, int ptn_hdl, int val) 
{ COM_get_com()->copy_dataitem(trg_hdl, src_hdl, wg, ptn_hdl, val); }
#endif

inline void COM_deallocate_array( const char *wa_str, const int pid=0) 
{ COM_get_com()->deallocate_array(wa_str, pid); }

#ifndef C_ONLY
inline void COM_deallocate_array( const std::string wa_str, const int pid=0) 
{ COM_get_com()->deallocate_array(wa_str.c_str(), pid); }

inline void COM_get_dataitem( const std::string wa_str, char *loc,
			       int *type, int *ncomp, std::string *unit) 
{ COM_get_com()->get_dataitem( wa_str.c_str(), loc, type, ncomp, unit); }
#endif

inline void COM_get_size(const char *wa_str, int pane_id, int *size, int *ng=0)
{ COM_get_com()->get_size( wa_str, pane_id, size, ng); }

#ifndef C_ONLY
inline void COM_get_size(const std::string wa_str, int pane_id, int *size, int *ng=0)
{ COM_get_com()->get_size( wa_str.c_str(), pane_id, size, ng); }
#endif

#define COM_get_array_prototype( type) \
inline void COM_get_array( const char *wa_str, int pane_id, \
			   type **addr, int *strd=NULL, int *cap=NULL) \
{ COM_get_com()->get_array( wa_str, pane_id, (void**)addr, strd, cap); }

COM_get_array_prototype( void)

#ifndef C_ONLY
COM_get_array_prototype( char)
COM_get_array_prototype( int)
COM_get_array_prototype( float)
COM_get_array_prototype( double)
#endif

#define COM_get_array_const_prototype( type) \
inline void COM_get_array_const( const char *wa_str, int pane_id, \
			  const type **addr, int *strd=NULL, int *cap=NULL) \
{ COM_get_com()->get_array( wa_str, pane_id, \
                               (void**)addr, strd, cap, true); }

COM_get_array_const_prototype( void)

#ifndef C_ONLY
COM_get_array_const_prototype( char)
COM_get_array_const_prototype( int)
COM_get_array_const_prototype( float)
COM_get_array_const_prototype( double)
#endif

inline void COM_copy_array( const char *wa_str, int pane_id,
			    void *val, int v_strd=0, int v_size=0, 
			    int offset=0)
{ COM_get_com()->copy_array( wa_str, pane_id, val, v_strd, v_size, offset); }

inline void COM_get_bounds( const char *wa_str, int pane_id, 
			    void *lbound, void *ubound)
{ COM_get_com()->get_bounds( wa_str, pane_id, lbound, ubound); }

inline int COM_check_bounds( const char *wa_str, int pane_id)
{ return COM_get_com()->check_bounds( wa_str, pane_id); }

inline void COM_set_function( const char *wf_str, 
			      Func_ptr func, 
			      const char *intents,
			      const COM_Type *types)
{ COM_get_com()->set_function( wf_str, func, intents, types); }

#ifndef C_ONLY
inline void COM_set_member_function( const char *wf_str, 
				     Member_func_ptr func, 
				     const char *wa_str, 
				     const char *intents,
				     const COM_Type *types)
{ COM_get_com()->set_member_function( wf_str, func,
					 wa_str, intents, types); }
#endif

inline void COM_set_member_function( const char *wf_str, 
				     Func_ptr func, 
				     const char *wa_str, 
				     const char *intents,
				     const COM_Type *types)
{ COM_get_com()->set_member_function( wf_str, func, wa_str, intents, types); }

inline void COM_get_communicator( const char *wname, MPI_Comm *comm) 
{ *comm = COM_get_com()->get_communicator( wname); }

#ifndef C_ONLY
inline void COM_get_panes( const char *wname, std::vector<int> &pane_ids, 
			   int rank=-2) 
{ COM_get_com()->get_panes( wname, pane_ids, rank); }

inline void COM_get_windows( std::vector<std::string> &names )
{ COM_get_com()->get_windows(names); }

inline void COM_get_modules( std::vector<std::string> &names )
{ COM_get_com()->get_modules(names); }

inline void COM_get_dataitems( const char *wname, int *na, std::string &names)
{ COM_get_com()->get_dataitems( wname, na, names); }

inline void COM_get_connectivities( const char *wname, int pane_id, 
				    int *nc, std::string &names) 
{ COM_get_com()->get_connectivities( wname, pane_id, nc, names); }

inline void COM_get_parent( const char *waname, int pane_id, 
			    std::string &parent)
{ COM_get_com()->get_parent( waname, pane_id, parent); }
#endif

inline void COM_get_panes( const char *wname, int *npanes, 
			   int **pane_ids=NULL, int rank=-2) {
  std::vector<int> vec;
  COM_get_com()->get_panes( wname, vec, rank, pane_ids); 
  if ( npanes) *npanes = vec.size();
}

inline void COM_get_dataitems( const char *wname, int *na, 
				char **names=NULL) { 
  std::string str; 
  COM_get_com()->get_dataitems( wname, na, str, names); 
}

inline void COM_get_connectivities( const char *wname, int pane_id, 
				    int *nc, char **names=NULL) {
  std::string str; 
  COM_get_com()->get_connectivities( wname, pane_id, nc, str, names); 
}

inline void COM_get_parent( const char *waname, int pane_id, char **parent) {
  std::string str; 
  COM_get_com()->get_parent( waname, pane_id, str, parent); 
}

#ifndef C_ONLY
inline void COM_free_buffer( int **buf)
{ COM_get_com()->free_buffer( buf); }
#endif

inline void COM_free_buffer( char **buf)
{ COM_get_com()->free_buffer( buf); }
  
inline int COM_get_window_handle( const char *wname)
{ return COM_get_com()->get_window_handle( wname); }

#ifndef C_ONLY
inline int COM_get_window_handle( const std::string wname)
{ return COM_get_com()->get_window_handle( wname.c_str()); }
#endif

inline int COM_get_dataitem_handle( const char *waname)
{ return COM_get_com()->get_dataitem_handle( waname); }

#ifndef C_ONLY
inline int COM_get_dataitem_handle( const std::string waname)
{ return COM_get_com()->get_dataitem_handle( waname.c_str()); }
#endif

inline int COM_get_dataitem_handle_const( const char *waname)
{ return COM_get_com()->get_dataitem_handle_const( waname); }

#ifndef C_ONLY
inline int COM_get_dataitem_handle_const( const std::string waname)
{ return COM_get_com()->get_dataitem_handle_const( waname.c_str()); }
#endif

inline int COM_get_function_handle( const char *wfname)
{ return COM_get_com()->get_function_handle( wfname); }

inline int COM_get_status( const char *waname, const int pane_id)
{ return COM_get_com()->get_status( waname, pane_id); }

/* Invoke a function by a name registered to COM. */
extern "C" void COM_call_function( const int wf, int argc, ...);

/* Non-blocking invocation of a function by a name registered to COM. */
extern "C" void COM_icall_function( const int wf, int argc, ...);

#ifndef C_ONLY
// Blocking function calls
inline void COM_call_function( const int wf)
{ COM_get_com()->call_function( wf, 0, 0); }
inline void COM_call_function( const int wf, const void *wa) {
  COM_get_com()->call_function( wf, 1, (void**)&wa);
}
inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2) {
  const void *args[] = { wa1, wa2};
  COM_get_com()->call_function( wf, 2, (void **)args);
}
inline void COM_call_function( const int wf, const void *wa1,
			       const void *wa2, const void *wa3) {
  const void *args[] = { wa1, wa2, wa3 };
  COM_get_com()->call_function( wf, 3, (void**)args);
}
inline void COM_call_function( const int wf, 
			       const void *wa1, const void *wa2, 
			       const void *wa3, const void *wa4) {
  const void *args[] = { wa1, wa2, wa3, wa4 };
  COM_get_com()->call_function( wf, 4, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5 };
  COM_get_com()->call_function( wf, 5, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6 };
  COM_get_com()->call_function( wf, 6, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6, const void *wa7)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7 };
  COM_get_com()->call_function( wf, 7, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6, const void *wa7,
                               const void *wa8)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7, wa8};
  COM_get_com()->call_function( wf, 8, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6, const void *wa7,
                               const void *wa8, const void *wa9)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7, wa8, wa9};
  COM_get_com()->call_function( wf, 9, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6, const void *wa7,
                               const void *wa8, const void *wa9,
                               const void *wa10)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7, wa8, wa9, wa10};
  COM_get_com()->call_function( wf, 10, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6, const void *wa7,
                               const void *wa8, const void *wa9,
                               const void *wa10, const void *wa11)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7, wa8, wa9, wa10, wa11};
  COM_get_com()->call_function( wf, 11, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6, const void *wa7,
                               const void *wa8, const void *wa9,
                               const void *wa10, const void *wa11,
                               const void *wa12)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7, wa8, 
                         wa9, wa10, wa11, wa12};
  COM_get_com()->call_function( wf, 12, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6, const void *wa7,
                               const void *wa8, const void *wa9,
                               const void *wa10, const void *wa11,
                               const void *wa12, const void *wa13)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7, wa8, 
                         wa9, wa10, wa11, wa12, wa13};
  COM_get_com()->call_function( wf, 13, (void**)args);
}

inline void COM_call_function( const int wf, const void *wa1, 
			       const void *wa2, const void *wa3, 
			       const void *wa4, const void *wa5,
			       const void *wa6, const void *wa7,
                               const void *wa8, const void *wa9,
                               const void *wa10, const void *wa11,
                               const void *wa12, const void *wa13,
                               const void *wa14)  {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7, wa8, 
                         wa9, wa10, wa11, wa12, wa13, wa14};
  COM_get_com()->call_function( wf, 14, (void**)args);
}

// Non-blocking function calls
inline void COM_icall_function( const int wf, int *id) 
{ COM_get_com()->icall_function( wf, 0, 0, id); }
inline void COM_icall_function( const int wf, const void *wa, int *id) {
  COM_get_com()->icall_function( wf, 1, (void**)&wa, id);
}
inline void COM_icall_function( const int wf, const void *wa1,
				const void *wa2, int *id) {
  const void *args[] = { wa1, wa2};
  COM_get_com()->icall_function( wf, 2, (void **)args, id);
}
inline void COM_icall_function( const int wf, const void *wa1,
				const void *wa2, const void *wa3, 
				int *id) {
  const void *args[] = { wa1, wa2, wa3 };
  COM_get_com()->icall_function( wf, 3, (void **)args, id);
}
inline void COM_icall_function( const int wf, const void *wa1,
				const void *wa2, const void *wa3, 
				const void *wa4, int *id) {
  const void *args[] = { wa1, wa2, wa3, wa4 };
  COM_get_com()->icall_function( wf, 4, (void **)args, id);
}

inline void COM_icall_function( const int wf, const void *wa1, 
				const void *wa2, const void *wa3, 
				const void *wa4, const void *wa5, 
				int *id) {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5 };
  COM_get_com()->icall_function( wf, 5, (void **)args, id);
}

inline void COM_icall_function( const int wf, const void *wa1,
				const void *wa2, const void *wa3, 
				const void *wa4, const void *wa5,
				const void *wa6, int *id) {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6 };
  COM_get_com()->icall_function( wf, 6, (void **)args, id);
}

inline void COM_icall_function( const int wf, const void *wa1,
				const void *wa2, const void *wa3, 
				const void *wa4, const void *wa5,
				const void *wa6, const void *wa7, 
				int *id) {
  const void *args[] = { wa1, wa2, wa3, wa4, wa5, wa6, wa7 };
  COM_get_com()->icall_function( wf, 7, (void **)args, id);
}
#endif

inline void COM_wait( const int id) 
{ COM_get_com()->wait( id); }

inline int COM_test( const int id) 
{ return COM_get_com()->test( id); }

inline void COM_set_verbose( int i) 
{ COM_get_com()->set_verbose( i); }

// Profiling tools
inline void COM_set_profiling( int i) 
{ COM_get_com()->set_profiling( i); }

// Profiling tools
inline void COM_set_profiling_barrier( int hdl, MPI_Comm comm) 
{ COM_get_com()->set_profiling_barrier( hdl, comm); }

inline void COM_print_profile( const char *fname, const char *header) 
{ COM_get_com()->print_profile( fname, header); }

inline int COM_get_sizeof( const COM_Type type, int c) 
{ return COM::DataItem::get_sizeof( type, c); }

inline int COM_compatible_types( COM_Type type1, COM_Type type2) 
{ return COM::DataItem::compatible_types( type1, type2); }

inline int COM_get_error_code() 
{ return COM_get_com()->get_error_code(); }

#endif

#endif /* __COM_CPP_H__ */



