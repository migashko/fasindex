#ifndef FIXED_SIZE_BLOCKS_ALLOCATION_HPP
#define FIXED_SIZE_BLOCKS_ALLOCATION_HPP

#include "offset_pointer.hpp"
#include "chunk.hpp"
#include "chain.hpp"

template<typename T, typename M /* = */ >
class fixed_size_blocks_allocation
{
public:
  typedef fixed_size_blocks_allocation<T, M> self;
  typedef M memory_manager;
  typedef chunk<T> chunk_type;
  typedef chain<T, chunk > chain_type;
  typedef offset_pointer<T, self> pointer;
  typedef offset_pointer<const T, self> const_pointer;

  fixed_size_blocks_allocation(memory_manager* mm)
    : _memory_manager(mm)
  {}

  bool acquire()
  {
    size_t offset = _memory_manager->size();
    _memory_manager->resize( offset + sizeof(chain_type) + sizeof(chunk_type) );
    char *addr = _memory_manager->addr();
    chain_type* ch = 0;
    if ( offset == 0 )
      ch = new (addr) chain_type;
    else
      ch =reinterpret_cast<chain_type*>(addr);
    ch->acquire(1);
    new (addr+offset)chunk_type;
    return true;
  }

  pointer allocate()
  {
    if ( _memory_manager->size() == 0 )
      this->acquire();

    chain_type* chn = (chain_type*)_memory_manager->addr();
    pointer p( this );
    p = chn->mark();
    if (!p)
    {
      acquire();
      chn = (chain_type*)_memory_manager->addr();
      p = chn->mark();
      if (!p)
        std::cout << "realloc fail!!!" << std::endl;
    }
    return p;
  }

  void deallocate(pointer ptr)
  {
    chain_type* chn = (chain_type*)_memory_manager->addr();
    chn->free(ptr);
  }

/// offset pointer

  T* get(size_t offset) const
  {
    return reinterpret_cast<T*>(_memory_manager->addr() + offset);
  }

  T* get(size_t offset1, size_t offset2) const
  {
    /// TODO: работает не правлильно, просто заглушка
    return reinterpret_cast<T*>(_memory_manager->addr() + offset1);
  }

  size_t offset(T* p)
  {
    if ( p==0 )
      return static_cast<size_t>(-1);
    return reinterpret_cast<char*>(p) - _memory_manager->addr();
  }

  size_t offset(T* p, size_t offset)
  {
    /// TODO: работает не правлильно, просто заглушка
    if ( p==0 )
      return static_cast<size_t>(-1);
    return reinterpret_cast<char*>(p) - _memory_manager->addr();
  }
  
  size_t next(size_t offset1, size_t offset2 = 1)
  {
    return offset1 + offset2;
  }
private:
  memory_manager* _memory_manager;
};

#endif
