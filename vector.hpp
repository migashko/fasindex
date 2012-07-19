#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <vector>
#include "managed_allocator.hpp"
#include "array.hpp"
#include "array_proxy.hpp"
#include "offset_iterator.hpp"
#include "next_index_holder.hpp"
#include "value_array.hpp"
#include "index_array.hpp"


template<typename T>
struct allocator_helper
{
  template< int N1, int N2 >
  struct mmap
  {
    typedef mmap_buffer buffer_type;
    typedef value_array<T,N1> value_array_type;
    typedef index_array<size_t, N2> index_array_type;
    
    typedef fixed_size_blocks_allocation<index_array_type, buffer_type> index_allocate_manager;
    typedef fixed_size_blocks_allocation<value_array_type, buffer_type> value_allocate_manager;

    typedef managed_allocator<value_array_type, value_allocate_manager> value_allocator;
    typedef managed_allocator<index_array_type, index_allocate_manager> index_allocator;
  };
  //template<typename T, typename AllocateManager = fixed_size_blocks_allocation<T, mmap_buffer> >
  /*
  typedef A1 value_allocator;
  typedef A2 index_allocator;

  vector_allocator(value_allocator a1, index_allocator a2 )
  {
  }
  */
  

};

template<
  typename T, /*int N1, int N2,*/
  typename A1 /*= managed_allocator< value_array<T,N1> >*/,
  typename A2 /*= managed_allocator< index_array<size_t, N2> >*/
>
class vector
{
  typedef vector<T, /*N1, N2,*/ A1, A2> self;
public:
  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;
  
  typedef A1 value_allocator;
  typedef A2 index_allocator;
  typedef typename index_allocator::pointer    index_pointer;
  typedef typename index_allocator::value_type index_array;
  typedef typename index_allocator::size_type  size_type;

  typedef typename value_allocator::pointer   value_pointer;
  
  typedef array_proxy<index_pointer, value_allocator> index_wrapper;
  typedef std::vector<index_wrapper> vector_of_index;
  
  typedef offset_iterator<T, self> iterator;
  typedef offset_iterator<const T, self> const_iterator;

  vector(value_allocator a1, index_allocator a2 )
    : _size(0)
    , _index_allocator(a2)
    , _value_allocator(a1)
  {
    /*
    _value_pointer = _value_allocator.allocate(1);
    _value_allocator.deallocate(_value_pointer,1);
    */
  }

  size_t size() const { return _size;}

  iterator begin() { return iterator(0, *this);}
  const_iterator begin() const { return const_iterator(0, *this);}

  iterator end() { return iterator( size(), *this);}
  const_iterator end() const { return const_iterator( size(), *this);}

  reference at(size_type position) { return this->_at(position); }
  const_reference at(size_type position) const { return this->_at(position); }

  reference operator[](size_type position) { return this->at(position); }
  const_reference operator[](size_type position) const { return this->at(position); }


  void push_back(const_reference x)
  {
    typename vector_of_index::iterator itr = _last_vector_of_index();

    if ( itr!=_vector_of_index.end() )
    {
      if ( itr->filled() )
      {
        index_wrapper iw = _create_vector_of_index();
        _vector_of_index.push_back( iw );
      }
      itr = _vector_of_index.end();
    }
    else
    {
      _vector_of_index.push_back( _create_vector_of_index() );
      itr = _vector_of_index.end();
    }

    if ( itr == _vector_of_index.end() )
      itr = _last_vector_of_index();

    itr->push_back(x);
    ++_size;
  }

private:

  reference _at(size_type position)
  {
    typename vector_of_index::iterator itr = _find_index(position);
    if ( itr == _vector_of_index.end() )
      throw std::out_of_range("vector_list::_at");

    return itr->at(position);
  }

  const_reference _at(size_type position) const
  {
    typename vector_of_index::const_iterator itr = _find_index(position);
    if ( itr == _vector_of_index.end() )
      throw std::out_of_range("vector_list::_at");

    return itr->at(position);
  }
   
  typename vector_of_index::iterator _find_index(size_type position)
  {
    
    // Можно указать следующий за последним 
    if ( position > _size )
      throw std::out_of_range("vector_list::_find_index");
    
    return std::lower_bound(
      _vector_of_index.begin(),
      _vector_of_index.end(),
      position,
      [] (const typename vector_of_index::value_type& ilv, const size_type& index) -> bool
      {
        return ilv.last_index()  < index + 1;
      }
    );
  }

  typename vector_of_index::const_iterator _find_index(size_type position) const
  {
    // Можно указать следующий за последним
    if ( position > _size )
      throw std::out_of_range("vector_list::_find_index");

    return std::lower_bound(
      _vector_of_index.begin(),
      _vector_of_index.end(),
      position,
      [] (const typename vector_of_index::value_type& ilv, const size_type& index) -> bool
      {
        return ilv->last_index  < index + 1;
      }
    );
  }

  typename vector_of_index::iterator _last_vector_of_index()
  {
    
    if ( _vector_of_index.empty() )
      return _vector_of_index.end();

    return (++_vector_of_index.rbegin()).base();
    
  }

  index_wrapper _create_vector_of_index()
  {
    _index_pointer = _index_allocator.allocate(1);
    _index_allocator.construct(_index_pointer, index_array() );
    return index_wrapper(_index_pointer, /*_value_pointer,*/ _value_allocator);
  }
  


private:
  size_t          _size;
  index_allocator _index_allocator;
  value_allocator _value_allocator;
  vector_of_index _vector_of_index;
  // Служебные переменные 
  mutable index_pointer   _index_pointer;
  // mutable value_pointer   _value_pointer;

};

#endif
