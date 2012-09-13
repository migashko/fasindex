#ifndef SET_ITEM_HPP
#define SET_ITEM_HPP

#include <set>
#include <pmi/vector/sorted_array.hpp>
#include <pmi/allocator.hpp>

/*
struct set_item_head
{
  std::size_t empty:1;
  std::size_t padding:63;

  set_item_head(): empty(0), padding(0) {}
};


template<typename T, size_t N, typename Compare = std::less<T> >
class set_item
  : set_item_head
  , public sorted_array<T, N, Compare>
{
  sorted_array( comparator cmp = comparator() )
    : set_item_head()
    , sorted_array<T, N, Compare>(cmp)
  {};
};
*/

// Не нужно - мы по size определяем что не используется блок

template<typename T, typename Compare>
struct array_compare
{
  array_compare( const Compare& cmp = Compare() )
    : _compare(cmp)
  {
    
  }
  
  // Сделать < > и упростить
  bool operator()(const T* first, const T* second) const
  {
    // Пустые массивы не допустимы
    return _compare( (*first)[0], (*second)[0] );
  }
  
private:
  
  Compare _compare;
};

template< typename SortedArray> 
struct array_tree: std::set< SortedArray*,  array_compare< SortedArray*, typename SortedArray::value_compare> > {};


#endif