#include <pmi/vset.hpp>
#include <fas/xtime.hpp>
#include <climits>
#include <limits>
#include <limits.h>

/*
#define COUNT 1000000L
#define MAX_SIZE 1000000000L
*/

#define COUNT 100L
#define MAX_SIZE 4L

int main(int argc, char* argv[])
{
  typedef vset<int> set_type;
  typedef set_type::value_compare value_compare;
  typedef set_type::allocator_type allocator_type;
  typedef allocator_type::allocation_manager allocation_manager;
  typedef allocation_manager::buffer buffer_type;

  buffer_type buffer;
  buffer.open("set.bin", MAX_SIZE*4L);
  allocation_manager manager(buffer);
  vset<int> s = vset<int>(value_compare(), allocator_type(manager)) ;
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
      buffer.sync(true);
      start = finish;
    }
    int value = i*(std::rand() * std::rand() )  % INT_MAX;
    //int value = std::rand()  % 256;
    s.insert( value  );
    //vv.push_back(value);
  }

  int first = *( s.begin() + 1 );
  int last = *( s.end() -2 );
  s.erase( s.begin() );
  s.erase( s.end() - 1 );

  if ( *s.begin() != first )
  {
    std::cout << first << std::endl;
    std::cout << *s.begin() << std::endl;
    abort();
  }

  if ( *(s.end()-1) != last )
  {
    std::cout << last << std::endl;
    std::cout << *(s.end()-1) << std::endl;
    abort();
  }

  for (int i = 0 ; i < 10 ; ++i)
      s.insert(43);

  for (int i = 0 ; i < 10 ; ++i)
      std::cout << "-->" << *(s.begin() + i) << std::endl;

  auto range = s.equal_range(43);

  if ( std::distance(range.first, range.second) != 10 )
  {
    //std::cout << *(range.first) << std::endl;
    std::cout << *(range.second-3) << std::endl;
    std::cout << std::distance(range.first, range.second) << std::endl;
    std::cout << range.second - range.first << std::endl;

    for ( ; range.first!=range.second; ++range.first )
    {
      std::cout << "fail:" << *range.first << std::endl;
    }
    throw std::logic_error( "std::distance(range.first, range.second)" );
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
