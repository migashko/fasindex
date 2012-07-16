#ifndef VECTOR_LIST_ITERATOR_HPP
#define VECTOR_LIST_ITERATOR_HPP

template<
  typename IndexListRange,
  typename IndexRange,
  typename ValueRange,
  typename IndexPointerType
>
class vector_list_iterator
{
public:
  typedef vector_list_iterator<IndexListRange, IndexRange, ValueRange, IndexPointerType> self;
  typedef IndexListRange index_list_range;
  typedef IndexRange index_range;
  typedef ValueRange value_range;
  typedef IndexPointerType index_pointer;

  typedef typename index_range::value_type index_value;
  typedef typename value_range::value_type value_type;
  typedef typename value_range::pointer pointer;
  typedef typename value_range::reference reference;
  typedef typename value_range::difference_type difference_type;
  

  vector_list_iterator(index_list_range ilr, index_range ir, value_range vr, index_pointer index_ptr)
    : _ilr(ilr), _ir(ir), _vr(vr), _index_ptr(index_ptr)
  {}
   
  value_type* operator->() { return &(*_vr);}
  
  self& operator++() 
  {
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

  self& operator += (difference_type n )
  {
    std::cout << "fas::distance(_ilr)=" << fas::distance(_ilr) << std::endl;
    int tmp_count = 0;
    bool flag = fas::distance(_vr) > n;
    while ( _ilr && !flag )
    {
      while ( _ir && !flag )
      {
        while ( _vr && !flag )
        {
          ++tmp_count;
          n-= _vr;
          ++_vr;
          if (_vr)
            flag = fas::distance(_vr) > n;
        }
        
        if ( !flag )
        {
          ++_ir;
          if ( _ir )
          {
            _index_ptr = *_ir;
            _vr = value_range( _index_ptr->begin() , _index_ptr->end() );
          }
        }
      }
      
      if ( !flag )
      {
        ++_ilr;
        if ( _ilr )
        {
          _ir = index_range( (*_ilr)->begin() , (*_ilr)->end()  );
          if ( _ir )
          {
            _index_ptr = *_ir;
            _vr = value_range( _index_ptr->begin() , _index_ptr->end() );
          }
        }
      }
    }
    _vr+=n;
    std::cout << "tmp_count=" << tmp_count << std::endl;
    return *this;
  }


  
  

  self operator++(int)
  {
    self ans = *this;
    ++(*this);
    return ans;
  }

private:
  index_list_range _ilr;
  index_range _ir;
  value_range _vr;
  index_pointer _index_ptr;
};

#endif

