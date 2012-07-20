#ifndef OFFSET_ITERATOR_HPP
#define OFFSET_ITERATOR_HPP

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

  reference operator*()  { return _container->at(_offset); }
  const reference operator*() const { return _container->at(_offset); }

  pointer operator->()  { return &(_container->at(_offset)); }
  const pointer operator->() const { return &(_container->at(_offset)); }

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

#endif
