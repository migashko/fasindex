#include <pmi/vset.hpp>
#include <fas/xtime.hpp>
#include <climits>
#include <limits>
#include <limits.h>

#include <set>
/*
#define COUNT 1000000L
#define MAX_SIZE 1000000000L
*/

#define COUNT 100L
// #define MAX_SIZE 10000000L
#define MAX_SIZE 1000000L
#define MAX_SIZE2 (MAX_SIZE - MAX_SIZE/10 )

template<typename C>
void test_container(C& cont, std::string head)
{
  std::cout << std::endl << std::endl << "+++++++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cout << head << std::endl;
  fas::nanospan start = fas::process_nanotime();
  fas::nanospan finish = start;

  for (int i =0 ; i < MAX_SIZE; i++ )
  {
    int value = i*(std::rand() * std::rand() )  % INT_MAX;
    cont.insert(value);
  }

  finish = fas::process_nanotime();

  std::cout << MAX_SIZE << ": " << MAX_SIZE * fas::rate(finish - start) << " persec" << std::endl;
  // std::cout << "(" << finish - begin << ") " << i / COUNT << " млн : " << finish - start  << ", " << COUNT * fas::rate(finish - start) << " persec" << std::endl;

  start = fas::process_nanotime();
  size_t dist = std::distance(cont.begin(), cont.end() );
  finish = fas::process_nanotime();
  std::cout << "distance: " <<  dist<< ", time: " << finish - start << ", rate: " << fas::rate(finish - start) << " persec" << std::endl;
  std::cout << "size: " << cont.size() << std::endl;
  
  std::cout << "-----------------------------------------" << std::endl;
}

template<typename C>
void test_erase(C& cont, std::string head)
{
  std::cout << std::endl << std::endl << "+++++++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cout << head << std::endl;
  fas::nanospan start = fas::process_nanotime();
  fas::nanospan finish = start;

  for (int i =0 ; i < MAX_SIZE2; i++ )
  {
    int value = i*(std::rand() * std::rand() )  % INT_MAX;
    auto itr = cont.lower_bound(value);
    if (itr==cont.end() )
      itr = cont.begin();
    if (itr!=cont.end() )
      cont.erase(itr);
    //cont.insert(value);
  }

  finish = fas::process_nanotime();

  std::cout << MAX_SIZE2 << ": " << MAX_SIZE2 * fas::rate(finish - start) << " persec" << std::endl;
  // std::cout << "(" << finish - begin << ") " << i / COUNT << " млн : " << finish - start  << ", " << COUNT * fas::rate(finish - start) << " persec" << std::endl;

  start = fas::process_nanotime();
  size_t dist = std::distance(cont.begin(), cont.end() );
  finish = fas::process_nanotime();
  std::cout << "distance: " <<  dist<< ", time: " << finish - start << ", rate: " << fas::rate(finish - start) << " persec" << std::endl;
  std::cout << "size: " << cont.size() << std::endl;
  std::cout << "-----------------------------------------" << std::endl;
}

template<typename C>
size_t overage(const C& c)
{
  if (c.empty())
    return 0;
  return (c.capacity() * 100 / (c.size() + 1)) - 100;
}

int main(int argc, char* argv[])
{

  typedef vset_helper<int, std::less<int>, alloc_type::persistent, 1024 > helper1;
  typedef helper1::vset_type set_type1;
  helper1::buffer_type buffer1;
  buffer1.open("vset1.bin", MAX_SIZE);
  buffer1.clear();
  helper1::memory_manager mm1(buffer1);
  set_type1 s1( (helper1::comparator()), (helper1::allocator_type(mm1)) ) ;
  test_container( s1, "persistent 1024");
  std::cout << "capacity: " << s1.capacity() << std::endl;
  std::cout << "overage " << overage(s1) << "%" << std::endl;
  test_erase( s1, "persistent erase 1024");
  std::cout << "capacity: " << s1.capacity() << std::endl;
  std::cout << "overage " << overage(s1) << "%" << std::endl;


  typedef vset_helper<int, std::less<int>, alloc_type::inmemmory, 1024 > helper2;
  typedef helper2::vset_type set_type2;
  set_type2 s2;
  test_container( s2, "inmemmory 1024");
  std::cout << "capacity: " << s2.capacity() << std::endl;
  std::cout << "overage " << overage(s2) << "%" << std::endl;
  test_erase( s2, "inmemmory erase 1024");
  std::cout << "capacity: " << s2.capacity() << std::endl;
  std::cout << "overage " << overage(s2) << "%" << std::endl;
  
  std::multiset<int> s3;
  test_container( s3, "std::set");
  test_erase( s3, "std::set erase");

  /*
  typedef vset_helper<int, std::less<int>, alloc_type::inmemmory, 2048 > helper4;
  typedef helper4::vset_type set_type4;
  set_type4 s4;
  test_container( s4, "inmemmory 2048");

  typedef vset_helper<int, std::less<int>, alloc_type::inmemmory, 490 > helper5;
  typedef helper5::vset_type set_type5;
  set_type5 s5;
  test_container( s5, "inmemmory 512");
  */

  
  /*
  // typedef vset<int> set_type;
  typedef vset_helper<int, std::less<int>, alloc_type::persistent >::vset_type set_type;
  typedef set_type::value_compare value_compare;
  typedef set_type::allocator_type allocator_type;
  typedef allocator_type::allocation_manager allocation_manager;
  typedef allocation_manager::buffer buffer_type;

  buffer_type buffer;
  buffer.open("set.bin", MAX_SIZE*4L);
  buffer.clear();
  allocation_manager manager(buffer);
  set_type s = set_type(value_compare(), allocator_type(manager)) ;
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

  */
  return 0;
}
