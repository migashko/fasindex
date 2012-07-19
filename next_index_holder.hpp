#ifndef NEXT_INDEX_HOLDER_HPP
#define NEXT_INDEX_HOLDER_HPP

#include <cstddef>

struct next_index_holder
{
  // на самом деле last_index + 1
  std::size_t next_index;
};

#endif
