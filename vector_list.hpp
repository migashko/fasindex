#ifndef VECTOR_LIST_HPP
#define VECTOR_LIST_HPP

#include <vector>
#include "managed_allocator.hpp"
#include "array.hpp"
#include "vector_list_iterator.hpp"

// TODO: pair< size_t, array<size_t, N2> >




template<typename T, int N1, int N2, typename A1 = managed_allocator< array<T,N1> >, typename A2 = managed_allocator< array<size_t, N2> > >
class vector_list
{
public:
  typedef T value_type;
  typedef A1 value_allocator;
  typedef A2 index_allocator;

  typedef typename index_allocator::pointer index_pointer;
  typedef typename value_allocator::pointer value_pointer;
  

  typedef std::vector<index_pointer> index_list;
  typedef typename index_list::iterator index_list_iterator;
  
  typedef typename fas::typerange<index_list>::range index_list_range;
  typedef typename fas::typerange< array<size_t, N2> >::range index_range;
  typedef typename fas::typerange< array<T,N1> >::range value_range;
  
  typedef vector_list_iterator<index_list_range, index_range, value_range, value_pointer> iterator;
  typedef vector_list_iterator<const index_list_range, const index_range, const value_range, const value_pointer> const_iterator;
  
  //typedef vector_list_iterator<>
  
  vector_list(value_allocator a1, index_allocator a2 )
    : _index_allocator(a2)
    , _value_allocator(a1)
  {
  }
  

  iterator begin()
  {
    _index_pointer = *(_index_list.begin());
    _value_pointer = *(_index_pointer->begin());
    return iterator( 
      index_list_range(_index_list.begin(), _index_list.end() ),
      index_range( _index_pointer->begin(), _index_pointer->end() ),
      value_range( _value_pointer->begin(), _value_pointer->end() ),
      _value_pointer
    );
  }

  /*
  iterator end()
  {
    _index_pointer = *(_index_list.begin());
    _value_pointer = *(_index_pointer->begin());
    return iterator(
      fas::erange(_index_list),
      fas::erange( *_index_pointer ),
      fas::range( _value_pointer->end(), _value_pointer->end() )
    );
  }
  */

  void push_back( const T& x )
  {
    index_list_iterator last_index = _last_index();
    _value_pointer = *((*last_index)->last());
    if (  _value_pointer->filled() )
    {
      if ( (*last_index)->filled() )
      {
        _index_pointer = _index_allocator.allocate(1);
        _index_allocator.construct(_index_pointer, typename index_allocator::value_type() );
        _value_pointer = _value_allocator.allocate(1);
        _value_allocator.construct(_value_pointer, typename value_allocator::value_type() );
        _index_pointer->push_back( _value_pointer );
        _index_list.push_back( _index_pointer );
        last_index = _last_index();
      }
      else
      {
        _value_pointer = _value_allocator.allocate(1);
        _value_allocator.construct(_value_pointer, typename value_allocator::value_type() );
        _index_pointer->push_back(_value_pointer);
      }
    }

    _value_pointer->push_back(x);
    
  }
  
protected:
  index_list_iterator _last_index()
  {
    if ( _index_list.empty() )
    {
      //std::cout << "create first chunk index" << std::endl;
      _index_pointer = _index_allocator.allocate(1);
      //std::cout << "create first chunk data" << std::endl;
      _value_pointer = _value_allocator.allocate(1);
      _index_pointer->push_back( _value_pointer );
      _index_list.push_back( _index_pointer );
    }
    return (++_index_list.rbegin()).base();
  }

private:
  index_list      _index_list;
  index_allocator _index_allocator;
  value_allocator _value_allocator;
  index_pointer   _index_pointer;
  value_pointer   _value_pointer;
};
#endif
