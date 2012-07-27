#ifndef MMAP_BUFFER_HPP
#define MMAP_BUFFER_HPP

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "buffer_base.hpp"
#include <iostream>


class mmap_buffer
  : public buffer_base
{
public:
  ~mmap_buffer()
  {
    this->close();
  }
  
  mmap_buffer()
    : buffer_base()
    , _fd(-1)
  {
  }
  
  mmap_buffer(const char* path, size_t size = 0)
  {
    this->open( path, size );
  }
  
  void close()
  {
    if ( _addr!=0 )
    {
      if ( -1 == ::msync( _addr, _size, MS_SYNC ) )
        std::cout << "mmap_buffer::msync fail" << std::endl;
      ::munmap( _addr, _size );
    }
    
    if (_fd!=-1)
      ::close( _fd );
    std::cout << "mmap_buffer::close" << std::endl;
  }

  
  bool open(const char* path, size_t size = 0)
  {
    _fd = ::open(path, O_RDWR | O_CREAT, (mode_t)0600);

    if ( _fd == -1)
      return false;

    struct stat sb = { 0 };
    if ( -1 == stat(path, &sb) )
    {
      this->close();
      return false;
    }

    if ( size < static_cast<size_t>(sb.st_size))
      size = sb.st_size;
    
    if ( size > static_cast<size_t>(sb.st_size) )
    {
      if ( -1 == lseek(_fd, size-1, SEEK_SET) )
      {
        this->close();
        return false;
      }
      ::write(_fd, "", 1);
    }
    
    _addr = (char*)::mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
    
    if (_addr == MAP_FAILED)
    {
      this->close();
      return false;
    }
    
    _size = size;
    if ( -1 == ::msync(_addr, _size, MS_SYNC) )
    {
      this->close();
      return false;
    }
    return true;
  }

  void clear() 
  {
    _size = 0;
    resize(0);
    ftruncate(_fd, 0);
  }

  
  bool resize(size_t size)
  {
    if (size > _size)
    {
      if ( -1 == ::lseek(_fd, size-1, SEEK_SET) )
        return false;
      write(_fd, "", 1); 
    }
    else if (size < _size)
    {
      // TODO: урезать файл
    }

    char* addr = (char*) ::mremap( _addr, _size, size, MREMAP_MAYMOVE);
    if ( addr == (void*)(-1) )
      return false;
    _addr = addr;
    _size = size;
    return true;
  }

  operator bool () const 
  {
    return _fd != -1;
  }

  void sync( bool async = false )
  {
    ::msync(_addr, _size, async ? MS_ASYNC : MS_SYNC);
  }
  
private:
  int _fd;
};

#endif
