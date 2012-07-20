#ifndef VALUE_ARRAY_HPP
#define VALUE_ARRAY_HPP

#include "next_index_holder.hpp"
#include "array.hpp"

template<typename T, int N>
struct value_array
  : next_index_holder
  , array<T, N>
{

  // тольк для данных, TODO: перенести в array_of_array
  size_t to_index(size_t n)
  {
    if ( n >= next_index_holder::next_index || array<T, N>::size() < (next_index_holder::next_index - n) )
      throw std::logic_error("value_array::to_index");
    return array<T, N>::size() - (next_index_holder::next_index - n);
  }
};


#endif
