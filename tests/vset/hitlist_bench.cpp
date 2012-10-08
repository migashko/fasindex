#include <pmi/vset.hpp>
#include <fas/xtime.hpp>
#include <climits>
#include <limits>
#include <limits.h>

#include <set>

#define MAX_BUFFER (1024L)
#define COUNT 1000000L
#define MAX_SIZE (COUNT*200)
#define MAX_SIZE_ERASE (MAX_SIZE - MAX_SIZE/10 )



typedef unsigned int anketa_id_t;
// Ахтунг! в 03:14:07, вторник, 19 января 2038 года по UTC, демон перестанет работать
typedef unsigned int hittime_t;
typedef int type_id_t;
typedef size_t offset_type;

struct hit
{
  anketa_id_t src; // Кто смотрел
  anketa_id_t dst; // Кого смотрели
  hittime_t   hittime;
  type_id_t   type_id;
};

struct f_dst_by_time
{
  bool operator()(const hit& f, const hit& s) const
  {
    if ( f.dst != s.dst ) return f.dst < s.dst;
    if ( f.hittime != s.hittime ) return f.hittime < s.hittime;
    return false;
  }
};

struct f_dst_by_type
{
  bool operator()(const hit& f, const hit& s) const
  {
    if ( f.dst != s.dst ) return f.dst < s.dst;
    if ( f.type_id != s.type_id ) return f.type_id < s.type_id;
    if ( f.hittime != s.hittime ) return f.hittime < s.hittime;
    return false;
  }
};

// Кого я смотрел
struct f_src_by_time
{
  bool operator()(const hit& f, const hit& s) const
  {
    if ( f.src != s.src ) return f.src < s.src;
    if ( f.hittime != s.hittime ) return f.hittime < s.hittime;
    return false;
  }
};

template<typename T, typename Container, typename Compare>
struct comparator:
  Compare
{
  Container* _container;
  //comparator(): _container(0){}
  comparator(Container& cnt): _container(&cnt){}
  template<typename Index>
  bool operator()(const Index& f, const Index& s) const
  {
    // std::cout << (_container!=0) << std::endl;
    assert(_container!=0);
    
    return Compare::operator()(
      *(reinterpret_cast<const T*>( _container->addr() + f) ),
      *(reinterpret_cast<const T*>( _container->addr() + s) )
    );
  }
};

// struct f_dst_by_time
// struct f_dst_by_type
// struct f_src_by_time

typedef vset_helper<size_t, std::less<size_t>, alloc_type::persistent, 1024 > index_helper;

typedef vset_helper<size_t, comparator<hit, mmap_buffer, f_dst_by_time>, alloc_type::persistent, 1024 > dst_by_time_helper;
typedef vset_helper<size_t, comparator<hit, mmap_buffer, f_dst_by_type>, alloc_type::persistent, 1024 > dst_by_type_helper;
typedef vset_helper<size_t, comparator<hit, mmap_buffer, f_src_by_time>, alloc_type::persistent, 1024 > src_by_time_helper;
typedef chain_memory<hit, mmap_buffer> main_container;

typedef dst_by_time_helper::vset_type dst_by_time_index;
typedef dst_by_type_helper::vset_type dst_by_type_index;
typedef src_by_time_helper::vset_type src_by_time_index;

main_container* _main;
dst_by_time_index* _dst_by_time;
dst_by_type_index* _dst_by_type;
src_by_time_index* _src_by_time;

dst_by_time_helper::buffer_type *_dst_by_time_buffer;
dst_by_type_helper::buffer_type *_dst_by_type_buffer;
dst_by_type_helper::buffer_type *_src_by_time_buffer;

template<typename C>
size_t overage(const C& c)
{
  if (c.empty() || c.capacity() == c.size() )
    return 0;
  return (c.capacity() * 100 / c.size() ) - 100;
}

template<typename C, typename B>
void test_container(C& cont, B& buffer, std::string head)
{
  std::cout << std::endl << std::endl << "+++++++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cout << head << std::endl;
  fas::nanospan start = fas::process_nanotime();
  fas::nanospan finish = start;

  for (int i =0 ; i < MAX_SIZE; i++ )
  {
    if ( i % COUNT == 0 )
    {
      finish = fas::process_nanotime();
      std::cout << "(" << finish - start << ") " << i / COUNT << " млн : " << finish - start  << ", " << COUNT * fas::rate(finish - start) << " persec" << std::endl;
      //buffer.sync(false);
      start = finish;
      _dst_by_time_buffer->sync(false);
      _dst_by_type_buffer->sync(false);
      _src_by_time_buffer->sync(false);
      finish = fas::process_nanotime();
      std::cout << "(" << finish - start << ") " << i / COUNT << " млн : " << finish - start  << ", " << COUNT * fas::rate(finish - start) << " persec" << std::endl;
      start = finish;
      std::cout << "size1: " << _dst_by_time->size() << std::endl;
      std::cout << "capacity1: " << _dst_by_time->capacity() << std::endl;
      std::cout << "overage1: " << overage(*_dst_by_time) << "%" << std::endl;
    }

    int value = i*(std::rand() * std::rand() )  % INT_MAX;
    cont.insert(value);
  }

  finish = fas::process_nanotime();
  std::cout << "-----------------------------------------" << std::endl;
}

void test_hitlist()
{
  std::cout << std::endl << std::endl << "+++++++++++++++++++++++++++++++++++++++++" << std::endl;
  fas::nanospan start = fas::nanotime();
  fas::nanospan finish = start;

  time_t now = time(0);
  double last_time = 3600*24*30;
  double time_step = last_time/MAX_SIZE;

  for (int i =0 ; i < MAX_SIZE; i++ )
  {
    if ( i % COUNT == 0 )
    {
      finish = fas::nanotime();
      std::cout << "(" << finish - start << ") " << i / COUNT << " млн : " << finish - start  << ", " << COUNT * fas::rate(finish - start) << " persec" << std::endl;
      start = finish;
      _dst_by_time_buffer->sync(false);
      _dst_by_type_buffer->sync(false);
      _src_by_time_buffer->sync(false);
      finish = fas::nanotime();
      std::cout << "(" << finish - start << ") " << i / COUNT << " млн : " << finish - start  << ", " << COUNT * fas::rate(finish - start) << " persec" << std::endl;
      start = finish;
      //std::cout << "main size: " << _main->size() << std::endl;
      std::cout << "size1: " << _dst_by_time->size() << std::endl;
      std::cout << "capacity1: " << _dst_by_time->capacity() << std::endl;
      std::cout << "overage1: " << overage(*_dst_by_time) << "%" << std::endl;
      std::cout << "size2: " << _dst_by_type->size() << std::endl;
      std::cout << "capacity2: " << _dst_by_type->capacity() << std::endl;
      std::cout << "overage2: " << overage(*_dst_by_type) << "%" << std::endl;
      std::cout << "size3: " << _src_by_time->size() << std::endl;
      std::cout << "capacity3: " << _src_by_time->capacity() << std::endl;
      std::cout << "overage3: " << overage(*_src_by_time) << "%" << std::endl;
      std::cout << std::endl;
    }

    /*
    hit hi;
    hi.src = rand() % 1000000;
    hi.dst = rand() % 1000000;
    hi.hittime = now - (last_time + time_step*i);
    hi.type_id = rand() % 3;
    */

    main_container::pointer ptr = _main->allocate(1);
    ptr->src = rand() % 1000000;
    ptr->dst = rand() % 1000000;
    ptr->hittime = now - (last_time + time_step*i);
    ptr->type_id = rand() % 3;

    _dst_by_time->insert(ptr);
    _dst_by_type->insert(ptr);
    _src_by_time->insert(ptr);

    
    /*
    int value = i*(std::rand() * std::rand() )  % INT_MAX;
    cont.insert(value);
    */
  }

  
  std::cout << "-----------------------------------------" << std::endl;
}

int main(int argc, char* argv[])
{
  mmap_buffer main_buffer;
  main_buffer.open("hitlist.bin", MAX_BUFFER);
  _main = new main_container(main_buffer);

  _dst_by_time_buffer = new  dst_by_type_helper::buffer_type;
  _dst_by_time_buffer->open("hitlist1.bin", MAX_BUFFER);
  dst_by_time_helper::memory_manager* dst_by_time_manager = new dst_by_time_helper::memory_manager(*_dst_by_time_buffer);
  std::cout << "restore1..." << std::endl;
  _dst_by_time = new dst_by_time_index( dst_by_time_helper::comparator(main_buffer), *dst_by_time_manager );
  std::cout << "...restore1" << std::endl;

  // dst_by_type_helper::buffer_type dst_by_type_buffer;
  _dst_by_type_buffer = new dst_by_type_helper::buffer_type;
  _dst_by_type_buffer->open("hitlist2.bin", MAX_BUFFER);
  dst_by_type_helper::memory_manager* dst_by_type_manager = new dst_by_type_helper::memory_manager(*_dst_by_type_buffer);
  std::cout << "restore2..." << std::endl;
  _dst_by_type = new dst_by_type_index( dst_by_type_helper::comparator(main_buffer), *dst_by_type_manager );
  std::cout << "...restore2" << std::endl;

  // src_by_time_helper::buffer_type src_by_time_buffer;
  _src_by_time_buffer = new src_by_time_helper::buffer_type;
  _src_by_time_buffer->open("hitlist3.bin", MAX_BUFFER);
  src_by_time_helper::memory_manager* src_by_time_manager = new src_by_time_helper::memory_manager(*_src_by_time_buffer);
  std::cout << "restore3..." << std::endl;
  _src_by_time = new src_by_time_index( src_by_time_helper::comparator(main_buffer), *src_by_time_manager );
  std::cout << "...restore3" << std::endl;

  
  test_hitlist();
  /*
  typedef vset_helper<int, std::less<int>, alloc_type::persistent, 1024 > helper1;
  typedef helper1::vset_type set_type1;
  helper1::buffer_type buffer1;
  buffer1.open("vset_bench.bin", MAX_SIZE);
  // buffer1.clear();
  helper1::memory_manager mm1(buffer1);
  std::cout << "restore..." << std::endl;
  set_type1 s1( (helper1::comparator()), (helper1::allocator_type(mm1)) ) ;
  std::cout << "...restore" << std::endl;
  test_container( s1, buffer1, "persistent 1024");
  std::cout << "capacity: " << s1.capacity() << std::endl;
  std::cout << "overage " << overage(s1) << "%" << std::endl;
  */
  /*
  test_erase( s1, "persistent erase 1024");
  std::cout << "capacity: " << s1.capacity() << std::endl;
  std::cout << "overage " << overage(s1) << "%" << std::endl;
  */

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
