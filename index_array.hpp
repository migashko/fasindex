#ifndef INDEX_ARRAY_HPP
#define INDEX_ARRAY_HPP

#include "next_index_holder.hpp"
#include "array.hpp"

template<typename T, int N>
struct index_array
  : next_index_holder
  , array<T, N>
{
};

#endif
