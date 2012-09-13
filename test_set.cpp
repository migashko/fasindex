#include <pmi/set.hpp>

int main(int argc, char* argv[])
{
  typedef set<int> set_type;
  typedef set_type::value_compare value_compare;
  typedef set_type::allocator_type allocator_type;
  typedef allocator_type::allocation_manager allocation_manager;
  typedef allocation_manager::buffer buffer_type;

  buffer_type buffer;
  buffer.open("set.bin", 1024);
  allocation_manager manager(buffer);
  set<int> s = set<int>(value_compare(), allocator_type(manager)) ;


  for (int i =0 ; i < 1024; i++ )
  {
    s.insert(i);
  }
  return 0;
}
