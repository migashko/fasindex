#ifndef SET_ITEM_HPP
#define SET_ITEM_HPP

#include <pmi/array/sorted_array.hpp>

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

template<typename Arr>
struct array_compare
{

};

template<typename ArrayType, typename Compare = std::less<T> >
class set_items
{
public:
  typedef std::set<ArrayType*, Compare >
private:
}


#endif