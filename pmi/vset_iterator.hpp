#ifndef MAP_ITERATOR_HPP
#define MAP_ITERATOR_HPP

template<typename TreeIterator>
class vset_iterator
{
public:
  typedef vset_iterator<TreeIterator> self;
  typedef TreeIterator tree_iteartor;
  typedef typename tree_iteartor::value_type tree_node;
  typedef typename tree_node::first_type key_type;
  typedef typename tree_node::second_type mapped_type;
  typedef typename mapped_type::value_type array_type;
  typedef typename array_type::value_type array_value_type;

  typedef typename std::iterator_traits<array_value_type*>::iterator_category iterator_category;
  typedef typename std::iterator_traits<array_value_type*>::value_type value_type;
  typedef typename std::iterator_traits<array_value_type*>::difference_type difference_type;
  typedef typename std::iterator_traits<array_value_type*>::pointer pointer;
  typedef typename std::iterator_traits<array_value_type*>::reference reference;


  vset_iterator(tree_iteartor itr, difference_type pos)
    : _itr(itr)
    , _pos(pos)
  {

  }

  vset_iterator(const self& slf)
    : _itr(slf._itr)
    , _pos(slf._pos)
  {}

  reference operator*()  { return _itr->second->at(_pos); }
  const reference operator*() const { return _itr->second->at(_pos); }

  pointer operator->()  { return &( _itr->second->at(_pos) ); }
  const pointer operator->() const { return &( _itr->second->at(_pos) ); }

  self& operator++()
  {
    /*
    std::cout << "++ size " <<  _itr->second->size() << " _pos=" << _pos << std::endl;
    if ( _itr->second->size() == static_cast<size_t>(_pos))
    {
      std::cout << "--" << std::endl;
      ++_itr;
      _pos = 0;
    }
    else
      ++_pos;
    */
    ++_pos;
    if ( _itr->second->size() == static_cast<size_t>(_pos))
    {
      ++_itr;
      _pos = 0;
    }
    return *this;
  }

  self operator++(int)
  {
    self ans = *this;
    ++*this;
    /*if ( _itr->second->size() == _pos)
    {
      ++_itr;
      _pos = 0;
    }
    else
      ++_pos;
    */
    return ans;
  }

  self& operator--()
  {
    if ( _pos == 0 )
    {
      --_itr;
      _pos = _itr->second->size() - 1;
    }
    else
      --_pos;
    return *this;
  }

  self operator--(int)
  {
    self ans = *this;
    --*this;
    return ans;
  }

  self& operator += (difference_type n )
  {
    if ( n >= static_cast<difference_type>( _itr->second->size() - _pos ) )
    {
      n -= _itr->second->size() - _pos;
      ++_itr;

      while ( n >= static_cast<difference_type>( _itr->second->size() ) )
      {
        n -= _itr->second->size();
        ++_itr;
      }
    }

    _pos = n;

    return *this;
  }

  self& operator -= (difference_type n )
  {
    if ( n > _pos )
    {
      n -= _pos;
      --_itr;

      while ( n >= static_cast<difference_type>( _itr->second->size() ) )
      {
        n -= _itr->second->size();
        --_itr;
      }
    }

    _pos = _itr->second->size() - n;
    return *this;
  }

  bool operator == (const self& r ) const
  {
    /*std::cout << "equal " << (_pos == r._pos && _itr == r._itr)
              << ": _pos " << _pos << "==" << r._pos
              << " && " << (_itr == r._itr)
              << std::endl;*/
    return _pos == r._pos && _itr == r._itr;
  }

  bool operator != (const self& r ) const
  {
    return !this->operator == (r);
  }

  bool operator < (const self& r ) const
  {
    if ( _itr < r._itr ) return true;
    return _pos < r._pos;
  }

  bool operator > (const self& r ) const
  {
    if ( _itr > r._itr ) return true;
    return _pos > r._pos;
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
    self ans = *this;
    ans += n;
    return *ans;
    //return _container->at( _offset + n );
  }


  template<typename TI>
  friend typename vset_iterator<TI>::difference_type operator - ( vset_iterator<TI> r1, vset_iterator<TI> r2 );


  tree_iteartor     get_tree_iteartor() const { return _itr;}
  difference_type   get_position() const { return _pos; }
private:
  tree_iteartor _itr;
  difference_type _pos;
};

template<typename TI, typename Dist>
inline vset_iterator<TI> operator +
  (
    vset_iterator<TI> r,
    Dist n
  )
{
  return r+=n;
}

template<typename TI, typename Dist>
inline vset_iterator<TI> operator +
  (
    Dist n,
    vset_iterator<TI> r
  )
{
  return r+= n;
}

template<typename TI, typename Dist>
inline vset_iterator<TI> operator -
  (
    vset_iterator<TI> r,
    Dist n
  )
{
  return r-= n;
}

template<typename TI, typename Dist>
inline vset_iterator<TI> operator -
  (
    Dist n,
    vset_iterator<TI> r
  )
{
  return r -= n;
}

template<typename TI>
inline typename vset_iterator<TI>::difference_type operator -
  (
    vset_iterator<TI> r1,
    vset_iterator<TI> r2
  )
{

  if ( r1._itr == r2._itr )
    return  r1._pos - r2._pos;

  typename vset_iterator<TI>::difference_type result = r1._itr->second->size() - r1._pos;
  for ( ++r1; r1!=r2; ++r1 )
    result += r1._itr->second->size();
  result += r1._pos;

  return result;

  // return r1._offset - r2._offset;
}

#endif

