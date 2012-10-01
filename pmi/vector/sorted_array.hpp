#ifndef SORTED_ARRAY_HPP
#define SORTED_ARRAY_HPP

#include <pmi/array.hpp>

// TODO: сделать операции < > и пр.
template<typename T, size_t N, typename Compare = std::less<T> >
class sorted_array
  : public array<T, N>
{
  typedef array<T, N> super;

public:
  using super::erase;
  using super::insert;
  /*
  enum { dimension = N};
  typedef T value_type;
  typedef size_t size_type;
  typedef const T& reference;
  typedef const T& const_reference;
  typedef const T* pointer;
  typedef const T* const_pointer;
  typedef pointer iterator;
  typedef const_pointer const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef const std::reverse_iterator<iterator> const_reverse_iterator;
  typedef std::ptrdiff_t difference_type;

  typedef  Compare comparator;
  */

  typedef Compare key_compare;
  typedef Compare value_compare;


  typedef typename super::value_type value_type;
  typedef typename super::data_type data_type;
  typedef typename super::size_type size_type;

  typedef typename super::reference reference;
  typedef typename super::const_reference const_reference;

  typedef typename super::pointer pointer;
  typedef typename super::const_pointer const_pointer;

  typedef typename super::const_iterator const_iterator;
  typedef typename super::iterator iterator;

  typedef typename super::reverse_iterator reverse_iterator;
  typedef typename super::const_reverse_iterator const_reverse_iterator;

  typedef std::ptrdiff_t difference_type;


  sorted_array( value_compare cmp = value_compare() )
    : super()
    , _comparator(cmp)
  {};

  /*
  const_reference operator[](size_type n) const
  {
    return super::at(n);
  }

  const_reference at ( size_type n ) const  // __attribute__ ((noinline))
  {
    return super::at(n);
  }

  const_reference front ( ) const
  {
    return super::front();
  }

  const_reference back ( ) const
  {
    return super::back();
  }

  size_type size() const
  {
    return super::size();
  }

  size_type max_size() const
  {
    return super::max_size();
  }

  size_type capacity() const
  {
    return super::capacity();
  }

  bool empty() const
  {
    return super::empty();
  }

  bool filled() const
  {
    return super::filled();
  }
  */

  void resize ( size_type sz, T value = value_type() )
  {
    super::resize(sz, value);
    this->sort();
  }

  /*
  void reserve( size_type n )
  {
    return super::reserve();
  }

  const_reverse_iterator rbegin() const
  {
    return super::rbegin();
  }

  const_reverse_iterator rend() const
  {
    return super::rend();
  }


  const_iterator begin() const
  {
    return super::begin();
  }

  const_iterator end() const
  {
    return super::end();
  }

  const_iterator last() const
  {
    return super::last();
  }

  void clear()
  {
    return super::clear();
  }
  */

  template <class InputIterator>
  void assign( InputIterator first, InputIterator last )
  {
    super::assign(first, last);
    this->sort();
  }

  /*
  void assign ( size_type n, const T& u )
  {
    super::assign(n, u);
  }

  void pop_back( )
  {
    super::pop_back();
  }
  */

  void push_back ( const T& x )
  {
    super::push_back(x);
    this->sort();
  }

  iterator insert ( const T& x )
  {
    iterator position = std::upper_bound(super::begin(), super::end(), x, _comparator );
    return super::insert(position, x);
  }

  void insert ( size_type n, const T& x )
  {
    iterator position = std::upper_bound(super::begin(), super::end(), x, _comparator );
    return super::insert(position, n, x);
  }

  size_type erase( const T& x )
  {
    size_type count = 0;
    std::pair<iterator, iterator> range = std::equal_range(super::begin(), super::end(), x, _comparator );
    for (;range.first!=range.second;++range.first, ++count)
      super::erase(range.first);
    return count;
    //return super::erase(position);
  }

  template <class InputIterator>
  void insert ( InputIterator first, InputIterator last )
  {
    super::insert( super::end(), first, last );
    this->sort();
  }

  void sort()
  {
    std::sort( super::begin(), super::end(), _comparator );
  }

  /*
  iterator erase ( iterator position )
  {
    return super::erase(position);
  }

  iterator erase ( iterator first, iterator last)
  {
    return super::erase(first, last);
  }*/

private:
  value_compare _comparator;
};

/*
#include <map>
*/


/*
template<
  size_t InitialStep,
  typename T,
  class Compare = less<size_t>,
  class Allocator = std::allocator< std::pair<const size_t, T> > >,
  template<class, class, class, class > class Map = std::map
>
class map_array
{
  typedef Map<size_t, ValueType, Compare, Allocator> map_type;
public:
  typedef size_t key_type;
  typedef T mapped_type;

  void insert( key_type key, mapped_type value )
  {

  }

private:
  map_type _map;
  size_t
};
*/


#endif