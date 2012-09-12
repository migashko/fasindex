#include "sorted_vector.hpp"
#include <vector>
#include <set>
#include <functional>
#include <pmi/vector/sorted_array.hpp>

#include <pmi/allocator.hpp>
#include <pmi/allocator.hpp>


int main(int argc, char* argv[])
{
  typedef std::vector<int> vector_type;
  sorted_vector< vector_type, std::less<int> > sv;

  /*std::set<int> s;
  *(s.begin()) = 10;
  */


  sorted_array<int, 1024> sa;
  sa.erase(sa.begin());




  return 0;
}
