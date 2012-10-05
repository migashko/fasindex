#ifndef MANAGED_ALLOCATOR_HPP
#define MANAGED_ALLOCATOR_HPP

#include <pmi/buffer/mmap_buffer.hpp>
#include <pmi/memory/chain_memory.hpp>
#include <limits>
#include <cassert>


template<typename T, typename MemoryManager /* = chain_memory<T, mmap_buffer> */ >
struct allocator
{
  typedef allocator<T, MemoryManager> self;
  typedef MemoryManager allocation_manager;
  typedef T value_type;
  typedef typename allocation_manager::pointer pointer;
  typedef typename allocation_manager::const_pointer const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef std::ptrdiff_t difference_type;

  allocator(): _mmm(0) { }
  
  allocator(allocation_manager& mmm): _mmm(&mmm) { }
  /*managed_allocator(const self& s): _mmm(s._mmm) { }
  void operator = (const self& s){ _mmm = s._mmm; }*/
  
  //rebind
  template <typename U>
  struct rebind {
    typedef allocator<U, MemoryManager> other;
  };

  pointer address(reference value ) const
  {
    assert(_mmm!=0);
    return static_cast<char*>(&value) - _mmm->addr() ;
  }

  const_pointer address (const_reference value) const
  {
    assert(_mmm!=0);
    return static_cast<char*>(&value) - _mmm->addr();
  }
  
  size_type max_size () const throw()
  {
    return ::std::numeric_limits <size_type>::max() / sizeof(T);
  }

  pointer allocate (size_type num, void *  hint = 0)
  {
    assert(_mmm!=0);
    return _mmm->allocate(num, hint);
  }

  void construct (pointer p, const_reference value)
  {
      new((void *)(&(*p))) T(value);  //placement new
  }

  void destroy (pointer p)
  {
    p->~T();
  }

  void deallocate (pointer p, size_type num)
  {
    assert(_mmm!=0);
    _mmm->deallocate(p, num);
  }

/// non-standart, для востановления

  allocation_manager* operator-> ()
  {
    assert(_mmm!=0);
    return _mmm;
  }

  void set_manager( allocation_manager* mm)
  {
    _mmm = mm;
  }
  
private:

  
  allocation_manager* _mmm;
};

#endif
