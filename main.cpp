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
#include "offset_iterator.hpp"
#include "vector.hpp"



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
  typedef value_array<data,  128> data_array;
  typedef index_array<size_t, 256> index_array;

  typedef fixed_size_blocks_allocation<data_array, mmap_buffer> data_buffer_manager;
  typedef fixed_size_blocks_allocation<index_array, mmap_buffer> index_buffer_manager;

  typedef managed_allocator< data_array, data_buffer_manager > data_allocator;
  typedef managed_allocator< index_array, index_buffer_manager > index_allocator;

  //template<typename T, int N, typename P, typename A>
  typedef array_proxy< /*index_type, 256*//*index_array,*/ index_allocator::pointer, data_allocator> arr_of_arr;

  data_buffer_manager dbm(buffer2);
  index_buffer_manager ibm(buffer1);

  data_allocator da(dbm);
  index_allocator ia(ibm);

  index_allocator::pointer iptr = ia.allocate(1);
  ia.construct(iptr, index_allocator::value_type() );

  //data_allocator::pointer data_ptr(&dbm);

  arr_of_arr aoa( iptr, /*data_ptr,*/ da );
  
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

  aoa.erase( 333, 555);
  //aoa.erase( 333);

  int coef = 0;
  for (int i =0; i < 16000 - coef; ++i)
  {
    if (i==333)
      coef = 555 - 333;
    /*if (i==555)
      coef = 0;*/
    if ( aoa[i].index!=i+coef )
      std::cout << aoa[i].index << "!=" << i << std::endl;
  };
  
  //std::cout << aoa.can_insert(10, -1) << std::endl;
  

};

void test_vector_list()
{
  mmap_buffer buffer1, buffer2;

  if ( !buffer1.open("./vector_index.bin", 1024) )
    std::cout << "fuck1" << std::endl;
  if (!buffer1)
    std::cout << "fuck2" << std::endl;

  if ( !buffer2.open("./vector_data.bin", 1024) )
    std::cout << "2 fuck1" << std::endl;
  if (!buffer2)
    std::cout << "2 fuck2" << std::endl;

  std::cout << "OK" << std::endl;

  typedef size_t index_type;
  typedef value_array<data,  128> data_array;
  typedef index_array<size_t, 256> index_array;

  typedef fixed_size_blocks_allocation<data_array, mmap_buffer> data_buffer_manager;
  typedef fixed_size_blocks_allocation<index_array, mmap_buffer> index_buffer_manager;

  typedef managed_allocator< data_array, data_buffer_manager > data_allocator;
  typedef managed_allocator< index_array, index_buffer_manager > index_allocator;

  typedef vector<data, data_allocator, index_allocator> vector_list_type;

  std::cout << "test_vector_list()" << std::endl;
  data_buffer_manager dbm(buffer2);
  index_buffer_manager ibm(buffer1);

  data_allocator da(dbm);
  index_allocator ia(ibm);

  vector_list_type vl(da, ia);
  data d;
  for (int i=0; i < 16000; ++i)
  {
    d.index = i;
    vl.push_back(d);
  }

  std::cout << "-------" << std::endl;
  for (int i =0; i < 16000; ++i)
  {
    //std::cout << vl[i].index << std::endl;
    if ( vl[i].index!=i )
      std::cout << vl[i].index << "!=" << i << std::endl;
  }


  
}


int main()
{
  rlimit rl={1024*1024*1024, 1024*1024*1024 };
  setrlimit(RLIMIT_DATA, &rl );

  test_array_of_array();
  test_vector_list();
  return 0;
}
