#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
       
#include <functional>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <memory>
#include <iterator>

#include <fas/range.hpp>

#include "mmap_buffer.hpp"
//#include "offset_pointer.hpp"
#include "fixed_size_blocks_allocation.hpp"
#include "managed_allocator.hpp"
#include "array.hpp"
#include "vector_list_iterator.hpp"
#include "vector_list.hpp"



#include <array>

struct data
{
  data(): index() { std::fill_n(buffer, 8, 0xABABABABABABABAB); }
  int index;
  size_t buffer[8];
};

int main()
{
  mmap_buffer mmm, mmm2;
  
  if ( !mmm.open("./data.bin", 1024) )
    std::cout << "fuck1" << std::endl;
  if (!mmm)
    std::cout << "fuck2" << std::endl;

  if ( !mmm2.open("./index.bin", 1024) )
    std::cout << "fuck1" << std::endl;
  if (!mmm2)
    std::cout << "fuck2" << std::endl;

  std::cout << "OK" << std::endl;

  /*allocate_manager<int> am1((&mmm));
  allocate_manager<int> am2((&mmm2));*/
  
  //the_allocator<int> alloca = the_allocator<int>(am1);

  typedef vector_list<data, 128, 128> vector_type;
  
  typedef vector_type::index_allocator index_allocator;
  typedef vector_type::value_allocator value_allocator;

  fixed_size_blocks_allocation<value_allocator::value_type, mmap_buffer> am1((&mmm));
  fixed_size_blocks_allocation<index_allocator::value_type, mmap_buffer> am2((&mmm2));

  vector_type vv(am1, am2);

  data d;
  for ( size_t i=0 ; i < 1024 *  1024  /** 100*/; i++)
  {
    d.index = i;
    vv.push_back(d);
  }

  //typedef std::iterator_traits< unsigned* const >::difference_type difference_type;

  vector_type::iterator beg = vv.begin();
  //vector_type::iterator end = vv.end();
  for ( int i=0 ; i < 1024 *  1024  /** 100*/; i++, ++beg)
  {
    beg = vv.begin();
    beg += i;
    std::cout << beg->index << " i==" << i << std::endl;
    if (beg->index!=i)
    {
      std::cout << "fuck" << std::endl;
      exit(1);
    }
  }
  
  
  /*std::cout << itr->index << std::endl;
  ++itr;
  std::cout << itr->index << std::endl;
  */
  
  
  
  return 0;
}
