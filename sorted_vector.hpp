#ifndef SORTED_VECTOR_HPP
#define SORTED_VECTOR_HPP

template<
  typename Container,
  typename Comparator
>
class sorted_vector
{
  typedef sorted_vector<Container, Comparator> self;
public:
  typedef Container container;
  typedef Comparator comparator;

  typedef T value_type;
  typedef T data_type[N];
  typedef size_t size_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef pointer iterator;
  typedef const_pointer const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef const std::reverse_iterator<iterator> const_reverse_iterator;
  typedef std::ptrdiff_t difference_type;

  sorted_vector(): _comparator(), _container() {};

  reference operator[](size_type n)
  {
    return this->at(n);
  }

  const_reference operator[](size_type n) const
  {
    return this->at(n);
  }

  const_reference at ( size_type n ) const
  {
    if ( n < _size)
      return _data[n];
    throw std::out_of_range("array::at");
  }

  reference at ( size_type n )
  {
    if ( n < _size )
      return _data[n];
    throw std::out_of_range("array::at");
  }
  reference front ( ){ return _data[0]; }
  const_reference front ( ) const{ return _data[0]; }
  reference back ( ){ return _data[_size-1]; }
  const_reference back ( ) const{ return _data[_size-1]; }

  size_type size() const { return _size;}
  size_type max_size() const { return N;}
  size_type capacity() const { return N;}
  bool empty () const {return _size==0;}
  bool filled () const { return _size == N;}
  void resize ( size_type sz, T value = value_type() )
  {
    if (sz > _size)
      std::fill_n( end(), sz - _size, value );
    _size = sz;
  }
  void reserve ( size_type n ) {}

  reverse_iterator rbegin() { return reverse_iterator(end()/*+_size-1*/); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(/*begin()+_size-1*/end()); }

  reverse_iterator rend() { return reverse_iterator(/*begin()-1*/begin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(/*begin()-1*/begin()); }

  iterator begin() { return _data;}
  const_iterator begin() const { return _data;}
  iterator end() { return _data + _size;}
  const_iterator end() const { return _data + _size;}
  iterator last() { return _data + _size - 1;}
  const_iterator last() const { return _data  + _size - 1;}

  void clear()
  {
    std::fill_n( begin(), N, T() );
    _size = 0;
  }


  template <class InputIterator>
  void assign ( InputIterator first, InputIterator last )
  {
    std::copy( first, last, _data );
    _size = std::distance(first, last);
  }

  void assign ( size_type n, const T& u )
  {
    std::fill_n( begin(), n, u );
    _size = n;
  }

  void push_back ( const T& x )
  {
    _data[_size++] = x;
  }

  void pop_back ( )
  {
    --_size;
  }

  iterator insert ( iterator position, const T& x )
  {
    if ( this->size() + 1 > this->capacity() )
      throw std::out_of_range("array::insert");

    std::copy_backward(position, end(), end()+1);
    *position = x;
    ++_size;
    return position;
  }

  void insert ( iterator position, size_type n, const T& x )
  {
    if ( this->size() + n > this->capacity() )
      throw std::out_of_range("array::insert");
    std::copy_backward(position, end(), end()+n);
    std::fill_n(position, n, x);
    _size+=n;
  }

  template <class InputIterator>
  void insert ( iterator position, InputIterator first, InputIterator last )
  {
    typename InputIterator::difference_type dist = std::distance(first,last);
    if ( this->size() + dist > this->capacity() )
      throw std::out_of_range("array::insert");

    std::copy_backward(position, end(), end()+dist );
    std::copy(first, last, position);
    _size+=std::distance(first,last);
  }

  iterator erase ( iterator position )
  {
    std::copy( position + 1, this->end(), position);
    this->resize( _size - 1 );
    return position;
  }

  iterator erase ( iterator first, iterator last )
  {
    difference_type dist = last - first;
    std::copy( last, this->end(), first);
    this->resize( _size - dist );
    return first;
  }

  
  

private:
  comparator _comparator;
  container  _container;
};

#endif
