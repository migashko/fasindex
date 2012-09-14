#ifndef FIXED_SIZE_BLOCKS_ALLOCATION_HPP
#define FIXED_SIZE_BLOCKS_ALLOCATION_HPP

#include <pmi/memory/offset_pointer.hpp>
#include <pmi/memory/chunk.hpp>
#include <pmi/memory/chain.hpp>
#include <iostream>

// chain_memory_mananger

template<typename T, typename B /* = */ >
class chain_memory
{
public:
  typedef chain_memory<T, B> self;
  typedef B buffer;
  typedef chunk<T> chunk_type;
  typedef chain<T, chunk > chain_type;
  typedef offset_pointer<T, self> pointer;
  typedef offset_pointer<const T, self> const_pointer;
  typedef size_t size_type;


  chain_memory(buffer& mm)
    : _buffer(&mm)
  {}

  pointer begin()
  {
    if ( _buffer->size() == 0 )
      return pointer( this );

    chain_type* chn = (chain_type*)_buffer->addr();
    if ( T* value = chn->first_value() )
      return pointer( this, reinterpret_cast<char*>(value) - _buffer->addr() );
    return pointer( this );
  }

  pointer end()
  {
    return pointer( this );
  }

  bool acquire()
  {
    size_t offset = _buffer->size();
    _buffer->resize( offset + sizeof(chain_type) + sizeof(chunk_type) );
    char *addr = _buffer->addr();
    chain_type* ch = 0;
    if ( offset == 0 )
      ch = new (addr) chain_type;
    else
      ch =reinterpret_cast<chain_type*>(addr);
    ch->acquire(1);
    new (addr+offset)chunk_type;

    /*
    std::cout << "acquire offset " << offset << std::endl;
    std::cout << "acquire sizeof(chain_type) " << sizeof(chain_type) << std::endl;
    std::cout << "acquire sizeof(chunk_type) " << sizeof(chunk_type) << std::endl;
    std::cout << "acquire sizeof(chunk_type::value_type) " << sizeof(typename chunk_type::value_type) << std::endl;
    std::cout << "acquire sizeof(chunk_type::value_type::dimension) " << chunk_type::value_type::dimension << std::endl;
  */

    return true;
  }

  pointer allocate(size_type /*num*/, void *  /*hint*/ = 0)
  {
    if ( _buffer->size() == 0 )
      this->acquire();

    chain_type* chn = (chain_type*)_buffer->addr();
    pointer p( this );
    p = chn->mark();
    if (!p)
    {
      acquire();
      chn = (chain_type*)_buffer->addr();
      p = chn->mark();
      if (!p)
        std::cout << "realloc fail!!!" << std::endl;
    }
    return p;
  }

  void deallocate(pointer ptr, size_type /* num */)
  {
    chain_type* chn = (chain_type*)_buffer->addr();
    chn->free( reinterpret_cast<T*>(_buffer->addr() + ptr.offset) );
  }

/// offset pointer

  T* get(size_t offset) const
  {
    return reinterpret_cast<T*>(_buffer->addr() + offset);
  }

  T* get(size_t offset1, size_t offset2) const
  {
    /// TODO: работает не правлильно, просто заглушка
    return reinterpret_cast<T*>(_buffer->addr() + offset1);
  }

  size_t offset(T* p)
  {
    if ( p==0 )
      return static_cast<size_t>(-1);
    return reinterpret_cast<char*>(p) - _buffer->addr();
  }

  size_t offset(T* p, size_t offset)
  {
    /// TODO: работает не правлильно, просто заглушка
    if ( p==0 )
      return static_cast<size_t>(-1);
    return reinterpret_cast<char*>(p) - _buffer->addr();
  }

  size_t next(size_t offset, size_t count = 1)
  {
    chain_type* chn = (chain_type*)_buffer->addr();

    for (size_t i = 0 ; i < count ; ++i)
    {
      if ( T* current = chn->next_value( reinterpret_cast<T*>(_buffer->addr() + offset) ) )
        offset = reinterpret_cast<char*>(current) - _buffer->addr();
      else
        return static_cast<size_t>(-1);
    }
    return offset;
  }
private:
  buffer* _buffer;
};

#endif
