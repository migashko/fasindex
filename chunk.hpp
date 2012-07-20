#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <cstddef>
#include <stdexcept>

template<typename T>
struct chunk
{
  size_t bits;
  T data[sizeof(size_t)*8];

  chunk(): bits(0) {}

  bool filled() const
  {
    return bits == static_cast<size_t>(-1);
  }

  bool empty() const
  {
    return bits == 0;
  }

  size_t max_count() const
  {
    return sizeof(size_t)*8;
  }

  size_t size() const
  {
    return sizeof(this);
  }

  void clear()
  {
    bits = 0;
  }

  /*
  size_t first()
  {
    for ( size_t i = 0; i < sizeof(size_t)*8; ++i )
      if ( bits & ( static_cast<size_t>(1) << i) )
        return i;
    return static_cast<size_t>(-1);
  }

  size_t next(size_t pos)
  {

  }*/

  T* first_value()
  {
    size_t index = next_occuped(0);
    if ( index == static_cast<size_t>(-1) )
      return 0;
    return data + index;
  }

  T* next_value(T* current)
  {
    size_t index = next_occuped(current - data + 1);
    if ( index == static_cast<size_t>(-1) )
      return 0;
    return data + index;
  }


  size_t next_occuped(size_t pos = 0)
  {
    for ( size_t i = pos; i < sizeof(size_t)*8; ++i )
      if ( bits & ( static_cast<size_t>(1) << i) )
        return i;
    return static_cast<size_t>(-1);
  }


  size_t first_free()
  {
    /// биты c права на лево
    /// 000000000 000000000 000000000 000000000 000000000 000000000 000000000 000000001
    /// Первый элемент свободен
    for ( size_t i = 0; i < sizeof(size_t)*8; ++i )
      if ( ! ( bits & ( static_cast<size_t>(1) << i) ) )
        return i;
    return static_cast<size_t>(-1);
    /*
      var
        x     : integer;
        index : word;
      begin
        x := 44; //101100b
        asm
          bsf   ax, x
          mov   index, ax  //index = 2 (биты нумеруются с 0)
        end;
      end;
    */
  }


  T* mark()
  {
    size_t index = first_free();
    if ( index == static_cast<size_t>(-1) )
      return 0;
    return mark(index);
  }

  T* mark(size_t index)
  {
    bits |= ( static_cast<size_t>(1) <<  index );
    return data + index;
  }

  void free(T* addr)
  {
    size_t index = addr - data;
    if ( index < 64 )
      bits &= ~( static_cast<size_t>(1)<<index);
    else
      throw std::invalid_argument("addr");
  }
};

#endif
