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

typedef allocator_helper<data>::mmap<3, 3> helper;
typedef vector<data, helper::value_allocator, helper::index_allocator> vector_type;

/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////


void test_insert_fill(vector_type& v)
{
  size_t index = 0;
  for (size_t i = 0 ; i <  MAX_COUNT/4; ++i)
  {
    //std::cout << index << std::endl;
    v.insert(v.begin(), data(index++) );
  }

  //std::for_each(v.begin(), v.end(), [](const data& d) { std::cout << d.index << ",";} ); std::cout << std::endl;
  std::reverse(v.begin(), v.end() );
  //std::for_each(v.begin(), v.end(), [](const data& d) { std::cout << d.index << ",";} ); std::cout << std::endl;
  
}

void test_insert_check(vector_type& v)
{
  size_t index = 0;
  for (size_t i = 0 ; i <  MAX_COUNT/4; ++i)
  {
      //std::cout << "i=" << i << std::endl;
      int value = v[index].index ;
      if ( value != i )
      {
        v.show_last_index();
        exit(1);
      }
      index++;
  }
}

void test_insert(bool clear)
{
  helper::buffer_type index_buffer;
  helper::buffer_type value_buffer;

  index_buffer.open("insert_index.bin", 1000);
  value_buffer.open("insert.bin", 1000);

  helper::index_allocate_manager iam(index_buffer);
  helper::value_allocate_manager vam(value_buffer);
  typedef helper::index_allocate_manager::pointer index_pointer;
  typedef helper::value_allocate_manager::pointer value_pointer;

  vector_type v = vector_type( helper::value_allocator(vam), helper::index_allocator(iam) );
  std::cout << "insert check dst1: " << iam.end() - iam.begin() << std::endl;
  v.restore(iam.begin(), iam.end());
  std::cout << "insert check dst2: " << iam.end() - iam.begin() << std::endl;
  if ( v.empty() )
    
    test_insert_fill(v);
  std::cout << "insert check dst3: " << iam.end() - iam.begin() << std::endl;
  
  {

    value_pointer vbeg = vam.begin();
    value_pointer vend = vam.end();

    std::vector<data> tmp;
    for ( ; vbeg!=vend; ++vbeg)
    {
      std::for_each(vbeg->begin(), vbeg->end(), [&tmp](const data& d){tmp.push_back(d); });
    }
    std::sort(tmp.begin(), tmp.end(), [](const data& f, const data& s)->bool { return f.index < s.index; });
    //std::for_each(tmp.begin(), tmp.end(), [](const data& d) { std::cout << d.index << "->";} ); std::cout << std::endl;
    
    
  }
  std::cout << "insert check" << std::endl;
  test_insert_check(v);
  if (clear)
  {
    std::cout << "insert clear" << std::endl;
    v.clear();
  }
}

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
        std::cout << "fail" << std::endl;
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
  std::cout << "push_back restore " <<  iam.end() - iam.begin() << std::endl;
  v.restore(iam.begin(), iam.end());
  if ( v.empty() )
  {
    std::cout << "push_back fill" << std::endl;
    test_push_back_fill(v);
  }
  std::cout << "push_back check" << std::endl;
  test_push_back_check(v);
  if (clear)
  {
    std::cout << "push_back clear" << std::endl;
    v.clear();
  }
}

/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////
/// //////////////////////////////////////////////////

int main(int argc, char* argv[])
{
 // test_push_back( argc > 1 );
  test_insert(argc > 1);
  rlimit rl={1024*1024*1024, 1024*1024*1024 };
  setrlimit(RLIMIT_DATA, &rl );
  
  return 0;
}
