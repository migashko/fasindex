#ifndef CHAIN_HPP
#define CHAIN_HPP

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
  
  chunk_type* begin()
  {
    return reinterpret_cast<chunk_type*>(this+1);
  }

  size_t chunk_size() const
  {
    return sizeof(chunk_type); 
  }

  chunk_type* find_free()
  {
    chunk_type* beg = begin();
    chunk_type* end = beg + size;

    if ( beg!=end && !( beg + first_free)->filled() )
      return beg + first_free;

    for ( ;beg!=end; ++beg)
    {
      if ( !beg->filled() )
      {
        first_free = beg - begin();
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
    size_t offset = value - this->begin()->begin();
    offset -= offset%sizeof(chunk_type);
    chunk_type* chk = begin() + offset;
    chk->free(value);
    if ( offset < first_free )
      first_free = offset;
  }
};

#endif
