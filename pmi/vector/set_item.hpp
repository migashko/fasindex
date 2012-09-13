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


  bool operator()(const typename T::value_type& first, const T* second) const
  {
    // Пустые массивы не допустимы
    return _compare( first, (*second)[0] );
  }

    bool operator()(const T* first, const typename T::value_type& second) const
  {
    // Пустые массивы не допустимы
    return _compare( (*first)[0], second );
  }

private:

  Compare _compare;
};

template< typename SortedArray>
class array_tree
  : public std::set< SortedArray*,  array_compare< SortedArray, typename SortedArray::value_compare> >
{
  typedef std::set< SortedArray*,  array_compare< SortedArray, typename SortedArray::value_compare> >  super;
public:
  typedef typename super::iterator iterator;
  typedef typename super::const_iterator const_iterator;
  typedef typename SortedArray::value_type array_key_type;


  iterator lower_bound(const array_key_type& key)
  {
    return std::lower_bound( super::begin(), super::end(), key, typename super::key_compare());
    //return super::lower_bound( &&key );
  }

  const_iterator lower_bound(const array_key_type& key) const
  {
    return std::lower_bound( super::begin(), super::end(), key, typename super::key_compare() );
    // return super::lower_bound( &key );
  }
};


#endif