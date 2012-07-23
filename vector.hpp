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
    _index_pointer = _index_allocator.allocate(1);
    _index_allocator.deallocate(_index_pointer, 1);
    /*
    _value_pointer = _value_allocator.allocate(1);
    _value_allocator.deallocate(,1);
    */
  }

  size_t size() const { return _size;}
  bool empty() const { return _size == 0;}

  iterator begin() { return iterator(0, *this);}
  const_iterator begin() const { return const_iterator(0, *this);}

  iterator end() { return iterator( size(), *this);}
  const_iterator end() const { return const_iterator( size(), *this);}

  reference at(size_type position) { return this->_at(position); }
  const_reference at(size_type position) const { return this->_at(position); }

  reference operator[](size_type position) { return this->at(position); }
  const_reference operator[](size_type position) const { return this->at(position); }

  void clear()
  {
    std::for_each(
      _vector_of_index.begin(),
      _vector_of_index.end(),
      [this](typename vector_of_index::value_type& proxy)
      {
        proxy.clear();
        _index_allocator.deallocate( proxy.get_pointer(), 1);
      }
    );
    _vector_of_index.clear();
    _size = 0;
  }


  void push_back(const_reference x)
  {
    this->check_next_index();
    typename vector_of_index::iterator itr = _last_vector_of_index();

    if ( itr!=_vector_of_index.end() )
    {

      if ( itr->filled() )
      {

        this->check_next_index();

        index_wrapper iw = _create_vector_of_index();

        this->check_next_index();
        iw.next_index( _vector_of_index.back().next_index() );

        _vector_of_index.push_back( iw );
        this->check_next_index();

      }
      itr = _vector_of_index.end();
    }
    else
    {

      index_wrapper iw = _create_vector_of_index();
      iw.next_index( 0 );
      _vector_of_index.push_back( iw );
      itr = _vector_of_index.end();
    }

    if ( itr == _vector_of_index.end() )
    {

      itr = _last_vector_of_index();
    }


    this->check_next_index();
    itr->push_back(x);

    ++_size;
    this->check_next_index();
    this->debug_check_size();
  }

  iterator insert(iterator itr, const value_type& vt)
  {
    size_type position = itr - begin();
    
    if ( itr == this->end() )
    {
      this->push_back( vt );
      return begin() + position;
    }

    typename vector_of_index::iterator index_itr = _find_index( position );
    if ( index_itr == _vector_of_index.end() )
      throw std::out_of_range("vector::insert");

    if ( !index_itr->insert(position, vt) )
    {
      std::cout << "main split" << vt.index << std::endl;
      // требуется разбиение;
      index_wrapper new_index = _create_vector_of_index();
      index_itr->split(new_index);
      _vector_of_index.insert(index_itr+1, new_index);

      index_itr = _find_index( position );
      if ( index_itr == _vector_of_index.end() )
        throw std::out_of_range("vector::insert fatal");

      if ( !index_itr->insert(position, vt) )
        throw std::out_of_range("vector::insert fatal 2");
    }

    ++_size;
    // Обновляем все следующие 
    for ( ++index_itr ;index_itr!=_vector_of_index.end(); ++index_itr)
      index_itr->inc_next_index();
    
    return begin() + position;
  }

  
  template<typename Iterator>
  void restore(Iterator beg, Iterator end)
  {
    std::cout << "pre restore: " << end - beg << std::endl;
    std::cout << "pre restore: " << _size << std::endl;
    for ( int i = 0 ;beg!=end; ++beg, ++i)
    {
      //_index_pointer = beg.offset;
      _vector_of_index.push_back( index_wrapper(/*_index_pointer*/beg, _value_allocator) );
      _vector_of_index.back().restore();
      _size += _vector_of_index.back().size();

      //_index_pointer = _vector_of_index.back().back();
      //std::cout << i << " restore: " << _size << " index=" << (begin() + (_size - 1))->index  << std::endl;
    }

    std::sort(
      _vector_of_index.begin(),
      _vector_of_index.end(),
      [](const typename vector_of_index::value_type& left, const typename vector_of_index::value_type& right) -> bool
      {
        return left.next_index() < right.next_index();
      }
    );
  }

  void debug_check_size()
  {
    size_t size = 0;
    std::for_each(
      _vector_of_index.begin(),
      _vector_of_index.end(),
      [this, &size](typename vector_of_index::value_type& proxy)
      {
        size += proxy.size();
        proxy.debug_check_size();
      }
    );
    
    if ( size!=_size)
      throw std::logic_error("vector::debug_check_size()");
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

public:

  void check_next_index()
  {

    std::for_each(
      _vector_of_index.begin(),
      _vector_of_index.end(),
      [] (typename vector_of_index::value_type& ilv)
      {
        ilv.check_next_index();
      }
    );

  }


  void show_last_index()
  {
    std::cout << "show_last_index()" << std::endl;
    std::for_each(
      _vector_of_index.begin(),
      _vector_of_index.end(),
      [] (const typename vector_of_index::value_type& ilv)
      {
        std::cout << ilv.next_index() << " ";
      }
    );
    std::cout << std::endl;
  }

  void show_last_index(size_type position)
  {
    std::cout << "show_last_index("<< position <<")" << std::endl;
    typename vector_of_index::iterator itr = _find_index(position);
    itr->show_last_index();

  }

private:
  typename vector_of_index::iterator _find_index(size_type position)
  {
    // Можно указать следующий за последним
    if ( position >= _size )
      throw std::out_of_range("vector_list::_find_index");

    return std::lower_bound(
      _vector_of_index.begin(),
      _vector_of_index.end(),
      position,
      [] (const typename vector_of_index::value_type& ilv, const size_type& index) -> bool
      {
        //std::cout << "debug: " << ilv.last_index() << " < " << index + 1 << std::endl;
        return ilv.next_index()  < index + 1;
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
