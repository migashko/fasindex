#ifndef NEXT_INDEX_HOLDER_HPP
#define NEXT_INDEX_HOLDER_HPP

#include <cstddef>

struct next_index_holder
{
  std::size_t next_index;
};


struct next_index_super_holder
  : next_index_holder
{
  // Тебуется пересчитать next_index
  std::size_t flag:1;
  std::size_t common_size:63;
};


#endif
