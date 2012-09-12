#ifndef MANAGED_ALLOCATOR_HPP
#define MANAGED_ALLOCATOR_HPP

#include <limits>
#include <pmi/buffer/mmap_buffer.hpp>
#include <pmi/memory/fixed_size_blocks_allocation.hpp>

template<typename T, typename AllocateManager = chain_memory<T, mmap_buffer> >
struct managed_allocator
{
  typedef managed_allocator<T, AllocateManager> self;
  typedef AllocateManager allocation_manager;
  typedef T value_type;
  typedef typename allocation_manager::pointer pointer;
  typedef typename allocation_manager::const_pointer const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef std::ptrdiff_t difference_type;
  
  managed_allocator(allocation_manager& mmm): _mmm(&mmm) { }
  /*managed_allocator(const self& s): _mmm(s._mmm) { }
  void operator = (const self& s){ _mmm = s._mmm; }*/
  
  //rebind
  template <typename U>
  struct rebind {
    typedef managed_allocator<U> other;
  };

  pointer address (reference value ) const { return static_cast<char*>(&value) - _mmm.addr() ; }

  const_pointer address (const_reference value) const { return static_cast<char*>(&value) - _mmm.addr() ; }
  
  size_type max_size () const throw() { return ::std::numeric_limits <size_type>::max() / sizeof(T); }

  pointer allocate (size_type num, void *  hint = 0)
  {
    //if (num!=1) throw;
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
    //if (num!=1) throw;
    _mmm->deallocate(p, num);
  }
    
  
private:
  allocation_manager* _mmm;
};

#endif
