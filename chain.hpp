#ifndef CHAIN_HPP
#define CHAIN_HPP
#include <iostream>
template<typename T, template<typename> class Chunk >
struct chain
{
  typedef Chunk<T> chunk_type;

  size_t size;
  size_t first_free;

  chain()
    : size(0)
    , first_free(0)
  {}

  void acquire(size_t count)
  {
    size += count;
  }

  chunk_type* first_chunk()
  {
    return reinterpret_cast<chunk_type*>(this+1);
  }

  size_t chunk_size() const
  {
    return sizeof(chunk_type);
  }

  T* first_value()
  {
    if ( chunk_type* beg = first_occuped() )
      return beg->first_value();
    return 0;
  }

  T* next_value(T* value)
  {
    size_t offset = (char*)value - (char*)this->first_chunk()/*->begin()*/;
    chunk_type* chk = first_chunk() + offset/sizeof(chunk_type);
    T* result =  chk->next_value(value);
    if ( result == 0)
    {
      ++chk;
      if ( static_cast<size_t>(chk - this->first_chunk()) == size )
        return 0;
      result = chk->first_value();
    }
    return result;
  }

  chunk_type* first_occuped()
  {
    chunk_type* beg = first_chunk();
    chunk_type* end = beg + size;

    for ( ;beg!=end; ++beg)
    {
      if ( !beg->empty() )
        return beg;
    }
    return 0;
  }

  chunk_type* find_free()
  {
    chunk_type* beg = first_chunk();
    chunk_type* end = beg + size;

    if ( beg!=end && !( beg + first_free)->filled() )
    {
      return beg + first_free;
    }


    for ( ;beg!=end; ++beg)
    {
      if ( !beg->filled() )
      {
        first_free = beg - first_chunk();
        return beg;
      }
    }

    return 0;
  }

  T* mark()
  {
    if ( chunk_type* chk = find_free() )
      return chk->mark();
    return 0;
  }


  void free(T* value)
  {

    size_t offset = (char*)value - (char*)this->first_chunk();
    //offset -= offset%sizeof(chunk_type);
    chunk_type* chk = first_chunk() + offset/sizeof(chunk_type) /*+ (offset%sizeof(chunk_type)!=0)*/;

    chk->free(value);
    if ( offset < first_free )
      first_free = offset;
  }
};

#endif
