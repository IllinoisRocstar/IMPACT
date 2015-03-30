// MPACT LICENSE 

/** \file ComponentInterface.hpp
 * Contains the prototypes for the ComponentInterface object.
 * @see ComponentInterface.C
 */

#ifndef __COM_COMPONENT_INTERFACE_H__
#define __COM_COMPONENT_INTERFACE_H__

#include "Function.hpp"
#include "Pane.hpp"
#include <map>

COM_BEGIN_NAME_SPACE

/** A ComponentInterface object contains multiple panes and multiple data dataitems.
 */
class ComponentInterface {
  typedef std::map< std::string, Function>           Func_map;
  typedef std::map< std::string, DataItem*>          Attr_map;
  typedef std::map< int, Pane*>                      Pane_map;

  class Pane_friend : public Pane {
    explicit Pane_friend( Pane&);
  public:
    using Pane::new_dataitem;
    using Pane::delete_dataitem;
    using Pane::inherit;
    using Pane::set_size;
    using Pane::reinit_dataitem;
    using Pane::reinit_conn;
    using Pane::connectivity;
  };
  typedef Pane::OP_Init    OP_Init;

public:
  typedef std::map<int,int>                          Proc_map;

  // Used by get_array. Note that the default dimension is -1, which
  // is for void*. Nonnegative dimensions are reserved for Fortran pointers.
  struct Pointer_descriptor {
    explicit Pointer_descriptor( void *p, int d=-1) 
      : ptr(p), dim(d), n1(0), n2(0) {}
    void *at() { return ptr; }

    void *ptr;
    int  dim;
    int  n1, n2;
  };

  /** \name Constructor and destructor
   * \{
   */
  /** Create a ComponentInterface object  with a given name and MPI communicator.
   * \param name  name of the CI window
   * \param c     MPI communicator where the CI resides
   */
  ComponentInterface( const std::string &name, MPI_Comm c);

  /// Destructor.
  virtual ~ComponentInterface();
  //\}

  /** \name Identity
   * \{
   */
  /// Obtain the CI's name.
  const std::string &name() const { return _name; }

  /// Obtain the communicator of the CI.
  MPI_Comm get_communicator() const { return _comm; }
  //\}

  /** \name Function and data management
   * \{
   */
  /// Initialize a Function record.
  void set_function( const std::string &fname, 
		     Func_ptr func,
		     const std::string &intents, 
		     const COM_Type *types, 
		     DataItem *a, 
		     bool if_f90=false) throw(COM_exception);

  /// Initialize a Function record.
  void set_function( const std::string &fname, 
		     Member_func_ptr func,
		     const std::string &intents, 
		     const COM_Type *types, 
		     DataItem *a, 
		     bool if_f90=false) throw(COM_exception);

  /** Create a new DataItem object with given properties.
   *  \param aname dataitem name.
   *  \param loc   location ('w', 'p', 'n', or 'e').
   *  \param type  base data type.
   *  \param ncomp number of components.
   *  \param unit unit of the dataitem.
   */
  DataItem *new_dataitem( const std::string &aname, const char loc,
			    const int type, int ncomp,
			    const std::string &unit) throw(COM_exception);

  /** Delete an existing DataItem object. */
  void delete_dataitem( const std::string &aname) throw(COM_exception);

  /** Set the sizes of an dataitem for a specific pane.
   *  \param aname  dataitem name
   *  \param pane_id pane ID
   *  \param nitems total number of items (including ghosts)
   *  \param ng     number of ghosts
   */
  void set_size( const std::string &aname, int pane_id,
		 int nitems, int ng=0) throw( COM_exception);

  /** Associate an array with an dataitem for a specific pane.
   *  \param aname  dataitem name
   *  \param pane_id  pane ID
   *  \param addr   address of the array
   *  \param strd   Stride between two items of each component
   *  \param cap    capacity of the array
   *  \seealso alloc_array, resize_array
   */
  void set_array( const std::string &aname, const int pane_id,
		  void *addr, int strd=0, int cap=0, bool is_const=false) 
    throw(COM_exception);

  /** Allocate memory for an dataitem for a specific pane and 
   *  set addr to the address.
   *  \seealso alloc_array, resize_array, append_array
   */
  void alloc_array( const std::string &aname, const int pane_id,
		    void **addr, int strd=0, int cap=0) throw(COM_exception);

  /** Resize memory for an dataitem for a specific pane and 
   *  set addr to the address.
   *  \seealso set_array, alloc_array, append_array
   */
  void resize_array( const std::string &aname, const int pane_id,
		     void **addr, int strd=-1, int cap=0) throw(COM_exception);

  void resize_array( DataItem *a, void **addr, 
		     int strd=-1, int cap=0) throw(COM_exception) 
  { reinit_dataitem( a, Pane::OP_RESIZE, addr, strd, cap); }

  void resize_array( Connectivity *c, void **addr, 
		     int strd=-1, int cap=0) throw(COM_exception) 
  { reinit_conn( c, Pane::OP_RESIZE, (int**)addr, strd, cap); }

  /** Append the given array to the end of the dataitem on a specific
   *  pane, and reallocate memory for the dataitem if necessary.
   *  \seealso set_array, alloc_array, resize_array
   */
  void append_array( const std::string &aname, const int pane_id,
		     const void *val, int v_strd, int v_size) throw(COM_exception);

  /** Deallocate memory for an dataitem for a specific pane if 
   *  allocated by Roccom.
   *  \seealso alloc_array, resize_array
   */
  void dealloc_array( const std::string &aname,
		      const int pane_id=0) throw(COM_exception);

  void dealloc_array( DataItem *a) throw(COM_exception)
  { reinit_dataitem( a, Pane::OP_DEALLOC); }

  void dealloc_array( Connectivity *c) throw(COM_exception) 
  { reinit_conn( c, Pane::OP_DEALLOC); }

  /** Inherit the dataitems of another CI window with a different name.
   *  Returns the corresponding value.
   *  \param from  dataitem being copied from
   *  \param aname new name of the dataitem
   *  \param cond  an integer pane-dataitem
   *  \param val   value to be compared against cond
   *  \param inherit_mode mode of inheritance
   *  \param withghost wheather ghost nodes/elements should be ignored */
  DataItem *inherit( DataItem *from, const std::string &aname, 
		      int inherit_mode, bool withghost,
		      const DataItem *cond, int val) throw(COM_exception);

  /// Copy an dataitem object onto another.
  void copy_dataitem( const DataItem *from, 
		       DataItem *to) throw(COM_exception) {
    inherit( const_cast<DataItem*>(from), to->name(), 
	     Pane::INHERIT_COPY, true, NULL, 0);
  }
  
  /** Get the meta-information about an dataitem.
   *  \param aname  dataitem name
   *  \param l      location
   *  \param t      data type
   *  \param n      number of components
   *  \param u      unit
   */
  DataItem *get_dataitem( const std::string &aname, char *l, int *t, 
			    int *n, std::string *u) const throw(COM_exception);
  
  /** Get the sizes of an dataitem for a specific pane.
   *  \param aname  dataitem name
   *  \param pane_id pane ID
   *  \param nitems total number of items (including ghosts)
   *  \param ng     number of ghosts
   */
  void get_size( const std::string &aname, int pane_id, 
		 int *nitems, int *ng) const throw( COM_exception);

  /** Get the status of an dataitem or pane.
   *  \seealso Roccom_base::get_status()
   */
  int get_status( const std::string &aname, int pane_id) const 
    throw(COM_exception);

  /** Get the parent name of an dataitem and load into name. 
   *  If the dataitem has no parent, then name is empty.
   */
  void get_parent( const std::string &aname, int pane_id,
		   std::string &name) const throw(COM_exception);

  /** Get the address associated with an dataitem for a specific pane.
   *  \param aname  dataitem name
   *  \param pane_id    pane ID
   *  \param addr   address of the array
   *  \param strd   Stride between two items of each component
   *  \param cap    capacity of the array
   *  \seealso alloc_array, resize_array, copy_array
   */
  void get_array( const std::string &aname, const int pane_id, 
		  Pointer_descriptor &addr, 
		  int *strd=NULL, int *cap=NULL, bool is_const=false) 
    throw(COM_exception);

  /** Copy an dataitem on a specific pane into a given array.
   *  \param aname   dataitem name
   *  \param pane_id pane ID
   *  \param val     address of the user array
   *  \param v_strd  stride of user array. 0 (the default) indicates 
   *               number of components.
   *  \param v_size  number of items to be copied. 
   *               0 (the default) indicates number of items of the dataitem.
   *  \param offset  starting item to be copied in the dataitem.
   *  \seealso alloc_array, resize_array, get_array
   */
  void copy_array( const std::string &aname, const int pane_id, 
		   void *val, int v_strd=0, int v_size=0, 
		   int offset=0) const throw(COM_exception);

  /// Perform some final checking of the CI.
  void init_done( bool pane_changed=true) throw(COM_exception);

  //\}

  /** \name Pane management
   * \{
   */
  /// Obtain the number of local panes in the CI window.
  int size_of_panes() const { return _pane_map.size(); }

  /// Obtain the total number of panes in the CI window on all processes.
  int size_of_panes_global() const { return _proc_map.size(); }

  /// Obtain the process rank that owns a given pane. Returns -1 if 
  /// the pane cannot be found in the process map.
  int owner_rank( const int pane_id) const;

  /// Return the last dataitem id.
  int last_dataitem_id() const { return _last_id; }

  /// Obtain the process map
  const Proc_map &proc_map() const { return _proc_map; }

  /// Remove the pane with given ID.
  void delete_pane( const int pane_id) throw(COM_exception) {
    if ( pane_id == 0) { // delete all panes
      _pane_map.clear();
    }
    else {
      Pane_map::iterator it = _pane_map.find( pane_id);
      if ( it == _pane_map.end()) throw COM_exception(COM_ERR_PANE_NOTEXIST);
      delete it->second;
      _pane_map.erase( it);
    }
  }

  //\}
 
  /** \name Miscellaneous
   *  \{
   */
  /// Find the pane with given ID. If not found, insert a pane with given ID.
  Pane &pane( const int pane_id, bool insert=false) throw(COM_exception);
  const Pane &pane( const int pane_id) const throw(COM_exception);

  /// Obtain all the local panes of the CI window.
  void panes( std::vector<int> &ps, int rank=-2);

  /// Obtain all the local panes of the CI window.
  void panes( std::vector<Pane*> &ps);
  /// Obtain all the local panes of the CI window.
  void panes( std::vector<const Pane*> &ps) const
  { const_cast<ComponentInterface*>(this)->panes( (std::vector<Pane*> &)ps); }

  /// Obtain all the dataitems of the pane.
  void dataitems( std::vector<DataItem*> &as)
  { _dummy.dataitems( as); }
  /// Obtain all the dataitems of the pane.
  void dataitems( std::vector< const DataItem*> &as) const
  { _dummy.dataitems( as); }

  /// Obtain a pointer to the dataitem metadata from its name.
  DataItem *dataitem( const std::string &a) throw(COM_exception);
  const DataItem *dataitem( const std::string &a) const throw(COM_exception)
  { return const_cast<ComponentInterface*>(this)->dataitem( a); }

  /// Obtain a pointer to the dataitem metadata from its index.
  DataItem *dataitem( int i) throw(COM_exception) 
  { return _dummy.dataitem( i); }
  const DataItem *dataitem( int i) const throw(COM_exception) 
  { return _dummy.dataitem( i); }

  /// Obtain the function pointer from its name.
  Function *function( const std::string &f);
  const Function *function( const std::string &f) const
  { return const_cast<ComponentInterface*>(this)->function( f); }
  //\}

protected:
  /** Implementation for setting (op==OP_SET or OP_SET_CONST), allocating 
   *   (op==OP_ALLOC), resizing (op==OP_RESIZE) and deallocating 
   *   (op==OP_DEALLOC) an array for a specific dataitem.
   *  \param attr   dataitem
   *  \param op     Operation (OP_SET, OP_SET_CONST, OP_ALLOC, OP_RESIZE)
   *  \param addr   address
   *  \param strd   stride
   *  \param cap    capacity
   */
  void reinit_dataitem( DataItem *attr, OP_Init op, void **addr=NULL, 
		    int strd=0, int cap=0) throw(COM_exception);

  /** Template implementation for setting (op==OP_SET or OP_SET_CONST), 
   *   allocating (op==OP_ALLOC), resizing (op==OP_RESIZE) and deallocating 
   *   (op==OP_DEALLOC) an array for a specific connectivity table.
   *  \param attr   connectivity table
   *  \param op     Operation (OP_SET, OP_SET_CONST, OP_ALLOC, OP_RESIZE)
   *  \param addr   address
   *  \param strd   stride
   *  \param cap    capacity
   */
  void reinit_conn( Connectivity *con, OP_Init op, int **addr=NULL, 
		    int strd=0, int cap=0) throw(COM_exception);

protected:
  Pane         _dummy;       ///< Dummy pane.
  std::string  _name;        ///< Name of the CI.
  Attr_map     _attr_map;    ///< Map from dataitem names to their metadata.
                             ///< It does not contain individual components.
  Func_map     _func_map;    ///< Map from function names to their metadata.
  Pane_map     _pane_map;    ///< Map from pane ID to their metadata.
  Proc_map     _proc_map;    ///< Map from pane ID to process ranks
 
  int          _last_id;     ///< The last used dataitem index. The next
                             ///< available one is _last_id+1.
  MPI_Comm     _comm;        ///< the MPI communicator of the CI.
  enum { STATUS_SHRUNK, STATUS_CHANGED, STATUS_NOCHANGE };
  int          _status;      ///< Status of the CI.

private:
  // Disable the following two functions (they are dangerous)
  ComponentInterface( const ComponentInterface&);
  ComponentInterface &operator=( const ComponentInterface&);

private:
#ifdef DOXYGEN
  // This is to fool DOXYGEN to generate the correct collabration diagram
  DataItem    *_attr_map;
  Function    *_func_map;
  Pane        *_pane_map;
#endif /* DOXYGEN */

};

typedef ComponentInterface Window;

COM_END_NAME_SPACE

#endif


