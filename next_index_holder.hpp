#ifndef NEXT_INDEX_HOLDER_HPP
#define NEXT_INDEX_HOLDER_HPP

#include <cstddef>

struct next_index_holder
{
  void next_index(size_t n)
  {
    _next_index = n;
  }

  void inc_next_index(size_t n = 1)
  {
    _next_index += n;
  }

  void dec_next_index(size_t n = 1)
  {
    _next_index -= n;
  }

  size_t next_index() const
  {
    return _next_index;
  }

private:
  // на самом деле last_index + 1
  std::size_t _next_index;
};

#endif
