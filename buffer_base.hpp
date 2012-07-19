#ifndef BUFFER_BASE_HPP
#define BUFFER_BASE_HPP

class buffer_base
{
public:
  buffer_base()
    : _addr(0)
    , _size(0)
    , _capacity(0)
  {
  }

  buffer_base(char *addr, size_t size)
    : _addr(addr)
    , _size(size)
    , _capacity(size)
  {
  }

  buffer_base(char *addr, size_t size, size_t capacity)
    : _addr(addr)
    , _size(size)
    ,_capacity(capacity)
  {
  }


  size_t size() const 
  { 
    return _size;
  }
  
  size_t capacity() const 
  {
    return _capacity; 
  }
  
  char *addr() const 
  {
    return _addr;
  }
  
protected:
  char *_addr;
  size_t _size;
  size_t _capacity;
};

#endif // BUFFER_BASE_HPP
