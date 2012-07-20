#ifndef ARRAY_PROXY_HPP
#define ARRAY_PROXY_HPP

#include <stdexcept>
#include <numeric>


/**
 * @tparam T
 */
template<typename P, typename A>
class array_proxy
{
public:
  typedef array_proxy<P, A> self;
  typedef P main_pointer;
  typedef typename main_pointer::value_type index_array;
  typedef typename index_array::value_type index_type;
  typedef A allocator;

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
  array_proxy( main_pointer main_ptr, /*array_pointer ap,*/ allocator a )
    : _size(0)
    , _main_array(main_ptr)
    //, _array(ap)
    , _allocator(a)
  {
    _array = _allocator.allocate(1);
    _allocator.deallocate( _array, 1);
  }

  array_proxy(const self& aoa)
    : _size(aoa._size)
    , _main_array(aoa._main_array)
    , _array(aoa._array)
    , _allocator(aoa._allocator)
  {
  }

  void operator = (const self& aoa)
  {
    _size = aoa._size;
    _main_array = aoa._main_array;
    _array = aoa._array;
    _allocator = aoa._allocator;
  }


  size_t next_index() const { return _main_array->next_index(); }
  void next_index(size_t new_last_index) { _main_array->next_index( new_last_index ); }

  reference operator[](size_type n) { return this->at(n); }
  const_reference operator[](size_type n) const { return this->at(n); }

  const_reference at ( size_type n ) const { return this->_at(n); }
  reference at ( size_type n ) { return this->_at(n); }

  reference front ( ){ /*return _data[0];*/ }
  const_reference front ( ) const { /*return _data[0];*/ }
  reference back ( ){ /*return _data[_size-1];*/ }
  const_reference back ( ) const{ /*return _data[_size-1];*/ }

  bool filled () const { return _main_array->filled();}


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

  void restore()
  {
    _size = std::accumulate(
      _main_array->begin(),
      _main_array->end(),
      0,
      [this](size_t value, const index_type& ind) -> size_t
      {
        _array = ind;
        return value + _array->size();
      }
    );
  }

  void check_next_index()
  {
    /*
    size_t current = 0;
    std::for_each(
      _main_array->begin(),
      _main_array->end(),
      [this, &current](index_type& ind)
      {
        _array = ind;
        if ( _array->next_index() < current )
        {
          this->show_last_index();
          throw std::logic_error("Ахтунг");
        }
        current = _array->next_index();
      }
    );
    */
  }


  void show_last_index()
  {
    std::for_each(
      _main_array->begin(),
      _main_array->end(),
      [this](index_type& ind)
      {
        _array = ind;
        std::cout << _array->next_index() << "->";
      }
    );
    std::cout << std::endl;
  }

  void clear()
  {
    std::for_each(
      _main_array->begin(),
      _main_array->end(),
      [this](index_type& ind)
      {
        _array = ind;
        _array->clear();
        _array->next_index(0);
        _allocator.deallocate(_array, 1);
      }
    );
    _main_array->clear();
    _main_array->next_index(0);
    _size = 0;

  }




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
    check_next_index();
    //size_t pred_last_index = 0;
    bool flag = false;
    if ( _main_array->empty() )
      flag = true;

    if ( !flag )
    {
      _array = *(_main_array->last());
      flag = _array->filled();
      //pred_last_index = _array->next_index;
    }

    if (flag)
    {
      if ( _main_array->filled() )
        return false;

      _array = _allocator.allocate(1);
      _allocator.construct(_array, array_type() );
      _array->next_index( _main_array->next_index() );
      _main_array->push_back(_array);

    }

    _array = *(_main_array->last());
    _array->push_back(x);
    _array->inc_next_index();
    _main_array->inc_next_index();
    ++_size;
    check_next_index();
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
 /* size_type can_insert(size_type n, size_type count)
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
  }*/

  // n - перед каким вставлять
  bool insert( size_type n, const value_type& x )
  {
    if ( n == _main_array->next_index() )
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
      new_array->next_index( _array->next_index() );
      _array->dec_next_index(  second_size );
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
      _array->inc_next_index();
    }
    ++_size;
    _main_array->inc_next_index();
    return true;
  }

  /// @return сколько элементов было добавленно
  size_type insert ( size_type n, size_type count, const value_type& x )
  {
    size_type i = 0;
    for (; i < count; ++i )
      if ( this->insert( n, x) )
        return i;
    return i;
  }

  template <class InputIterator>
  InputIterator insert ( size_type position, InputIterator first, InputIterator last )
  {
    /// TODO: Это не эффективно
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

  size_type erase ( size_type position )
  {
    typename index_array::iterator itr = _find_array(position);
    if (itr==_main_array->end())
      throw std::out_of_range("array_of_array::insert");

    typename index_array::difference_type dist = itr - _main_array->begin();
    _array = *itr;
    _array->erase( _array->begin() + _array->to_index(position) );
    if ( _array->empty() )
    {
      _main_array->erase( itr++ );
      itr = _main_array->begin() + dist;
    }
    typename index_array::iterator end = _main_array->end();
    for ( ;itr!=end;++itr)
    {
      _array = *itr;
      _array->dec_next_index();
    }
    --_size;
    _main_array->dec_next_index();

    return 1;
  }

  /** @return скотлько удалил элементов */
  size_type erase ( size_type first, size_type last )
  {
    /// TODO: Это мега не эффективно
    size_type count = last - first;
    for ( size_type i = 0; i < count; ++i  )
      if ( !this->erase(first) )
        return i;
    return count;
    /*
    typename index_array::iterator itr = _find_array(first);
    if (itr==_main_array->end())
      throw std::out_of_range("array_of_array::insert");

    //typename index_array::difference_type dist = itr - _main_array->begin();
    _array = *itr;

    size_type count = last - first;
    size_type total = 0;

    size_type current_index = _array->to_index(first);
    while (total!=count && itr!=_main_array->end())
    {
      _array = *itr;

      size_type current_count = count - total;
      if ( _array->size() - current_index < current_count )
        current_count = _array->size() - current_index;
      _array->erase( _array->begin() + current_index , _array->begin() + current_index  + current_count );
      total += current_count;
      if ( _array->empty() )
        _main_array->erase( itr++ );
      else
      {
        _array->last_index -= count;
        ++itr;
      }
      current_index = 0;

    }

    for ( ;itr!=_main_array->end();++itr)
    {
      _array = *itr;
      _array->last_index -=total ;
    }

    _size-=total;
    _main_array->last_index-=total;


    return total;
    */

    /*
    size_type count = last - first;

    size_type current_offset = _array->to_index( first );
    size_type current_count = _array->size() - current_offset;
    if ( current_count < count)
      current_count = count;

    while ( count > 0 )
    {
      _array->erase( _array->begin() + current_offset, _array->begin() + current_offset + current_count );
      count-=
    }
    */





    /*
      difference_type dist = last - first;
      std::copy( last, this->end(), first);
      this->resize( _size - dist );
    */
  }


  main_pointer get_pointer() { return _main_array;}

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
          //std::cout << _main_array->size() << " _find_array: " << _array->next_index << "<" << index + 1 << std::endl;
          return _array->next_index() < index + 1;
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
          return _array->next_index < index + 1;
        }
      );
  }


private:
  size_type _size;
  main_pointer _main_array;
  mutable array_pointer _array;
  allocator _allocator;
};

#endif
