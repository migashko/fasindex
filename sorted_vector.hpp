#ifndef SORTED_VECTOR_HPP
#define SORTED_VECTOR_HPP

#include <algorithm>
#include <functional>


template<
  typename Container,
  typename Comparator = std::less< typename Container::value_type >
>
class sorted_vector_base
{
  typedef sorted_vector_base<Container, Comparator> self;
public:
  typedef Container container;
  typedef Comparator comparator;

  typedef typename container::value_type value_type;
  typedef typename container::size_type size_type;
  typedef typename container::reference reference;
  typedef typename container::const_reference const_reference;
  typedef typename container::pointer pointer;
  typedef typename container::const_pointer const_pointer;
  typedef typename container::iterator iterator;
  typedef typename container::const_iterator const_iterator;
  typedef typename container::reverse_iterator reverse_iterator;
  typedef typename container::const_reverse_iterator const_reverse_iterator;
  typedef typename container::difference_type difference_type;

  sorted_vector_base(container cnt = container(), comparator cmp = comparator() ): _comparator(cmp), _container(cnt) {};

  const_reference operator[](size_type n) const
  {
    return _container[n];
  }

  const_reference at( size_type n ) const
  {
    return _container->at(n);
  }

  const_reference front ( ) const { return _container.front(); }
  const_reference back ( ) const{ return _container.back(); }

  size_type size() const  { return _container.size();}
  size_type max_size() const { return _container.max_size();}
  size_type capacity() const { return _container.capacity();}
  bool empty () const {return _container.empty();}
  
  void resize ( size_type sz, value_type value = value_type() )
  {
    _container.resize( sz, value );
  }
  
  void reserve ( size_type n ) { _container.reserve(n); }
  const_reverse_iterator rbegin() const { return _container.rbegin(); }
  const_reverse_iterator rend() const { return _container.rend(); }
  const_iterator begin() const { return _container.begin();}
  const_iterator end() const { return _container.rend();}
  
  void clear() { _container.clear(); }

  template <class InputIterator>
  void assign ( InputIterator first, InputIterator last )
  {
    _container.assign(first, last);
    std::sort(_container.begin(), _container.end(), _comparator);
  }

  void assign ( size_type n, const value_type& u )
  {
    _container.assign(n, u);
  }

  
  // void push_back ( const T& x ) {  }

  void pop_back ( ) {  _container.pop_back();  }

  iterator erase ( iterator position )
  {
    return _container.erase(position);
  }

  iterator erase ( iterator first, iterator last )
  {
    return _container.erase(first, last);
  }

/// ext. interface

  container& get_container() { return _container; }
  const container& get_container() const { return _container; }

protected:
  comparator _comparator;
  container  _container;
};

template<
  typename Container,
  typename Comparator = std::less< typename Container::value_type >
>
class sorted_vector: public sorted_vector_base<Container, Comparator>
{
  typedef sorted_vector_base<Container, Comparator> super;
public:
  typedef typename super::value_type value_type;
  typedef typename super::iterator iterator;
  typedef typename super::const_iterator const_iterator;
  typedef typename super::size_type size_type;
  typedef typename super::container container;
  typedef typename super::comparator comparator;

  sorted_vector(container cnt = container(), comparator cmp = comparator() ): super(cnt, cmp) {};
  
  std::pair<iterator, bool> insert ( const value_type& x )
  {
    typename container::iterator itr = std::lower_bound( super::_container.begin(), super::_container.end(), x, super::_comparator );
    return std::make_pair( super::_container.insert(itr, x), true );
  }

  void insert( size_type n, const value_type& x )
  {
    typename container::iterator itr = std::lower_bound( super::_container.begin(), super::_container.end(), x, super::_comparator );
    super::_container.insert(itr, n, x);
  }

  template <class InputIterator>
  void insert ( InputIterator first, InputIterator last )
  {
    std::for_each(first, last, [this](const typename InputIterator::value_type& value ) { this->insert(value);} );
  }

  void update(iterator position, const value_type& value )
  {
    if ( !super::_comparator(*position, value) && !super::_comparator(value, *position) )
      *position = value;
    else
    {
      this->erase(position);
      this->insert(value);
    }
  }
};

template<
  typename Container,
  typename Comparator = std::less< typename Container::value_type >
>
class unique_sorted_vector: public sorted_vector_base<Container, Comparator>
{
  typedef unique_sorted_vector<Container, Comparator> super;
public:
  typedef typename super::value_type value_type;
  typedef typename super::iterator iterator;
  typedef typename super::const_iterator const_iterator;
  typedef typename super::size_type size_type;
  typedef typename super::container container;
  typedef typename super::comparator comparator;

  unique_sorted_vector(container cnt = container(), comparator cmp = comparator() ): super(cnt, cmp) {};
  
  std::pair<iterator, bool> insert ( const value_type& x )
  {
    typename container::iterator itr = std::lower_bound( super::_container.begin(), super::_container.end(), x, super::_comparator );
    if (itr!=super::_container.end() && !super::_comparator(*itr, x) && !super::_comparator(x, *itr))
      return std::make_pair(itr, false);
    return std::make_pair( super::_container.insert(itr, x), true );
  }

  void insert( size_type n, const value_type& x )
  {
    this->insert( x );
  }

  template <class InputIterator>
  void insert ( InputIterator first, InputIterator last )
  {
    std::for_each(first, last, [this](const typename InputIterator::value_type& value ) { this->insert(value);} );
  }

  void update(iterator position, const value_type& value )
  {
    if ( !super::_comparator(*position, value) && !super::_comparator(value, *position) )
      *position = value;
    else
    {
      this->erase(position);
      this->insert(value);
    }
  }
};

/*
std::pair<iterator, bool> insert ( const value_type& x )
  {
    typename container::iterator itr = std::lower_bound(_container.begin(), _container.end(), _comparator );
    return std::make_pair( _container.insert(itr, x), true );
  }*/
//typedef typename sorted_vector< std::vector<int>, std::less<int>, true>::size_type tmp_size_type;
/*
template<typename Container,typename Comparator>
std::pair< typename sorted_vector<Container, Comparator, true>::iterator, bool>
sorted_vector<Container, Comparator, true>
::insert( const typename sorted_vector<Container, Comparator, true>::value_type& x )
{
  typename container::iterator itr = std::lower_bound(_container.begin(), _container.end(), _comparator );
  if (itr!=_container.end() && !_comparator(*itr, x) && !_comparator(x, *itr))
    return std::make_pair(itr, false);
  return std::make_pair( _container.insert(itr, x), true );
}*/

/*
template<typename Container, typename Comparator>
void sorted_vector<Container, Comparator, true>::insert( typename Container::size_type n, const typename Container::value_type& x )
{
  this->insert(x);
}*/

/*
template<typename Container, typename Comparator, bool Flag>
void sorted_vector<Container, Comparator, Flag>::insert( typename Container::size_type n, const typename Container::value_type& x )
{
  typename container::iterator itr = std::lower_bound(_container.begin(), _container.end(), _comparator );
  return _container.insert(itr, n, x);
}

template<typename Container, typename Comparator>
typename Comparator::size_type sorted_vector<Container, Comparator, false>::size( ) const
{
  return 0;;
}
*/




#endif
