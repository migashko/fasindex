#ifndef VECTOR_LIST_ITERATOR_HPP
#define VECTOR_LIST_ITERATOR_HPP

template<
  typename IndexListRange,
  typename IndexRange,
  typename ValueRange
>
class vector_list_iterator
{
public:
  typedef vector_list_iterator<IndexListRange, IndexRange, ValueRange> self;
  typedef IndexListRange index_list_range;
  typedef IndexRange index_range;
  typedef ValueRange value_range;
  
  typedef typename value_range::value_type value_type;
  typedef typename value_range::pointer pointer;
  typedef typename value_range::reference reference;

  vector_list_iterator(index_list_range ilr, index_range ir, value_range vr)
    : _ilr(ilr), _ir(ir), _vr(vr)
  {}
   
  value_type* operator->() { return &(*_vr);}
  
  self& operator++() 
  {
    // TODO: недоделан
    ++_vr;
    return *this; 
  }
private:
  index_list_range _ilr;
  index_range _ir;
  value_range _vr;
};

#endif

