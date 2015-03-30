//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include "Pane.hpp"
#include "ComponentInterface.hpp"
#include "com_assertion.h"
#include <cstdlib>
#include <cstdio>

COM_BEGIN_NAME_SPACE

Pane::Pane( Window *w, int i) :  
  _window(w), _id(i), _ignore_ghost(false) 
{
  _attr_set.resize( COM_NUM_KEYWORDS);
  DataItem *as = new DataItem[COM_NUM_KEYWORDS];

  for ( int i=0; i<COM_NUM_KEYWORDS; ++i) {
    // Call constructor in placement form.
    _attr_set[i] = new (&as[i]) DataItem_friend(this, i);
  }
}

Pane::Pane( Pane *p, int id) :
  _window(p->_window), _id(id), _ignore_ghost(false) 
{
  int n = p->_attr_set.size();
  _attr_set.resize( n, NULL);

  // First allocate for keywords
  DataItem *as = new DataItem[COM_NUM_KEYWORDS];
  for ( int i=0; i<COM_NUM_KEYWORDS; ++i) {
    _attr_set[i] = new (&as[i]) DataItem( this, p->_attr_set[i], "", i);
  }

  for ( int i=COM_NUM_KEYWORDS; i<n; ) {
    DataItem *ap=p->_attr_set[i];
    if ( ap==NULL || ap->pane()==NULL) { ++i; continue; }
    int ncomp=ap->size_of_components();

    // Allocate for dataitem and its components
    as = new DataItem[ncomp+(ncomp>1)];

    // Call constructor at the dataitem and its individual components
    for ( int k=0, nk=(ncomp==1)?0:ncomp; k<=nk; ++k,++i) 
      _attr_set[i] = new (&as[k]) DataItem( this, &ap[k], "", i);

    // Inherit window dataitems from dummy pane
    if ( ap->is_windowed())
      reinterpret_cast<DataItem_friend*>(as)->inherit( ap, INHERIT_USE, true);
  }
}

Pane::~Pane() {
  delete [] _attr_set[0];  // Delete all keywords

  delete_dataitem( COM_DATA); // Delete all user-defined dataitems.

  for ( Size i=0,n=_cnct_set.size(); i<n; ++i) 
    delete _cnct_set[i];  // Delete all connectivity tables.
}


void Pane::init_done() throw(COM_exception) {
  _attr_set.resize( _window->last_dataitem_id()); // Shrink the array.

  // Loop through the dataitems to initialize the unitialized ones to NULL
  for ( int i=0, s=_attr_set.size(); i<s; ++i) {
    DataItem *a = _attr_set[i];
    if ( a==NULL || a->pane()==NULL) continue;

    if ( a->initialized() && a->size_of_items()>a->capacity()) 
      throw COM_exception( COM_ERR_INVALID_CAPACITY, 
			   append_frame(a->fullname(), Pane::init_done));
  }
}

Connectivity *Pane::connectivity( Size i)  throw( COM_exception)
{
  if ( i>size_of_elements()) {
    // if i is not a valid element ID, it must be one past the last ID
    COM_assertion( i-1==size_of_elements());
    return NULL;
  }

  // i must be either a valid element ID
  COM_assertion_msg( i>=1, "ElementID out of bounds");

  Cnct_set::iterator it=_cnct_set.begin();
  for ( ; it != _cnct_set.end(); ++it) {
    Connectivity *c=*it;
    if ( i>c->index_offset() && i<=c->index_offset()+c->size_of_elements()) 
      return c;
  }
  return NULL;
}


Connectivity *Pane::connectivity(  const std::string &a, 
				   bool insert) throw( COM_exception)
{
  if ( _id == 0) {
    throw COM_exception( COM_ERR_PANE_NOTEXIST, append_frame
			 (_window->name()+"."+a, Pane::connectivity));
  }
  Cnct_set::iterator it=_cnct_set.begin();
  for ( ; it != _cnct_set.end(); ++it) {
    if ( a==(*it)->name()) return *it;
  }
  if ( insert) {
    const int *size_info=Connectivity::get_size_info(a);
    if ( size_info) {
      Connectivity *c = new Connectivity(this, a, -1-int(_cnct_set.size()), 
					 size_info);
      _cnct_set.push_back( c);
      return _cnct_set.back();
    }
    else
      throw COM_exception( COM_ERR_INVALID_DATAITEM_NAME, append_frame
			   (_window->name()+"."+a, Pane::connectivity));
  }

  return NULL;
}

DataItem* Pane::
dataitem( const std::string &a) {
  DataItem *w_attr = _window->dataitem(a);
  if ( w_attr==NULL) return NULL;

  return dataitem( w_attr->id());
}

DataItem* Pane::
dataitem( int i) {
  if ( (int)_attr_set.size() <= i) return NULL;
  DataItem *p_attr = _attr_set[ i];
  
  if ( p_attr==NULL || p_attr->id () != i) return NULL;
  return p_attr;
}

// Obtain all the nonempty dataitems of the pane.
void Pane::dataitems( std::vector<DataItem*> &as) {
  as.reserve( _attr_set.size()-COM_NUM_KEYWORDS);
  for ( int i=COM_NUM_KEYWORDS, size=_attr_set.size(); i<size; ++i) {
    DataItem *a = _attr_set[i];
    if ( a==NULL || a->pane()==NULL) continue;

    as.push_back( a);
    int ncomp = a->size_of_components();
    if ( ncomp>1) i+=ncomp;
  }
}

DataItem *Pane::
new_dataitem( const std::string &aname, int aid, const char loc, 
	       const int type, int ncomp, const std::string &unit) 
  throw(COM_exception) 
{

  if ( loc == 'c' && id() != 0) {
    // check if connectivity table exists, make sure it is not dummy pane
    std::string::size_type pos = aname.find( ":");
    std::string conn = aname.substr(pos, aname.size());
    Connectivity *con = connectivity( conn,false);
    if (con == NULL)
      throw COM_exception( COM_ERR_INVALID_DATAITEM_NAME, 
			 append_frame(aname, Pane::new_dataitem));
  }

  // Insert the object into both the set and the map.
  DataItem *atts;
  if ( aid>=int(_attr_set.size()))
    atts = new DataItem[ncomp+(ncomp>1)]; // Allocate memory
  else {
    atts = dataitem(aid); COM_assertion( atts);
    // Call destructors
    for ( int i=0, n=ncomp+(ncomp>1); i<n; ++i) atts[i].~DataItem();
  }
  DataItem *a = new (atts) DataItem( this, aname, aid, loc, 
				       type, ncomp, unit);
  insert( a);
  
  // Create an object for each individual component
  if ( ncomp>1) for ( int i=1; i<=ncomp; ++i) {
    char buf[10];
    std::sprintf( buf, "%d-", i);
    std::string newname = std::string( buf)+a->name();
    
    DataItem *ai = new (&atts[i]) DataItem( this, newname, aid+i, 
					      loc,  type, 1, unit);
    insert( ai);
  }

  if ( _id && loc == 'w') { // Inherit window dataitem
    reinterpret_cast<DataItem_friend*>(a)->
      inherit( _window->dataitem( aid), INHERIT_USE, true);
  }
  return a;
}

void Pane::insert( DataItem *attr) throw(COM_exception) {
  COM_assertion( attr->pane()==this);

  // Determine id of the new dataitem
  int id = attr->id();

  // Resize _attr_set and delete existing dataitem with the id
  if ( int(_attr_set.size())<=id) { // resize _attr_set
    int s = attr->size_of_components();
    _attr_set.resize( id+s+(s>1), NULL);
  }

  if ( _attr_set[id]==NULL) _attr_set[id] = attr;
  else COM_assertion( attr == _attr_set[id]);
}

void Pane::delete_dataitem( int id) throw(COM_exception) {
  if ( id == COM_DATA) { // Delete all user-defined dataitems.
    for ( Size i=COM_NUM_KEYWORDS,n=_attr_set.size(); i<n; ++i) {
      DataItem *a=_attr_set[i];
      if ( a==NULL || a->pane()==NULL) continue;
      int ncomp = a->size_of_components();
      delete [] a; // Delete dataitem and its individual components
      if (ncomp>1) i+=ncomp;
    }
    _attr_set.resize( COM_NUM_KEYWORDS);
  }
  else {
    DataItem *a = _attr_set[id]; COM_assertion(a);
    int ncomp = a->size_of_components();
    delete [] a; // Delete the dataitem and its components.
    
    // Reset its pointers in _attr_set.
    for ( int i=id+((ncomp>1)?ncomp:0); i>=id; --i) _attr_set[i] = NULL;
    
    if ( id+ncomp+(ncomp>1) ==int(_attr_set.size())) 
      _attr_set.resize( id);
  }
}

void Pane::
reinit_dataitem( int aid, OP_Init op, void **addr, 
                 int strd, int cap) throw (COM_exception)
{
  switch ( aid) {
  case COM_CONN: {
    COM_assertion( op != OP_SET && op != OP_SET_CONST);
    // loop through the connectivity
    for ( int i=0, n=_cnct_set.size(); i<n; ++i) {
      if ( !_cnct_set[i]->is_structured())
	reinit_conn( _cnct_set[i], op, NULL, strd, cap);
    }

    if ( addr) *addr = NULL;
    return;
  }
  case COM_MESH: {
    COM_assertion( op != OP_SET && op != OP_SET_CONST);
    reinit_dataitem( COM_NC, op, NULL, strd, cap);
    reinit_dataitem( COM_CONN, op, NULL, strd, cap);
    reinit_dataitem( COM_RIDGES, op, NULL, strd, cap);
    if ( addr) *addr = NULL;
    return;
  }
  case COM_PMESH: {
    COM_assertion( op != OP_SET && op != OP_SET_CONST);
    reinit_dataitem( COM_NC, op, NULL, strd, cap);
    reinit_dataitem( COM_CONN, op, NULL, strd, cap);
    reinit_dataitem( COM_RIDGES, op, NULL, strd, cap);
    reinit_dataitem( COM_PCONN, op, NULL, strd, cap);
    if ( addr) *addr = NULL;
    return;
  }
  case COM_DATA: {
    COM_assertion( op != OP_SET && op != OP_SET_CONST);
    // Loop through the dataitems
    for ( int i=COM_NUM_KEYWORDS, n=_attr_set.size(); i<n; ++i) {
      DataItem *a = _attr_set[i];
      if ( a==NULL || a->pane()==NULL) continue;
      if ( !a->is_windowed() || _id==0)
	reinit_dataitem( i, op, NULL, strd, cap);
      int ncomp= a->size_of_components();
      if ( ncomp>1) i+= ncomp;
    }
    return;
  }
  case COM_ALL: {
    COM_assertion( op != OP_SET && op != OP_SET_CONST);
    reinit_dataitem( COM_PMESH, op, NULL, strd, cap);
    reinit_dataitem( COM_DATA, op, NULL, strd, cap);
    return;
  }
  default: ;
  }

  DataItem *a = dataitem( aid);
  if ( a==NULL) 
    throw COM_exception( COM_ERR_DATAITEM_NOTEXIST, 
			 append_frame(a->fullname(), Pane::reinit_dataitem));

  // If _id is 0, then handle only window dataitems.
  if ( !a->is_windowed() && !_id) {
    if (op == OP_SET || op == OP_SET_CONST)
      throw COM_exception( COM_ERR_PANE_NOTEXIST, append_frame
			   ( _window->name(), Pane::reinit_dataitem));
    else
      return;
  }

  int errcode;
  void *p;
  int ncomp = a->size_of_components();
      
  if ( op==OP_DEALLOC) {
    // Deallocate if was allocated by Roccom
    errcode = a->deallocate(); 
    p = NULL;
  }
  else {
    COM_assertion( op==OP_SET_CONST||op==OP_SET||op==OP_ALLOC||op==OP_RESIZE);

    // If size is not yet set
    if ( !a->size_set()) {
      if ( a->is_windowed())
	a->set_size(1);  // Change the default size of window dataitems to 1
      else // Throw an exception for other types of dataitems
	throw COM_exception( COM_ERR_INVALID_SIZE, 
			     append_frame(a->fullname(), Pane::reinit_dataitem));
    }

    // Assign default value for cap and strd
    if ( cap==0) {
      // Default value for capacity is the number of items for set_array
      // but the large of the current capacity and the the number of items.
      if ( op==OP_SET || op==OP_SET_CONST) cap = a->size_of_items();
      else {
	if ( a->capacity()==0)
	  cap = a->size_of_items();
	else if ( a->size_of_items()<=a->capacity())
	  cap = a->capacity();
	else 
	  cap = a->size_of_items()+a->size_of_items()/5;  // Current size pluse 20% more
      }
    }

    if ( (op == OP_SET || op == OP_SET_CONST) && *addr && cap==0)
      *addr = NULL; // Enforce the array to be a NULL pointer

    // Make the dataitem itself set to NULL if operating on its components.
    if ( DataItem::is_digit(a->name()[0])) {
      int icomp=std::atoi( a->name().c_str());
      if ( dataitem(aid-icomp)->initialized())
	throw COM_exception(COM_ERR_WAS_INITIALIZED, 
			    append_frame(a->fullname(), Pane::reinit_dataitem));
    }

    if ( strd==0 || strd<0 && a->stride()==0) 
      // Default value for stride is number of components
      strd = ncomp;
    else if ( strd<0)
      strd = a->stride();

    if ( op == OP_SET || op == OP_SET_CONST) {
      ((DataItem_friend*)a)->set_pointer(*addr, strd, cap, 
					  0, op==OP_SET_CONST);
      p = *addr;
    }
    else {
      p = a->allocate( strd, cap, op == OP_ALLOC);
      if ( addr) *addr = p;
    }
    errcode=0;
  }
}

void Pane::
reinit_conn( Connectivity *con, OP_Init op, int **addr, 
	     int strd, int cap) throw (COM_exception)
{
  // Assign default value for cap and strd
  if ( op!=OP_DEALLOC) {
    if ( cap==0) {
      // Default value for capacity is the number of items for set_array
      // but the large of the current capacity and the the number of items plus some buffer.
      if ( op==OP_SET || op==OP_SET_CONST) cap = con->size_of_items();
      else {
	if ( con->capacity()==0)
	  cap = con->size_of_items();
	else if ( con->size_of_items()<=con->capacity())
	  cap = con->capacity();
	else 
	  cap = con->size_of_items()+con->size_of_items()/5;  // Current size pluse 20% more
      }
    }

    if ( strd==0 || strd<0 && con->stride()==0) 
      // Default value for stride is number of components
      strd = con->size_of_components();
    else if ( strd<0)
      strd = con->stride();
  }

  if ( op == OP_SET || op == OP_SET_CONST) {
    if ( !id()) 
      throw COM_exception( COM_ERR_PANE_NOTEXIST, 
			   append_frame(con->fullname(),Pane::reinit_conn));

    if ( *addr && cap==0 && !con->is_structured())
      *addr = NULL; // Enforce the array to be a NULL pointer

    ((Connectivity_friend*)con)->set_pointer( *addr, strd, cap,
					      op==OP_SET_CONST);
  }
  else {
    if ( con->is_structured()) 
      throw COM_exception( COM_ERR_ALLOC_STRUCTURED, 
			   append_frame(con->fullname(),Pane::reinit_conn));
    void *p=NULL;
    if ( op==OP_DEALLOC) 
    { con->deallocate(); p=NULL; }
    else 
      p = con->allocate( strd, cap, op==OP_ALLOC);
    
    if ( addr) *addr = (int*)p;
  }
}

DataItem *Pane::inherit( DataItem *from, const std::string &aname,
			  int mode, bool withghost) throw(COM_exception) 
{
  if ( from==NULL) 
    throw COM_exception( COM_ERR_DATAITEM_NOTEXIST, append_frame
			 (_window->name()+"."+aname,Pane::inherit));

  switch (from->id()) { // Process keywords
  case COM_ALL:
    inherit( from->pane()->dataitem( COM_PMESH), "", mode, withghost);
    inherit( from->pane()->dataitem( COM_DATA), "", mode, withghost);
    return dataitem( COM_ALL);
  case COM_MESH:
    inherit( from->pane()->dataitem( COM_NC), "", mode, withghost);
    inherit( from->pane()->dataitem( COM_CONN), "", mode, withghost);
    inherit( from->pane()->dataitem( COM_RIDGES), "", mode, withghost);
    return dataitem( COM_MESH);
  case COM_PMESH:
    inherit( from->pane()->dataitem( COM_NC), "", mode, withghost);
    inherit( from->pane()->dataitem( COM_CONN), "", mode, withghost);
    inherit( from->pane()->dataitem( COM_RIDGES), "", mode, withghost);
    inherit( from->pane()->dataitem( COM_PCONN), "", mode, withghost);
    return dataitem( COM_PMESH);
  case COM_DATA: {
    // Loop through all dataitems
    std::vector<DataItem*> &atts=from->pane()->_attr_set;
    
    for ( int i=COM_NUM_KEYWORDS,n=atts.size(); i<n; ++i) {
      DataItem *a = atts[i];
      if ( a==NULL || a->pane()==NULL) continue;
      if ( !a->is_windowed() || _id==0)
	inherit( a, "", mode, withghost);
      int ncomp= a->size_of_components();
      if ( ncomp>1) i+= ncomp;
    }
    return dataitem( COM_DATA);
  }
  case COM_CONN: {
    if ( _id==0) return dataitem(COM_CONN);

    if (mode != INHERIT_COPY) {
      // Clean up the current connectivity tables
      for ( Size i=0; i<_cnct_set.size(); ++i) delete _cnct_set[i]; 
      _cnct_set.clear();
      
      if ( mode==INHERIT_USE) {
	((DataItem_friend*)_attr_set[COM_CONN])->inherit( from, false, withghost);
	// Check that if structured meshes, the nodal coordinates 
	// was also used.
	COM_assertion( !is_structured() || _attr_set[COM_NC]->parent() && 
		       _attr_set[COM_NC]->parent()->pane() == from->pane());
      }

      // Inherit individual connectivity tables
      std::vector<Connectivity*>::iterator it=from->pane()->_cnct_set.begin();
      std::vector<Connectivity*>::iterator iend=from->pane()->_cnct_set.end();
      for ( ; it != iend; ++it) {
	_cnct_set.push_back( new Connectivity( this, *it, (*it)->name(),
					       -_cnct_set.size()-1));
	((Connectivity_friend*)_cnct_set.back())->inherit(*it, mode, withghost);
      }

      // Set _ignore_ghost
      _ignore_ghost = !withghost;
      if ( _ignore_ghost && is_structured() && size_of_ghost_layers())
	throw COM_exception(COM_ERR_GHOST_LAYERS, append_frame
			    (_window->name()+"."+aname,Pane::inherit));
    
      if ( mode == INHERIT_USE) return dataitem( COM_DATA);
    }

    // Continue to copy mode
    // Do not copy connectivity tables for structured meshes.
    const std::vector<Connectivity*> &es = from->pane()->_cnct_set;
    COM_assertion_msg( es.size() == _cnct_set.size(),
		       "Number of connectivity tables do not match");
      
    // Loop over the connectivity tables to copy each table
    for ( int i=0, ni=_cnct_set.size(); i<ni; ++i) {
      const Connectivity *conn=es[i];
      int n=withghost?conn->size_of_items():conn->size_of_real_items();
      _cnct_set[i]->copy_array( const_cast<int*>(conn->pointer()), 
				conn->stride(), n);
    }

    refresh_connectivity();
    return dataitem( COM_DATA);
  }
  default: ;
  }

  std::string str = aname.size()?aname:from->name();

  // If from is a keyword and target is the same, then the current 
  // dataitem must have the same ID. Otherwise, must find from name.
  DataItem *a=(from->id()<COM_NUM_KEYWORDS && aname.empty())?
    dataitem(from->id()) : _window->dataitem(str);

  if ( mode != INHERIT_COPY) {
    if ( _id == 0) {
      if ( a==NULL || a->size_of_components()!=from->size_of_components()) { 
	// New dataitem only if it does not exist or the sizes do not match
	try {
	  a=_window->new_dataitem( str, from->location(), from->data_type(),
				    from->size_of_components(), from->unit());

	  if ( from->is_windowed())
	    ((DataItem_friend*)a)->inherit( from, mode, withghost);

	} CATCHEXP_APPEND(Pane::inherit);
      }
      else try {
	((DataItem_friend*)a)->inherit( from, mode, withghost);
      } CATCHEXP_APPEND(Pane::inherit) CATCHBADALLOC_APPEND(Pane::inherit);

      if ( mode == INHERIT_USE || !from->is_windowed()) return a;
    }
    else {
      COM_assertion( a);
      a = _attr_set[a->id()]; 
      try { 
	((DataItem_friend*)a)->inherit( from, mode, withghost);
      } CATCHEXP_APPEND(Pane::inherit) CATCHBADALLOC_APPEND(Pane::inherit);
      
      if ( mode == INHERIT_USE) return a;
    }
  }
  else {
    // Copy an dataitem only if it exists in the target window.
    if ( a) a = _attr_set[a->id()];
    else return a;

    if ( !DataItem::compatible_types( from->data_type(), a->data_type()))
      throw COM_exception( COM_ERR_INCOMPATIBLE_TYPES, append_frame
			   (from->fullname()+" and "+a->fullname(),
			    Pane::inherit));
  }

  if ( (_id==0) != (from->is_windowed())) return a;

  // Continue to copy dataset
  if ( !withghost && is_structured() && size_of_ghost_layers())
    throw COM_exception(COM_ERR_GHOST_LAYERS, append_frame
			(_window->name()+"."+aname,Pane::inherit));
    
  // count is the number of panes, nodes, or elements to loop through
  int count = withghost?from->size_of_items():from->size_of_real_items();
  int s_nc = from->size_of_components();
  const Pane *src_pane = from->pane();

  COM_assertion_msg( from->location() == a->location(),
		     (std::string("Location of dataitems ")+
		      from->fullname()+" and " +a->fullname()+ 
		      " do not match during copying.").c_str());

  // loop through components of source's data
  if ( count) for ( int j=(s_nc>1), nj=s_nc-(s_nc==1); j<=nj; ++j) {
    // src_data points to source data for current component
    const DataItem *src_data = src_pane->dataitem( from->id()+j);
      
    // dest_data points to dest data for current component
    DataItem *dest_data = dataitem( a->id()+j);
    
    if ( dest_data->pointer() && src_data->pointer()) {
      COM_assertion_msg( withghost && dest_data->size_of_items()==src_data->size_of_items() ||	
			 !withghost && dest_data->size_of_real_items()==src_data->size_of_real_items(),
			 (std::string("Number of items of dataitems ")+
			  from->fullname()+" and " +a->fullname()+ 
			  " do not match during copying.").c_str());
      try {
	dest_data->copy_array( const_cast<void*>(src_data->pointer()), 
			       src_data->stride(), count);
      } CATCHEXP_APPEND(Pane::inherit);
    }
  }

  return a;
}

void Pane::set_size( DataItem *a, 
		     int nitems, int ng) throw( COM_exception) {
  if ( nitems<ng)
    throw COM_exception( COM_ERR_INVALID_SIZE, 
			 append_frame(a->fullname(),Pane::set_size));
  // Get the corresponding dataitem object.
  if ( !a->is_windowed() && !_id) 
    throw COM_exception(COM_ERR_PANE_NOTEXIST,
			append_frame(a->fullname(),Pane::set_size));

  if ( a->is_nodal()) {
    COM_assertion_msg( a->id() == COM_NC, 
		       (std::string("Cannot set size for nodal dataitem ")+
			a->fullname()+". Must use the nc dataitem").c_str());

    _attr_set[COM_NC]->set_size( nitems, ng);
  }
  else if ( a->is_elemental()) {
    COM_assertion_msg( a->id() == COM_CONN, 
		       (std::string("Cannot set size for elemental dataitem ")+
			a->fullname()+". Must use the conn dataitem").c_str());
    
    _attr_set[COM_CONN]->set_size( nitems, ng);
  }
  else {
    dataitem(a->id())->set_size( nitems, ng);
  }
}

void Pane::set_size( Connectivity *con, 
		     int nitems, int ng) throw( COM_exception) {
  if ( !con->is_structured() && nitems<ng)
    throw COM_exception( COM_ERR_INVALID_SIZE, 
			 append_frame(con->fullname(),Pane::set_size));
  con->set_size( nitems, ng);
}

void Pane::refresh_connectivity() throw(COM_exception) {
  int nelems = 0, ngelems = 0;

  // Set the number of elements
  if ( !_attr_set[COM_CONN]->parent()) {
    // Loop through the connectivity table
    Cnct_set::iterator it=_cnct_set.begin();
    for ( ; it != _cnct_set.end(); ++it) {
      Connectivity_friend *con = (Connectivity_friend*)*it;
      con->set_offset( nelems);
      nelems += con->size_of_elements();
      ngelems += con->size_of_ghost_elements();
    }

    if ( nelems<0 || ngelems<0)
      throw COM_exception( COM_ERR_INVALID_SIZE, append_frame
			   ( _window->name(), Pane::refresh_connectivity));
    _attr_set[COM_CONN]->set_size(nelems, ngelems);
  }

  // Set the number of nodes for structured meshes
  if ( is_structured() && !_attr_set[COM_NC]->parent()) {
    _attr_set[COM_NC]->set_size(_cnct_set[0]->size_of_nodes(), 
				_cnct_set[0]->size_of_ghost_nodes());
  }
}

COM_END_NAME_SPACE



