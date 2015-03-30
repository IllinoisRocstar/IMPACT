//
//  Copyright@2001 Xiangmin Jiao
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//
// ======================================================================
//
//
// This file implements a doubly-connected linked list that manages items in 
//   place (where inserted items are not copied). It allows an item to be 
//   in multiple In_place_list's at the same time.
//
// ======================================================================

#ifndef RFC_IN_PLACE_LIST_H
#define RFC_IN_PLACE_LIST_H

#include "rfc_basic.h"
#include <iterator>
#include <algorithm>
#include <cstddef>

RFC_BEGIN_NAME_SPACE

// Forward declarations
template <class T> class _In_place_list_iterator;
template <class T, bool managed=false> class In_place_list;

template < class T, int n=1>
struct In_place_list_base {
  In_place_list_base() 
  { for (int i=0; i<n; ++i) {next_link[i]=prev_link[i]=0;} }

  T* next_link[n];        // forward pointer
  T* prev_link[n];        // backwards pointer
};

template <class T>
class _In_place_list_iterator {
protected:
  T* node;
  int dim;
public:
  friend  class In_place_list<T,false>;
  friend  class In_place_list<T,true>;

  typedef _In_place_list_iterator<T> Self;

  typedef T               value_type;
  typedef T*              pointer;
  typedef T&              reference;
  typedef std::size_t     size_type;
  typedef std::ptrdiff_t  difference_type;
  typedef std::bidirectional_iterator_tag   iterator_category;

  explicit _In_place_list_iterator( int d=0) : node(0), dim(d) {}
  explicit _In_place_list_iterator(T* x, int d=0) : node(x), dim(d) {}

  // Use default copy constructor and copy assignment operator.

  bool  operator==( const Self& x) const { return node == x.node &&dim==x.dim;}
  bool  operator!=( const Self& x) const { return node != x.node ||dim!=x.dim;}
  T&    operator*()  const { return *node; }
  T*    operator->() const { return  node; }
  Self& operator++() {
    node = node->next_link[dim];
    return *this;
  }
  Self  operator++(int) {
    Self tmp = *this;
    ++*this;
    return tmp;
  }
  Self& operator--() {
    node = node->prev_link[dim];
    return *this;
  }
  Self  operator--(int) {
    Self tmp = *this;
    --*this;
    return tmp;
  }
};

template <class T, bool managed>
class In_place_list {
protected:
  T*           node;
  std::size_t  length;
  int          dim;

  T*   get_node()           { return new T;}
  T*   get_node(const T& t) { return new T(t);}
  void put_node(T* p)       { delete p;}

public:
  typedef T               value_type;
  typedef T*              pointer;
  typedef const T*        const_pointer;
  typedef T&              reference;
  typedef const T&        const_reference;
  typedef std::size_t     size_type;
  typedef std::ptrdiff_t  difference_type;

  typedef _In_place_list_iterator<T>          iterator;
  typedef _In_place_list_iterator<const T>    const_iterator;

  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef In_place_list<T,managed>  Self;

  // creation
  explicit In_place_list( int d=0) : length(0), dim(d) {
    // creats an empty list.
    node = get_node();
    (*node).next_link[dim] = node;
    (*node).prev_link[dim] = node;
  }
  In_place_list(const Self& x) : length(0), dim(x.dim) {
    node = get_node();
    (*node).next_link[dim] = node;
    (*node).prev_link[dim] = node;
    insert(begin(), x.begin(), x.end());
  }

  ~In_place_list() {
    erase(begin(), end());
    put_node(node);
  }

  void set_dimension( int d) { dim = d; }

  // Access member functions

  iterator       begin() { return iterator(node->next_link[dim],dim); }
  const_iterator begin() const { return const_iterator(node->next_link[dim],dim); }
  iterator       end() { return iterator(node,dim); }
  const_iterator end() const { return const_iterator(node,dim); }

  reverse_iterator       rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const 
  { return const_reverse_iterator(end()); }
  reverse_iterator       rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const 
  { return const_reverse_iterator(begin()); }

  bool            empty() const    { return length == 0; }
  size_type       size() const     { return length; }
  size_type       max_size() const { return size_type(-1); }

  reference       front()          { return *begin(); }
  const_reference front() const    { return *begin(); }
  reference       back()           { return *node->prev_link[dim]; }
  const_reference back() const     { return *node->prev_link[dim]; }

  // Insertion

  iterator insert(iterator position, T& x) {
    x.next_link[dim] = position.node;
    x.prev_link[dim] = (*position.node).prev_link[dim];
    (*((*position.node).prev_link[dim])).next_link[dim] = &x;
    (*position.node).prev_link[dim] = &x;
    ++length;
    return iterator(&x,dim);
  }
  iterator insert(T* pos, T& x) {
    return insert( iterator(pos,dim), x);
  }
  void push_front(T& x) { insert(begin(), x); }

  void push_back(T& x)  { insert(end(), x); }

  void insert( T* pos, size_type n) { insert( iterator(pos,dim), n); }
  void insert( T* pos, size_type n, const T& x = T()) 
  { insert( iterator(pos,dim), n, x); }

  template <class InputIterator>
  void insert(iterator pos, InputIterator first, InputIterator last) {
    while (first != last)
      insert(pos, *get_node(*first++));
  }

  template <class InputIterator>
  void insert(T* pos, InputIterator first, InputIterator last) {
    while (first != last)
      insert(pos, *get_node(*first++));
  }

  void insert(T* pos, const T* first, const T* last) {
    insert( iterator(pos,dim), const_iterator(first,dim),
            const_iterator(last,dim));
  }

  // removal
  void erase(iterator i) {
    RFC_assertion( length > 0);
    (*((*i.node).prev_link[dim])).next_link[dim] = (*i.node).next_link[dim];
    (*((*i.node).next_link[dim])).prev_link[dim] = (*i.node).prev_link[dim];
    if (managed)
      put_node(i.node);
    --length;
  }
  void erase(T* pos)  { erase( iterator( pos,dim)); }

  void pop_front() { erase(begin()); }

  void pop_back() {
    iterator tmp = end();
    erase(--tmp);
  }

  void erase(iterator first, iterator last);

  void erase(T* first, T* last) {
    erase( iterator(first,dim), iterator(last,dim));
  }

  void clear() { erase( begin(), end()); }
};

template <class T, bool managed>
void In_place_list<T,managed>::
erase( _In_place_list_iterator<T> first,
       _In_place_list_iterator<T> last) {
  while (first != last)
    erase(first++);
}

RFC_END_NAME_SPACE
#endif // RFC_IN_PLACE_LIST_H //

