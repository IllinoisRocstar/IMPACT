// MPACT LICENSE 

/** \file maps.hpp
 * Contains declaration of mappings for module, window, function, dataitem.
 * @see COM_base.C
 */

#ifndef __COM_MAPS_H__
#define __COM_MAPS_H__

#include "com_basic.h"
#include "com_exception.hpp"
#include <list>

COM_BEGIN_NAME_SPACE

/// Supports mapping from names to handles and vice-versa for 
/// a module, window, function, or attribute.
template <class Object>
class COM_map {
  typedef std::vector<Object>         I2O; ///< Mapping from indices to objects
  typedef std::map<std::string, int>  N2I; ///< Mapping from names to indices
public:
  typedef Object                      value_type;

  COM_map() {}
  ~COM_map() {}
  
  /// Insert an object into the table.
  int add_object( std::string name, Object t, 
		  bool is_const=false) throw(COM_exception);

  /// Remove an object from the table.
  void remove_object( std::string name, 
		      bool is_const=false) throw(COM_exception);

  /// whether the object mutable
  bool is_immutable(int i) const
  { return names[i].find(" (const)")!=std::string::npos; }

  /// Access an object using its handle.
  const Object &operator[](int i) const throw(COM_exception) {
    if ( i>=(int)i2o.size()) throw COM_exception( COM_UNKNOWN_ERROR);
    return i2o[i];
  }

  Object &operator[](int i) throw(COM_exception) {
    if ( i>=(int)i2o.size()) throw COM_exception( COM_UNKNOWN_ERROR);
    return i2o[i];
  }

  /// Name of the object
  const std::string &name( int i) const { return names[i]; }

  int size() const { return names.size(); }

  std::pair<int,Object *> find( const std::string &name, bool is_const=false) {
    N2I::iterator it=n2i.find( is_const?name+"(const)":name);
    if ( it == n2i.end()) 
      return std::pair<int,Object *>(-1,NULL);
    else
      return std::pair<int,Object *>(it->second, &i2o[ it->second]);
  }
  
  std::vector<std::string> get_names() { return names; }
protected:
  I2O    i2o;                      ///< Mapping from index to objects
  N2I    n2i;                      ///< Mapping from names to indices
  std::vector<std::string>  names; ///< Name of the objects
};

template <class Object>
int COM_map<Object>::add_object( std::string name, Object t, 
				    bool is_const) throw(COM_exception)
{
  if (is_const) name.append( " (const)");

  N2I::iterator it = n2i.find(name);
  int i = ( it == n2i.end()) ? -1 : it->second;

  if ( i>0) 
  { i2o[i] = t; }
  else { 
    i=i2o.size(); n2i[name] = i; 
    i2o.push_back( t); names.push_back( name); 
  }
  return i; 
}

template <class Object>
void COM_map<Object>::remove_object( std::string name, 
					bool is_const) throw(COM_exception) {
  if (is_const) name.append( " (const)");

  N2I::iterator it = n2i.find(name);
  if ( it == n2i.end()) throw COM_exception( COM_UNKNOWN_ERROR);
  int i = it->second;
  n2i.erase(it);
  it = n2i.begin();
  while(it != n2i.end()){
    if(it->second > i)
      it->second--;
    it++;
  }
  typename I2O::iterator oi = i2o.begin() + i;
  i2o.erase(oi);
  std::vector<std::string>::iterator ni = names.begin() + i;
  names.erase(ni);
}

class Function;

/// A map functions. Supports quickly finding a function object
/// from a function handle. Also contains timing information of
/// functions to support profiling.
class Function_map : protected COM_map<Function*> {
  typedef COM_map<Function*>        Base;
public:
  /// Insert a function into the table.
  int add_object( const std::string &n, Function *t) {
    unsigned int i = COM_map<Function*>::add_object( n, t);
    if ( i+1>verbs.size()) {
      verbs.resize(i+1, false);
      wtimes_self.resize(i+1, 0.);
      wtimes_tree.resize(i+1, 0.);
      counts.resize(i+1, 0);
    }
    return i;
  }

  using Base::name;
  using Base::operator[];
  using Base::size;

  std::vector<char>         verbs;         ///< Whether verbose is on
  std::vector<double>       wtimes_self;   ///< Accumulator of wall-clock time spent by itself excluding functions called by it.
  std::vector<double>       wtimes_tree;   ///< Accumulator of wall-clock time spent by itself and those functions called by it
  std::vector<int>          counts;        ///< Counts of the number of calls
};

COM_END_NAME_SPACE

#endif


