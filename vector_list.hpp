#ifndef VECTOR_LIST_HPP
#define VECTOR_LIST_HPP

#include <vector>
#include "managed_allocator.hpp"
#include "array.hpp"
#include "vector_list_iterator.hpp"

// TODO: pair< size_t, array<size_t, N2> >

struct last_index_holder
{
  // на самом деле last_index + 1
  size_t last_index;
};

template<typename T, int N>
struct last_index_array
  : last_index_holder
  , array<T, N>
{
  // тольк для данных, TODO: перенести в array_of_array
  size_t to_index(size_t n)
  {
    return array<T, N>::size() - (last_index_holder::last_index - n);
  }
};

/**
 * @tparam T 
 */
template<typename T, int N, typename P, typename A>
class array_of_array
{
public:
  typedef T index_type;
  typedef last_index_array<index_type, N> index_array;
  typedef A allocator;
  typedef P main_pointer;
  typedef typename allocator::pointer array_pointer;
  typedef typename allocator::value_type array_type;

  typedef typename array_type::value_type value_type;
  typedef typename array_type::reference reference;
  typedef typename array_type::const_reference const_reference;
  typedef typename array_type::pointer pointer;
  typedef typename array_type::const_pointer const_pointer;
  typedef typename array_type::iterator iterator;
  typedef typename array_type::const_iterator const_iterator;
  typedef typename array_type::reverse_iterator reverse_iterator;
  typedef typename array_type::const_reverse_iterator const_reverse_iterator;
  typedef typename array_type::difference_type difference_type;
  typedef typename array_type::size_type size_type;

  /**
   * @param main_ptr указатель на array< T, N>, где T offset
   * @param ap       прототип указателя на array< value_type, X>
   */
  array_of_array( main_pointer main_ptr, array_pointer ap, allocator a )
    : _size(0)
    , _main_array(main_ptr)
    , _array(ap)
    , _allocator(a)
  {}

  size_t last_index() const { return _main_array->last_index; }
  void last_index(size_t new_last_index) const { _main_array->new_last_index; }

  reference operator[](size_type n) { return this->at(n); }
  const_reference operator[](size_type n) const { return this->at(n); }
  
  const_reference at ( size_type n ) const { return this->_at(n); }
  reference at ( size_type n ) { return this->_at(n); }
  
  reference front ( ){ /*return _data[0];*/ }
  const_reference front ( ) const { /*return _data[0];*/ }
  reference back ( ){ /*return _data[_size-1];*/ }
  const_reference back ( ) const{ /*return _data[_size-1];*/ }

  /*
  size_type size() const { return _size;}
  size_type max_size() const { return N;}
  size_type capacity() const { return N;}
  */
  /*
  bool empty () const {return _size==0;}
  bool filled () const { return _size == N;}
  void resize ( size_type sz, T value = value_type() )
  {
    if (sz > _size)
      std::fill_n( end(), sz - _size, value );
    _size = sz;
  }
  void reserve ( size_type n ) {}
  */

  /*
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  */

  /*
  iterator begin() { return _data;}
  const_iterator begin() const { return _data;}
  iterator end() { return _data + _size;}
  const_iterator end() const { return _data + _size;}
  iterator last() { return _data + _size - 1;}
  const_iterator last() const { return _data  + _size - 1;}
  */

  /*?
  void clear()
  {
    _size = 0;
    std::fill_n( begin(), N, T() );
  }
  */


  template <class InputIterator>
  InputIterator assign ( InputIterator first, InputIterator last )
  {
    /*
    std::copy( first, last, _data );
    _size = std::distance(first, last);
    */
  }

  /** @return сколько осталось */
  size_type assign ( size_type n, const value_type& u )
  {
    /*std::fill_n( begin(), n, u );
    _size = n;
    */
  }

  /** @return false - если нет места для вставки */
  bool push_back ( const value_type& x )
  {
    size_t pred_last_index = 0;
    bool flag = false;
    if ( _main_array->empty() )
      flag = true;
    
    if ( !flag )
    {
      _array = *(_main_array->last());
      flag = _array->filled();
      pred_last_index = _array->last_index;
    }
    
    if (flag)
    {
      if ( _main_array->filled() )
        return false;
      
      _array = _allocator.allocate(1);
      _allocator.construct(_array, array_type() );
      _array->last_index  = pred_last_index;
      _main_array->push_back(_array);
      
    }
    
    _array = *(_main_array->last());
    _array->push_back(x);
    ++_array->last_index;
    ++_main_array->last_index;
    ++_size;
    return true;
  }

  void pop_back( )
  {
    if ( _main_array->empty() )
      throw std::out_of_range("pop_back");
    _array = *(_main_array->last());
    _array->pop_back( );
    if (_array->empty() )
      _main_array->pop_back( );
    else
      --_array->last_index;
  }
  
  /** Если n == this->size() то возвращает сколько из count
   *  м.б. добавленно в конец.
   *  count - если можно вставить все элементы
   *  0 - в противном случае
   */
  size_type can_insert(size_type n, size_type count)
  {
    size_type capacity_size;
    if ( n == _size )
    {
      if ( _main_array->empty() )
      {
        capacity_size = main_pointer::value_type::dimension * array_type::dimension;
        return capacity_size < count ? capacity_size : count;
      }

      _array = *(_main_array->last());
    }
    else
    {
      typename index_array::iterator itr = _find_array(n);
      if (itr==_main_array->end())
        throw std::out_of_range("array_of_array::_at");
      _array = *itr;
    }
    
    capacity_size = ( _array->capacity() - _array->size() ) + ( _main_array->capacity() - _main_array->size())*array_type::dimension;
    if ( n == _size )
      return capacity_size < count ? capacity_size : count;
    return capacity_size < count ? 0 : count;
  }

  // n - перед каким вставлять
  bool insert( size_type n, const value_type& x )
  {
    if ( n == _main_array->last_index)
      return push_back(x);
    
    typename index_array::iterator itr = _find_array(n);
    if (itr==_main_array->end())
      throw std::out_of_range("array_of_array::insert");
    
    _array = *itr;
    if ( _array->to_index(n) == 0 && itr!=_main_array->begin())
    {
      --itr;
      _array = *itr;
    }
    
    typename index_array::difference_type dist = itr - _main_array->begin();
    
    
    if ( _array->filled() )
    {
      /*if ( _main_array->filled() )
        return false;*/
      array_pointer new_array = _allocator.allocate(1);
      _allocator.construct(new_array, array_type());
      new_array = *(_main_array->insert(itr+1, new_array));
      itr = _main_array->begin() + dist;
      _array = *itr;
      size_type second_size = _array->size()/2;
      size_type first_size = _array->size() - second_size;
      bool to_second = _array->to_index(n) > first_size;
      
      new_array->resize(second_size);
      std::copy( _array->begin() + first_size, _array->end(), new_array->begin() );
      new_array->last_index = _array->last_index;
      _array->last_index -= second_size;
      _array->resize(first_size);
      if ( to_second )
      {
         ++itr;
        _array = *itr;
       
      }
    }
    
    _array->insert(_array->begin() + _array->to_index(n), x );
    
    for ( ;itr!=_main_array->end();++itr)
    {
      _array = *itr;
      ++_array->last_index;
    }
    ++_size;
    ++_main_array->last_index;
    return true;
  }

  /// @return сколько элементов было добавленно
  size_type insert ( size_type n, size_type count, const value_type& x )
  {
    if ( size_type insert_count = can_insert(n, count) )
    {
      for (size_type i=0; i < insert_count; ++i )
        this->insert( n, x);
      return insert_count;
    }
    return 0;
  }

  template <class InputIterator>
  InputIterator insert ( size_type position, InputIterator first, InputIterator last )
  {
    if ( size_type insert_count = last-first )
    {
      for (size_type i=0; i < insert_count; ++i, ++first )
      {
        if ( !insert( position+i, *first) )
          return first;
      }
    }
    return first;
  }
private:
  
  reference _at(size_type n)
  {
    typename index_array::iterator itr = _find_array(n);
    if (itr==_main_array->end())
      throw std::out_of_range("array_of_array::_at");
    _array = *itr;
    return _array->at( _array->to_index(n) );
  }

  const_reference _at(size_type n) const
  {
    typename index_array::const_iterator itr = _find_array(n);
    if (itr==_main_array->end())
      throw std::out_of_range("array_of_array::_at");
    _array = *itr;
    return _array->at( _array->to_index(n) );
  }

  typename index_array::iterator _find_array(size_type n)
  {
    return 
      std::lower_bound(
        _main_array->begin(),
        _main_array->end(),
        n,
        [this] (const typename index_array::value_type& iv, const size_type& index) -> bool
        {
          _array = iv;
          //std::cout << _array->last_index << "<" << index + 1 << std::endl;
          return _array->last_index < index + 1;
        }
      );
  }

  typename index_array::const_iterator _find_array(size_type n) const
  {
    return 
      std::lower_bound(
        _main_array->begin(),
        _main_array->end(),
        n,
        [this] (const typename index_array::value_type& iv, const size_type& index) -> bool
        {
          _array = iv;
          return _array->last_index < index + 1;
        }
      );
  }


private:
  size_type _size;
  main_pointer _main_array;
  mutable array_pointer _array;
  allocator _allocator;
};



template<
  typename T, int N1, int N2,
  typename A1 = managed_allocator< last_index_array<T,N1> >,
  typename A2 = managed_allocator< last_index_array<size_t, N2> >
>
class vector_list
{
public:
  typedef vector_list<T, N1, N2, A1, A2> self;
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

  typedef offset_iterator<T, self> iterator;
  typedef offset_iterator<const T, self> const_iterator;
  
  typedef vector_list_iterator2<index_list_range, index_range, value_range, value_pointer> iterator2;
  typedef vector_list_iterator2<const index_list_range, const index_range, const value_range, const value_pointer> const_iterator2;
  
  //typedef vector_list_iterator<>
  
  vector_list(value_allocator a1, index_allocator a2 )
    : _size(0)
    , _index_allocator(a2)
    , _value_allocator(a1)
  {
  }

  size_t size() const { return _size;}
  iterator begin()
  {
    return iterator(0, *this);
  }

  iterator end()
  {
    return iterator( size(), *this);
  }


  iterator2 begin2()
  {
    _index_pointer = *(_index_list.begin());
    _value_pointer = *(_index_pointer->begin());
    return iterator2( 
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
    bool flag1 = true;
    bool flag2 = true;
    index_list_iterator last_index = _last_index();
    _value_pointer = *((*last_index)->last());
    if (  _value_pointer->filled() )
    {
      if ( (*last_index)->filled() )
      {
        //size_t start_index = _value_pointer->start_index + _value_pointer->size();
        size_t last_index_value = _value_pointer->last_index;
        _index_pointer = _index_allocator.allocate(1);
        _index_allocator.construct(_index_pointer, typename index_allocator::value_type() );
        // _index_pointer->start_index = start_index;
        _index_pointer->last_index = last_index_value;
        _value_pointer = _value_allocator.allocate(1);
        _value_allocator.construct(_value_pointer, typename value_allocator::value_type() );
        // _value_pointer->start_index = start_index;
        _value_pointer->last_index = last_index_value;
        _index_pointer->push_back( _value_pointer );
        _index_list.push_back( _index_pointer );
        last_index = _last_index();
        //flag1 = false;
        //flag2 = false;
      }
      else
      {
        //size_t start_index = _value_pointer->start_index + _value_pointer->size();
        size_t last_index_value = _value_pointer->last_index;
        _value_pointer = _value_allocator.allocate(1);
        _value_allocator.construct(_value_pointer, typename value_allocator::value_type() );
        _value_pointer->last_index = last_index_value;
        _index_pointer->push_back(_value_pointer);
        // flag1 = false;
      }
    }

    //++ (*last_index)->total_size;
    _value_pointer->push_back(x);
    ++_size;
    if (flag1) ++_value_pointer->last_index;
    if (flag2) ++_index_pointer->last_index;
  }

  T& at(std::ptrdiff_t n)
  {
    typename index_list::iterator index_list_itr = std::lower_bound(
      _index_list.begin(),
      _index_list.end(),
      n,
      [] (const typename index_list::value_type& ilv, const size_t& index) -> bool
      {
        return ilv->last_index  < index + 1;
      }
    );

    _index_pointer = *index_list_itr;
    typename index_pointer::value_type::iterator index_itr = std::lower_bound(
      _index_pointer->begin(),
      _index_pointer->end(),
      n,
      [this] (const typename index_pointer::value_type::value_type& iv, const size_t& index) -> bool
      {
        _value_pointer = iv;
        return _value_pointer->last_index < index + 1;
      }
    );

    _value_pointer = *index_itr;
    return (*_value_pointer)[ _value_pointer->size() - (_value_pointer->last_index - n) ];
  }


  iterator insert ( iterator position, const T& x )
  {
    if ( position == this->end() )
      this->push_back(x);

    std::ptrdiff_t offset = position - this->begin();
    this->_insert( offset, x);
    /*
    std::ptrdiff_t offset = position - this->begin();
    _value_pointer = this->_split(offset);
    std::ptrdiff_t local_offset = _value_pointer->size() - (_value_pointer->last_index - offset);
    _value_pointer->insert( _value_pointer->begin() + local_offset, x );
    ++(_value_pointer->last_index);
    ++_size;
    */
    // TODO: инкрементировать у всех последующих
    return position;
    /*_value_pointer = _find_value_pointer( offset );
    if ( _value_pointer->filled() )
    {
      this->_split(offset);
      _value_pointer = _find_value_pointer( offset );
    }
    */
    
  }

protected:
  /*value_pointer*/void _insert( std::ptrdiff_t n, const T& x )
  {
    // Расщипляем при заполненни

    typename index_list::iterator index_list_itr = std::lower_bound(
      _index_list.begin(),
      _index_list.end(),
      n,
      [] (const typename index_list::value_type& ilv, const size_t& index) -> bool
      {
        return ilv->last_index  < index + 1;
      }
    );

    _index_pointer = *index_list_itr;
    typename index_pointer::value_type::iterator index_itr = std::lower_bound(
      _index_pointer->begin(),
      _index_pointer->end(),
      n,
      [this] (const typename index_pointer::value_type::value_type& iv, const size_t& index) -> bool
      {
        _value_pointer = iv;
        return _value_pointer->last_index < index + 1;
      }
    );
    _value_pointer = *index_itr;

    if ( _value_pointer->filled() )
    {
      value_pointer filled_value_pointer = _value_pointer;
      if ( _index_pointer->filled() )
      {
        // разбиваем основной вектор
        std::ptrdiff_t pos = _index_list.end() - index_list_itr;
        _index_list.resize( _index_list.size() + 1 );
        std::copy_backward( _index_list.begin() + pos, _index_list.begin() + _index_list.size()-1 , _index_list.begin() + pos + 1 );
        index_list_itr = _index_list.begin() + pos;
        _index_pointer = _index_allocator.allocate(1);
        _index_allocator.construct(_index_pointer, typename index_allocator::value_type() );
        _index_list[pos] = _index_pointer;
        _value_pointer = _value_allocator.allocate(1);
        _value_allocator.construct(_value_pointer, typename value_allocator::value_type() );
        //??? _value_pointer->last_index = last_index_value;
        _index_pointer->push_back( _value_pointer );
      }
      else
      {
        // разбиваем массив масивов
        std::ptrdiff_t pos = _index_pointer->end() - index_itr;
        _index_pointer->resize( _index_pointer->size() + 1 );
        std::copy_backward( _index_pointer->begin() + pos, _index_pointer->begin() + _index_pointer->size()-1 , _index_pointer->begin() + pos + 1 );
        index_itr = _index_pointer->begin() + pos;
        _value_pointer = _value_allocator.allocate(1);
        (*_index_pointer)[pos] = _value_pointer;
      }

      // разбиваем основной масив
      std::ptrdiff_t pos = filled_value_pointer->size() - ( filled_value_pointer->last_index -n );
      _value_pointer->resize( filled_value_pointer->size() - n );
      std::copy( filled_value_pointer->begin() + pos, filled_value_pointer->end(), _value_pointer->begin() );
      filled_value_pointer->resize(pos);
      _value_pointer->last_index = filled_value_pointer->last_index;
      filled_value_pointer->last_index -= _value_pointer->size();
    }

    
    std::ptrdiff_t local_offset = _value_pointer->size() - (_value_pointer->last_index - n);
    _value_pointer->insert( _value_pointer->begin() + local_offset, x );
    ++_size;
    std::for_each( index_list_itr, _index_list.end(), [this]( typename index_list::value_type& iv)
    {
      _index_pointer = iv;
      ++iv->last_index;
      std::for_each( _index_pointer->begin(), _index_pointer->end(), [this](typename index_pointer::value_type::value_type& ivv )
      {
        _value_pointer = ivv;
        ++_value_pointer->last_index;
      });
    });
    
    //return _value_pointer;
  }
  
  value_pointer _find_value_pointer(std::ptrdiff_t n)
  {
    typename index_list::iterator index_list_itr = std::lower_bound(
      _index_list.begin(),
      _index_list.end(),
      n,
      [] (const typename index_list::value_type& ilv, const size_t& index) -> bool
      {
        return ilv->last_index  < index + 1;
      }
    );

    _index_pointer = *index_list_itr;
    typename index_pointer::value_type::iterator index_itr = std::lower_bound(
      _index_pointer->begin(),
      _index_pointer->end(),
      n,
      [this] (const typename index_pointer::value_type::value_type& iv, const size_t& index) -> bool
      {
        _value_pointer = iv;
        return _value_pointer->last_index < index + 1;
      }
    );
    _value_pointer = *index_itr;
    return _value_pointer;
  }
  
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
  size_t          _size;
  index_list      _index_list;
  index_allocator _index_allocator;
  value_allocator _value_allocator;
  index_pointer   _index_pointer;
  value_pointer   _value_pointer;
};
#endif
