#include "sorted_vector.hpp"
#include <vector>
#include <functional>

int main(int argc, char* argv[])
{
  typedef std::vector<int> vector_type;
  sorted_vector< vector_type, std::less<int> > sv;
  return 0;
}
