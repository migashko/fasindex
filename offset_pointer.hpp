#ifndef OFFSET_POINTER_HPP
#define OFFSET_POINTER_HPP

template<typename T, typename M>
struct offset_pointer
{
  typedef offset_pointer<T, M> self;
  M *mmm;
  size_t offset;

  typedef typename std::iterator_traits<T*>::iterator_category iterator_category;
  typedef typename std::iterator_traits<T*>::value_type        value_type;
  typedef typename std::iterator_traits<T*>::difference_type   difference_type;
  typedef typename std::iterator_traits<T*>::pointer           pointer;
  typedef typename std::iterator_traits<T*>::reference         reference;



  offset_pointer()
    : mmm(0)
    , offset(static_cast<size_t>(-1))
  { }

  offset_pointer(M* mmm, size_t offset = static_cast<size_t>(-1) )
    : mmm(mmm)
    , offset(offset)
  {}

  offset_pointer(M* mmm, T* ptr )
    : mmm(mmm)
    , offset(mmm->offset(ptr))
  {
  }

  T& operator*() { return *(mmm->get(offset));}
  const T& operator*() const { return *(mmm->get(offset));}

  T* operator->() { return mmm->get(offset);}
  const T* operator->() const { return mmm->get(offset);}

  void operator = ( size_t offset ) { this->offset = offset; }
  void operator = ( T* t ) { this->offset = mmm->offset(t); }
  operator size_t () const { return offset;}

  operator bool () const { return offset != static_cast<size_t>(-1);}

  self& operator++()
  {
    // offset+=sizeof(T);
    offset = mmm->next(offset);
    return *this;
  }

  self operator++(int)
  {
    self ans = *this;
    offset = mmm->next(offset);
    return ans;
  }

  self& operator--()
  {
    /// TODO:!!!!!!!!
    /*
    offset-=sizeof(T);
    return *this;
    */
  }

  self operator--(int)
  {
    /// TODO:!!!!!!!!
    /*
    self ans = *this;
    offset-=sizeof(T);
    return ans;
    */
  }

  bool operator == (const self& r ) const
  {
    return offset == r.offset && mmm==r.mmm ;
  }

  bool operator != (const self& r ) const
  {
    return !this->operator == (r);
  }

  bool operator < (const self& r ) const
  {
    return offset < r.offset;
  }

  bool operator > (const self& r ) const
  {
    return offset > r.offset;
  }

  bool operator <= (const self& r ) const
  {
    return !this->operator >( r );
  }

  bool operator >= (const self& r ) const
  {
    return !this->operator < ( r );
  }

  self& operator += (difference_type n )
  {
    //offset += n * sizeof(T);
    offset = mmm->next(offset, n);
    return *this;
  }

  self& operator -= (difference_type n )
  {
    /// TODO:!!!!!!!!
    /*
    offset -= n  * sizeof(T);
    return *this;
    */
  }

  reference operator[] ( difference_type n ) const
  {
    return mmm->template get<T>( offset + sizeof(T)*n );
  }

};


template<typename T, typename M, typename Dist>
inline offset_pointer<T, M> operator +
  (
    offset_pointer<T, M> r,
    Dist n
  )
{
  return r+=n;
}

template<typename T, typename M, typename Dist>
inline offset_pointer<T, M> operator +
  (
    Dist n,
    offset_pointer<T, M> r
  )
{
  return r+= n;
}

template<typename T, typename M, typename Dist>
inline offset_pointer<T, M> operator -
  (
    offset_pointer<T, M> r,
    Dist n
  )
{
  return r-= n;
}

template<typename T, typename M, typename Dist>
inline offset_pointer<T, M> operator -
  (
    Dist n,
    offset_pointer<T, M> r
  )
{
  return r -= n;
}

template<typename T, typename M>
inline typename offset_pointer<T, M>::difference_type operator -
  (
    offset_pointer<T, M> r1,
    offset_pointer<T, M> r2
  )
{
  typename offset_pointer<T, M>::difference_type dist = 0;
  for ( ;r2!=r1; ++dist, ++r2);
  return dist;
  //return r1.offset - r2.offset;
}

#endif
