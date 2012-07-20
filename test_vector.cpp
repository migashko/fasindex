#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>


#include "vector.hpp"
#define MAX_COUNT (1024*1024*1)

struct data
{
  data(): index() { std::fill_n(buffer, 8, 0); }
  data(int index): index(index) { std::fill_n(buffer, 8, 0xABABABABABABABAB); }
  int index;
  size_t buffer[8];
};

typedef allocator_helper<data>::mmap<7, 13> helper;
typedef vector<data, helper::value_allocator, helper::index_allocator> vector_type;

void test_push_back_impl(int count, vector_type& v)
{
  //std::cout << "test_push_back_impl { "  << count<< std::endl;
  //v.clear();
  for (int i = 0; i < count; ++i )
  {
    v.push_back( data(i) );

    if ( v[v.size()-1].index != i)
      throw;
  }
  //std::cout << "} test_push_back_impl "  << count<< std::endl;
}

void test_push_back(vector_type& v)
{
  for (int i = 0 ; i < 1024; ++i)
  {
    //!std::cout << i << std::endl;
    test_push_back_impl( i, v);
    //v.show_last_index();
  }
  //!std::cout << v.size() << std::endl;
};

void check_push_back(vector_type& v)
{
  size_t n = 0;
  for (int i = 0 ; i < 1024; ++i)
  {
    for (int j = 0 ; j < i; ++j)
    {
      int value = v[n].index;
      if ( value != j )
      {
        std::cout << "fail" << std::endl;
        v.show_last_index();
        exit(1);
      }
      n++;
    }
  }
}


int main(int argc, char* argv[])
{
  /*rlimit rl={1024*1024*1024, 1024*1024*1024 };
  setrlimit(RLIMIT_DATA, &rl );*/
  helper::buffer_type index_buffer;
  helper::buffer_type value_buffer;

  index_buffer.open("index_buffer.bin", 1000);
  //index_buffer.clear();
  value_buffer.open("value_buffer.bin", 1000);
  //value_buffer.clear();


  helper::index_allocate_manager iam(index_buffer);
  helper::value_allocate_manager vam(value_buffer);

  vector_type v = vector_type( helper::value_allocator(vam), helper::index_allocator(iam) );
  size_t dist = iam.end() - iam.begin();
  std::cout << "dist " << dist << std::endl;
  test_push_back(v);
  check_push_back(v);
  if ( argc == 1 )
  {
    std::cout << "clear" << std::endl;
    v.clear();
  }
  return 0;
}
