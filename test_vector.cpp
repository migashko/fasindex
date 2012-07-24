#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>


#include "vector.hpp"
#define MAX_COUNT (1024*3)
#ifdef NDEBUG
#define COEF 1
#else
#define COEF 20
#endif

struct data
{
  data(): index() { std::fill_n(buffer, 8, 0); }
  data(int index): index(index) { std::fill_n(buffer, 8, 0xABABABABABABABAB); }
  size_t index;
  size_t buffer[8];
};

//typedef allocator_helper<data>::mmap<3, 3> helper;
typedef allocator_helper<data>::mmap<4, 4> helper;
typedef vector<data, helper::value_allocator, helper::index_allocator> vector_type;

/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////

void test_push_back_fill_impl(int count, vector_type& v)
{
  for (int i = 0; i < count; ++i )
  {
    v.push_back( data(i) );

    if ( v[v.size()-1].index != i)
      throw;
  }
}

void test_push_back_fill(vector_type& v)//
{
  for (int i = 0 ; i < MAX_COUNT/COEF ; ++i)
  {
    //!std::cout << i << std::endl;
    test_push_back_fill_impl( i, v);
    //v.show_last_index();
  }
  //!std::cout << v.size() << std::endl;
};

void test_push_back_check(vector_type& v)
{
  size_t n = 0;
  for (int i = 0 ; i < MAX_COUNT/1024/COEF; ++i)
  {
    for (int j = 0 ; j < i; ++j)
    {
      int value = v[n].index;
      if ( value != j )
      {
        std::cout << "FAIL" << std::endl;
        v.show_last_index();
        exit(1);
      }
      n++;
    }
  }
}

void test_push_back(bool clear)
{
  std::cout << "test_push_back" << std::endl;
  helper::buffer_type index_buffer;
  helper::buffer_type value_buffer;

  index_buffer.open("push_back_index.bin", 10000000);
  value_buffer.open("push_back.bin", 100000000);

  helper::index_allocate_manager iam(index_buffer);
  helper::value_allocate_manager vam(value_buffer);
  typedef helper::index_allocate_manager::pointer index_pointer;

  vector_type v = vector_type( helper::value_allocator(vam), helper::index_allocator(iam) );
  v.restore(iam.begin(), iam.end());
  if ( v.empty() )
    test_push_back_fill(v);
  test_push_back_check(v);
  if (clear)
    v.clear();
}


/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////


void test_insert_fill(vector_type& v)
{
  size_t index = 0;
  for (size_t i = 0 ; i <  MAX_COUNT/4; ++i)
  {
    v.insert(v.begin(), data(index++) );
  }
  for (size_t i = 0 ; i <  MAX_COUNT/4; ++i)
  {
    index++;
    //std::cout << MAX_COUNT - MAX_COUNT/4 + i << " "; 
    v.insert(v.begin() + MAX_COUNT/4, data(MAX_COUNT - MAX_COUNT/4 + i ) );
  }
  std::cout << std::endl;
  std::cout << "-------------------" << std::endl;
  std::cout << std::endl;
  for (size_t i = 0 ; i <  (MAX_COUNT/2/* - MAX_COUNT/4*/) ; ++i)
  {
    index++;
    //std::cout << MAX_COUNT/2 - MAX_COUNT/4 + i << " "; 
    v.insert(v.begin() + MAX_COUNT/4, data(MAX_COUNT/2 - MAX_COUNT/4 + i) );
  }
  std::cout << std::endl;

  if (v.size() != MAX_COUNT)
  {
    std::cout << "insert FAIL " << v.size() << "!=" << MAX_COUNT << " index=" << index << std::endl;
    exit(1);
  }
  
  std::reverse(v.begin(), v.begin() + MAX_COUNT/4 );
  std::reverse(v.begin() + MAX_COUNT/4 , v.begin() + MAX_COUNT/4 + MAX_COUNT/2 );
  std::reverse(v.begin() + MAX_COUNT/4 + MAX_COUNT/2 , v.end());
  std::cout << MAX_COUNT/4 << std::endl;
  std::cout << MAX_COUNT/4 + MAX_COUNT/2 << std::endl;
}

void test_insert_check(vector_type& v)
{
  std::cout << "test_insert_check(vector_type& v)" << std::endl;
  size_t index = 0;
  for (size_t i = 0 ; i <  MAX_COUNT; ++i)
  {
      int value = v[index].index ;
      if ( value != i )
      {
        std::cout << "FAIL " << i << std::endl;
        v.dump();
        exit(1);
      }
      index++;
  }
 
}

void test_insert(bool clear)
{
  std::cout << "test_insert" << std::endl;
  helper::buffer_type index_buffer;
  helper::buffer_type value_buffer;

  index_buffer.open("insert_index.bin", 1000);
  value_buffer.open("insert.bin", 1000);

  helper::index_allocate_manager iam(index_buffer);
  helper::value_allocate_manager vam(value_buffer);
  typedef helper::index_allocate_manager::pointer index_pointer;
  typedef helper::value_allocate_manager::pointer value_pointer;

  vector_type v = vector_type( helper::value_allocator(vam), helper::index_allocator(iam) );
  
  v.restore(iam.begin(), iam.end());
  
  if ( v.empty() )
    test_insert_fill(v);
  
  test_insert_check(v);
  if (clear)
    v.clear();
}

/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////

void test_erase_fill(vector_type& v)
{
  std::cout << "test_erase_fill(vector_type& v)" << std::endl;
  size_t size = v.size();
  for ( size_t i = 0 ; i < size/2; ++i)
    v.erase(v.begin() + i);
}

void test_erase_check(vector_type& v)
{
  std::cout << "test_erase_check(vector_type& v)" << std::endl;
  size_t index = 1;
  for (size_t i = 0 ; i <  MAX_COUNT/2; ++i, index+=2)
  {
      int value = v[i].index ;
      if ( value != index )
      {
        v.dump();
        std::cout << "erase check FAIL i=" << i << " index=" << index << std::endl;
        //v.show_last_index();
        exit(1);
      }
      //index++;
  }

}


void test_erase(bool clear)
{
  std::cout << "test_erase" << std::endl;
  helper::buffer_type index_buffer;
  helper::buffer_type value_buffer;

  index_buffer.open("erase_index.bin", 1000);
  value_buffer.open("erase.bin", 1000);

  helper::index_allocate_manager iam(index_buffer);
  helper::value_allocate_manager vam(value_buffer);
  typedef helper::index_allocate_manager::pointer index_pointer;
  typedef helper::value_allocate_manager::pointer value_pointer;

  vector_type v = vector_type( helper::value_allocator(vam), helper::index_allocator(iam) );

  v.restore(iam.begin(), iam.end());

  if ( v.empty() )
  {
    test_insert_fill(v);
    test_erase_fill(v);
  }

  test_erase_check(v);
  if (clear)
    v.clear();
}


/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  test_push_back( argc > 1 );
  test_insert(argc > 1);
  test_erase(argc > 1);
  rlimit rl={1024*1024*1024, 1024*1024*1024 };
  setrlimit(RLIMIT_DATA, &rl );

  return 0;
}
