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
#include <fas/xtime.hpp>

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

#define MAX_COUNT (1024*1024*1)

int main()
{
  rlimit rl={1024*1024*1024, 1024*1024*1024 };
  setrlimit(RLIMIT_DATA, &rl );
  
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

  typedef vector_list<data, 4096, 4096> vector_type;
  
  typedef vector_type::index_allocator index_allocator;
  typedef vector_type::value_allocator value_allocator;

  fixed_size_blocks_allocation<value_allocator::value_type, mmap_buffer> am1((&mmm));
  fixed_size_blocks_allocation<index_allocator::value_type, mmap_buffer> am2((&mmm2));

  vector_type vv(am1, am2);

  data d;
  for ( size_t i=0 ; i < MAX_COUNT; i++)
  {
    d.index = i;
    vv.push_back(d);
  }

  std::cout << "test insert " << std::endl;
  vector_type::iterator cur = vv.begin() + 1000/*MAX_COUNT/2*/;
  d.index = 3333;
  cur = vv.insert( cur, d );
  std::cout << cur->index << std::endl;

  //typedef std::iterator_traits< unsigned* const >::difference_type difference_type;
  std::cout << "benchmark " << std::endl;

  vector_type::iterator beg = vv.begin();
  //vector_type::iterator end = vv.end();
  fas::nanospan start = fas::process_nanotime();
  for ( int i=0 ; i < MAX_COUNT; i++, ++beg)
  {
    /*
    beg = vv.begin();
    beg += 1024 *  1024 -1;
    beg -= i*/ /*1024 *  1024 - 1*/;
    //std::cout << beg->index << " i==" << i << std::endl;
    if (beg->index!=/*1024 *  1024 - 1 - */i)
    //if (vv.at(i).index!=i)
    {
      std::cout << "fuck " << beg->index << std::endl;
      exit(1);
    }
  }
  fas::nanospan finish = fas::process_nanotime();

  std::cout << "time: " << finish - start << std::endl;
  std::cout << "rate: " << fas::rate(finish - start) * MAX_COUNT << std::endl;
  
  
  /*std::cout << itr->index << std::endl;
  ++itr;
  std::cout << itr->index << std::endl;
  */
  
  
  
  return 0;
}
