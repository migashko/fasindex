#include <pmi/set.hpp>
#include <fas/xtime.hpp>
#include <climits>
#include <limits>
#include <limits.h>

#define COUNT 1000000L
#define MAX_SIZE 1000000000L
int main(int argc, char* argv[])
{
  typedef set<int> set_type;
  typedef set_type::value_compare value_compare;
  typedef set_type::allocator_type allocator_type;
  typedef allocator_type::allocation_manager allocation_manager;
  typedef allocation_manager::buffer buffer_type;

  buffer_type buffer;
  buffer.open("set.bin", MAX_SIZE*4L);
  allocation_manager manager(buffer);
  set<int> s = set<int>(value_compare(), allocator_type(manager)) ;
  std::vector<int> vv;

  fas::nanospan start = fas::nanotime();
  fas::nanospan finish = start;
  fas::nanospan begin = start;
  for (int i =0 ; i < MAX_SIZE; i++ )
  {
    if ( i % COUNT == 0 )
    {
      finish = fas::nanotime();
      std::cout << "(" << finish - begin << ") " << i / COUNT << " млн : " << finish - start  << ", " << COUNT * fas::rate(finish - start) << " persec" << std::endl;
      start = finish;
    }
    int value = (std::rand() * std::rand() )  % INT_MAX;
    //int value = std::rand()  % 256;
    s.insert( value  );
    vv.push_back(value);
    //buffer.sync(true);
  }

  /*
  int pred = 0;
  int i = 0;
  std::sort(vv.begin(), vv.end());
  std::for_each( s.begin(), s.end(), [&i, &pred, &vv](int v)
  {
    
    std::cout << i << ":" << v << " " << vv[i]<< std::endl;
    i++;
    
    // std::cout << i++ << ":" << v << " "; std::cout.flush(); if ( v < pred || v != vv[i] ) std::cout << "|" <<  std::endl;; pred = v;}
  });
  */

  //std:: cout << s.size() << std::endl;
  return 0;
}
