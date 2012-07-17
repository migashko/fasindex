#ifndef VECTOR_LIST_ITERATOR_HPP
#define VECTOR_LIST_ITERATOR_HPP

template<typename T, typename Container>
class offset_iterator
{
public:
  typedef offset_iterator<T, Container> self;
  typedef Container container_type;
  typedef typename std::iterator_traits<T*>::iterator_category iterator_category;
  typedef typename std::iterator_traits<T*>::value_type value_type;
  typedef typename std::iterator_traits<T*>::difference_type difference_type;
  typedef typename std::iterator_traits<T*>::pointer pointer;
  typedef typename std::iterator_traits<T*>::reference reference;

  offset_iterator(difference_type offset, container_type& container)
    : _offset(offset), _container(&container)
  {}

  value_type* operator->()
  {
    return &(_container->at(_offset));
  }

  self& operator++()
  {
    ++_offset;
    return *this;
  }

  self operator++(int)
  {
    self ans = *this;
    ++_offset;
    return ans;
  }

  self& operator--()
  {
    --_offset;
    return *this;
  }

  self operator--(int)
  {
    self ans = *this;
    --_offset;
    return ans;
  }

  self& operator += (difference_type n )
  {
    _offset+=n;
    return *this;
  }

  self& operator -= (difference_type n )
  {
    _offset-=n;
    return *this;
  }

  bool operator == (const self& r ) const
  {
    return _offset == r._offset;
  }

  bool operator != (const self& r ) const
  {
    return !this->operator == (r);
  }

  bool operator < (const self& r ) const
  {
    return _offset < r._offset;
  }

  bool operator > (const self& r ) const
  {
    return _offset > r._offset;
  }

  bool operator <= (const self& r ) const
  {
    return !this->operator >( r );
  }

  bool operator >= (const self& r ) const
  {
    return !this->operator < ( r );
  }

  reference operator[] ( difference_type n ) const
  {
    return _container->at( _offset + n );
  }

  
  template<typename TT, typename MM>
  friend typename offset_iterator<TT, MM>::difference_type operator - ( offset_iterator<TT, MM> r1, offset_iterator<TT, MM> r2 );

private:
  difference_type _offset;
  container_type* _container;
};

template<typename T, typename M, typename Dist>
inline offset_iterator<T, M> operator +
  (
    offset_iterator<T, M> r,
    Dist n
  )
{
  return r+=n;
}

template<typename T, typename M, typename Dist>
inline offset_iterator<T, M> operator +
  (
    Dist n,
    offset_iterator<T, M> r
  )
{
  return r+= n;
}

template<typename T, typename M, typename Dist>
inline offset_pointer<T, M> operator -
  (
    offset_iterator<T, M> r,
    Dist n
  )
{
  return r-= n;
}

template<typename T, typename M, typename Dist>
inline offset_iterator<T, M> operator -
  (
    Dist n,
    offset_iterator<T, M> r
  )
{
  return r -= n;
}

template<typename T, typename M>
inline typename offset_iterator<T, M>::difference_type operator -
  (
    offset_iterator<T, M> r1,
    offset_iterator<T, M> r2
  )
{
  return r1._offset - r2._offset;
}


template<
  typename IndexListRange,
  typename IndexRange,
  typename ValueRange,
  typename IndexPointerType
>
class vector_list_iterator2
{
public:
  typedef vector_list_iterator2<IndexListRange, IndexRange, ValueRange, IndexPointerType> self;
  typedef IndexListRange index_list_range;
  typedef IndexRange index_range;
  typedef ValueRange value_range;
  typedef IndexPointerType index_pointer;

  typedef typename index_list_range::value_type index_list_value;
  typedef typename index_range::value_type index_value;
  typedef typename value_range::value_type value_type;
  typedef typename value_range::pointer pointer;
  typedef typename value_range::reference reference;
  typedef typename value_range::difference_type difference_type;
  

  vector_list_iterator2(index_list_range ilr, index_range ir, value_range vr, index_pointer index_ptr)
    : _ilr(ilr), _ir(ir), _vr(vr), _index_ptr(index_ptr)
  {}
   
  value_type* operator->() { return &(*_vr);}
  
  self& operator++() 
  {
    /* в 10 раз медленнее
      this+=1;
      return *this;
    */
    
    if ( !_vr )
      throw std::out_of_range("vector_list_iterator");
      
    ++_vr;
    if ( !_vr )
    {
      ++_ir;
      if ( !_ir )
      {
        ++_ilr;
        if (!_ilr)
          return *this;
        _ir = index_range( (*_ilr)->begin() , (*_ilr)->end()  );
      }
      _index_ptr = *_ir;
      _vr = value_range( _index_ptr->begin() , _index_ptr->end() );
    }
    return *this; 
  }

  self& operator--()
  {
    *this-=1;
    return *this;
  }

  self& operator -= (difference_type n )
  {
    *this+=-n;
    return *this;
  }

  self& operator += (difference_type n )
  {
    _index_ptr = *_ir;
    size_t current_index = _index_ptr->last_index - fas::distance(_vr);
    size_t new_index = current_index + n;
   
    typename index_list_range::iterator index_list_itr = std::lower_bound(
      fas::begin(_ilr),
      fas::end(_ilr),
      new_index,
      [] (const index_list_value& ilv, const size_t& index) -> bool
      {
        return ilv->last_index  < index + 1;
      }
    );

    _ilr = index_list_range( index_list_itr, fas::end(_ilr) );
    if (!_ilr)
      return *this;
    _ir = index_range( (*_ilr)->begin() , (*_ilr)->end()  );
    typename index_range::iterator index_itr = std::lower_bound(
      fas::begin(_ir),
      fas::end(_ir),
      new_index,
      [this] (const index_value& iv, const size_t& index) -> bool
      {
        _index_ptr = iv;
        return _index_ptr->last_index < index + 1;
      }
    );

    _ir = index_range( index_itr, fas::end(_ir) );
    if (!_ir)
      throw std::logic_error("vector_list_iterator::operator +=");

    _index_ptr = *_ir;
    _vr = value_range( _index_ptr->begin() , _index_ptr->end() );
    if (!_vr)
      throw std::logic_error("vector_list_iterator::operator +=");

    size_t offset = _index_ptr->size() - ( _index_ptr->last_index - new_index );
    _vr += offset;
    return *this;
  }

  self operator++(int)
  {
    self ans = *this;
    ++(*this);
    return ans;
  }

  self operator--(int)
  {
    self ans = *this;
    --(*this);
    return ans;
  }

private:
  index_list_range _ilr;
  index_range _ir;
  value_range _vr;
  index_pointer _index_ptr;
};

#endif

