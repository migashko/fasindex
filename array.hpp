#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <algorithm>
#include <cstddef>


template<typename T, size_t N>
class array
{
public:
  typedef T value_type;
  typedef T data_type[N];
  typedef size_t size_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef pointer iterator;
  typedef const_pointer const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef const std::reverse_iterator<iterator> const_reverse_iterator;
  typedef std::ptrdiff_t difference_type;
  
  array():_size(0) {};
  
  reference operator[](size_type n) { return _data[n]; }
  const_reference operator[](size_type n) const { return _data[n]; }
  const_reference at ( size_type n ) const { return _data[n]; }
  reference at ( size_type n ) { return _data[n]; }
  reference front ( ){ return _data[0]; }
  const_reference front ( ) const{ return _data[0]; }
  reference back ( ){ return _data[_size-1]; }
  const_reference back ( ) const{ return _data[_size-1]; }

  size_type size() const { return _size;}
  size_type max_size() const { return N;}
  size_type capacity() const { return N;}
  bool empty () const {return _size==0;}
  bool filled () const { return _size == N;}
  void resize ( size_type sz, T value = value_type() ) 
  {
    if (sz > _size)
      std::fill_n( end(), sz - _size, value );
    _size = sz;
  }
  void reserve ( size_type n ) {}
  
  reverse_iterator rbegin() { return reverse_iterator(end()/*+_size-1*/); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(/*begin()+_size-1*/end()); }

  reverse_iterator rend() { return reverse_iterator(/*begin()-1*/begin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(/*begin()-1*/begin()); }

  iterator begin() { return _data;}
  const_iterator begin() const { return _data;}
  iterator end() { return _data + _size;}
  const_iterator end() const { return _data + _size;}
  iterator last() { return _data + _size - 1;}
  const_iterator last() const { return _data  + _size - 1;}
  
  
  template <class InputIterator>
  void assign ( InputIterator first, InputIterator last )
  {
    std::copy( first, last, _data );
    _size = std::distance(first, last);
  }
  
  void assign ( size_type n, const T& u )
  {
    std::fill_n( begin(), n, u );
    _size = n;
  }
  
  void push_back ( const T& x )
  {
    _data[_size++] = x;
  }
  
  void pop_back ( )
  {
    --_size;
  }
  
  iterator insert ( iterator position, const T& x )
  {
    std::copy_backward(position, end(), end()+1);
    *position = x;
    ++_size;
  }
  
  void insert ( iterator position, size_type n, const T& x )
  {
    std::copy_backward(position, end(), end()+n);
    std::fill_n(position, n, x);
    _size+=n;
  }
  
  template <class InputIterator>
  void insert ( iterator position, InputIterator first, InputIterator last )
  {
    std::copy_backward(position, end(), end()+std::distance(first,last) );
    std::copy(first, last, position);
    _size+=std::distance(first,last);
  }
  
private:
  size_type _size;
  data_type _data;
};
#endif
