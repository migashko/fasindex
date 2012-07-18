#ifndef MANAGED_ALLOCATOR_HPP
#define MANAGED_ALLOCATOR_HPP

#include "mmap_buffer.hpp"
#include "fixed_size_blocks_allocation.hpp"

template<typename T, typename AllocateManager = fixed_size_blocks_allocation<T, mmap_buffer> >
struct managed_allocator
{
  typedef AllocateManager allocation_manager;
  typedef T value_type;
  typedef typename allocation_manager::pointer pointer;
  typedef typename allocation_manager::const_pointer const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef std::ptrdiff_t difference_type;

  /*
  enum { data_block_size = sizeof(T)*sizeof(size_t) };
  enum { block_size = sizeof(size_t) + data_block_size };
  */
  
  managed_allocator(allocation_manager& mmm): _mmm(mmm) { }
  
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
    return _mmm.allocate();
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
    _mmm->deallocate(p);
  }
    
  
private:
  allocation_manager& _mmm;
};

#endif
