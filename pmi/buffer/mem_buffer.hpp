#ifndef MEM_BUFFER_HPP
#define MEM_BUFFER_HPP

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

#include <pmi/buffer/buffer_base.hpp>
#include <stdexcept>

//#include <string.h>
#include <cstring>

class mem_buffer
  : public buffer_base
{
public:
  ~mem_buffer()
  {
    this->close();
  }

  mem_buffer()
    : buffer_base( 0, 0 )
    , _fd(-1)
  {
  }

  mem_buffer(const char* path, size_t size = 0)
    : buffer_base(0, 0 )
    , _fd(-1)
  {
    this->open( path, size );
  }

  void close()
  {
    this->sync(false);

    if (_fd!=-1)
      ::close( _fd );
    //std::cout << "mmap_buffer::close" << std::endl;
  }

 
  bool open(const char* path, size_t size = 0)
  {

    _fd = ::open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

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

    /*
    MAP_HUGETLB (since Linux 2.6.32)
    Allocate the mapping using "huge pages."  See the kernel source file
    Documentation/vm/hugetlbpage.txt for further information.

     MAP_NORESERVE
     Do not reserve swap space for this mapping.  When swap space is
     reserved, one has the guarantee that it is possible to modify the
    mapping.  When swap space is not reserved one might get SIGSEGV upon a
    write if no physical memory is available.  See also the discussion of
              the file /proc/sys/vm/overcommit_memory in proc(5).  In kernels before
              2.6, this flag only had effect for private writable mappings.

    */

    
    _addr = new char[size];

    if (_addr == 0)
    {
      int err = errno;
      this->close();
      throw std::domain_error( std::strerror(err));
      return false;
    }

    lseek(_fd, 0, SEEK_SET);
    ::read( _fd, _addr, size);
    
    _size = size;
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
    this->sync(false);
    delete[] _addr;
    _addr = new char[size];
    lseek(_fd, 0, SEEK_SET);
    ::read( _fd, _addr, size);
    _size = size;

    /*
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

    _size = 0;
    _addr = (char*) ::mremap( _addr, _size, size, MREMAP_MAYMOVE);
    if ( _addr == MAP_FAILED )
      return false;
    _size = size;*/
    return true;
  }

  operator bool () const
  {
    return _fd != -1;
  }

  void sync( bool async = false )
  {
    lseek(_fd, 0, SEEK_SET);
    ::write( _fd, _addr, _size);
  }

private:
  int _fd;
};

#endif
