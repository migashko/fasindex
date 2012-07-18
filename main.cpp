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

void test_array_of_array()
{
  mmap_buffer buffer1, buffer2;

  if ( !buffer1.open("./buffer_index.bin", 1024) )
    std::cout << "fuck1" << std::endl;
  if (!buffer1)
    std::cout << "fuck2" << std::endl;

  if ( !buffer2.open("./buffer_data.bin", 1024) )
    std::cout << "2 fuck1" << std::endl;
  if (!buffer2)
    std::cout << "2 fuck2" << std::endl;

  std::cout << "OK" << std::endl;

  typedef size_t index_type;
  typedef last_index_array<data,  128> data_array;
  typedef last_index_array<size_t, 256> index_array;

  typedef fixed_size_blocks_allocation<data_array, mmap_buffer> data_buffer_manager;
  typedef fixed_size_blocks_allocation<index_array, mmap_buffer> index_buffer_manager;

  typedef managed_allocator< data_array, data_buffer_manager > data_allocator;
  typedef managed_allocator< index_array, index_buffer_manager > index_allocator;

  //template<typename T, int N, typename P, typename A>
  typedef array_of_array<index_type, 256, index_allocator::pointer, data_allocator> arr_of_arr;

  data_buffer_manager dbm(&buffer2);
  index_buffer_manager ibm(&buffer1);

  data_allocator da(dbm);
  index_allocator ia(ibm);

  index_allocator::pointer iptr = ia.allocate(1);
  ia.construct(iptr, index_allocator::value_type() );

  data_allocator::pointer data_ptr(&dbm);

  arr_of_arr aoa( iptr, data_ptr, da );
  
  data d;
  // 32768
  for (int i=0; i < 1000; ++i)
  {
    d.index = i;
    aoa.push_back(d);
  }

  for (int i=10000; i < 16000; ++i)
  {
    d.index = i;
    aoa.push_back(d);
  }

  std::vector<data> vd;
  for (int i=1000; i < 10000; ++i)
  {
    //std::cout << i << std::endl;
    d.index = i;
    vd.push_back(d);
    //aoa.insert(i, d);
  }

  aoa.insert(1000, vd.begin(), vd.end());
  std::cout << "--------------" << std::endl;
  for (int i =0; i < 16000; ++i)
  {
    //std::cout << aoa.can_insert(i, -1) << std::endl;
    if ( aoa[i].index!=i )
    {
      std::cout << aoa[i].index << "!=" << i << std::endl; 
      //exit(0);
    }
  }
  std::cout << aoa.can_insert(10, -1) << std::endl;
  

};


int main()
{
  rlimit rl={1024*1024*1024, 1024*1024*1024 };
  setrlimit(RLIMIT_DATA, &rl );

  test_array_of_array();
  return 0;
  
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
