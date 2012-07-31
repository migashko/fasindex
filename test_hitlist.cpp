#include "offset_iterator.hpp"
#include "sorted_vector.hpp"
#include "vector.hpp"
#include <vector>
#include <functional>

#include <fas/xtime.hpp>

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
  comparator(Container& cnt): _container(&cnt){}
  template<typename Index>
  bool operator()(const Index& f, const Index& s) const
  {
    return Compare::operator()(
      *(reinterpret_cast<const T*>( _container->addr() + f) ),
      *(reinterpret_cast<const T*>( _container->addr() + s) )
    );
  }
};

/*
typedef allocator_helper<data>::mmap<4, 4> helper;
typedef vector<data, helper::value_allocator, helper::index_allocator> vector_type;
 */

class hitlist
{
  // OK
  // typedef allocator_helper<hit>::mmap<1024*32, 1024*32> hit_helper;
  // typedef allocator_helper<offset_type>::mmap<1024*32, 1024*32> index_helper;

  typedef allocator_helper<hit>::mmap<1024*64, 1024*64> hit_helper;
  typedef allocator_helper<offset_type>::mmap<1024*64, 1024*64> index_helper;

  /// /////////////////////////////////////////
  typedef hit_helper::buffer_type hit_index_buffer;
  typedef hit_helper::buffer_type hit_value_buffer;
  typedef hit_helper::index_allocate_manager hit_index_allocate_manager;
  typedef hit_helper::value_allocate_manager hit_value_allocate_manager;
  typedef hit_helper::value_allocator hit_value_allocator;
  typedef hit_helper::index_allocator hit_index_allocator;
  typedef vector<hit, hit_value_allocator, hit_index_allocator> hit_vector;

  /// /////////////////////////////////////////
  typedef index_helper::buffer_type dst_by_time_index_buffer;
  typedef index_helper::buffer_type dst_by_time_value_buffer;
  typedef index_helper::index_allocate_manager dst_by_time_index_allocate_manager;
  typedef index_helper::value_allocate_manager dst_by_time_value_allocate_manager;
  typedef index_helper::value_allocator dst_by_time_value_allocator;
  typedef index_helper::index_allocator dst_by_time_index_allocator;
  
  typedef vector<offset_type, dst_by_time_value_allocator, dst_by_time_index_allocator> dst_by_time_vector;
  typedef sorted_vector<dst_by_time_vector, comparator<hit, hit_value_buffer, f_dst_by_time> > dst_by_time_index;
  
  
public:
  hitlist()
    : _hit_index_buffer()
    , _hit_value_buffer()
    , _hit_index_allocate_manager(_hit_index_buffer)
    , _hit_value_allocate_manager(_hit_value_buffer)
    //, _hits( hit_value_allocator(_hit_value_allocate_manager), hit_index_allocator(_hit_index_allocate_manager) )

    , _dst_by_time_index_buffer()
    , _dst_by_time_value_buffer()
    , _dst_by_time_index_allocate_manager(_dst_by_time_index_buffer)
    , _dst_by_time_value_allocate_manager(_dst_by_time_value_buffer)
    /*, _dst_by_time(
        dst_by_time_vector( dst_by_time_value_allocator(_dst_by_time_value_allocate_manager), dst_by_time_index_allocator(_dst_by_time_index_allocate_manager) ),
        comparator<hit_vector, f_dst_by_time>(_hits)
      )
      */
    {
      
    }

  size_t size() const { return _hits->size();}

  
  void sync(bool async = false)
  {
    _hit_index_buffer.sync(async);
    _hit_value_buffer.sync(async);
    _dst_by_time_index_buffer.sync(async);
    _dst_by_time_value_buffer.sync(async);
  }

  void restore(const std::string& path)
  {
    std::string file;
    file = path + "/hit_index.bin";
    _hit_index_buffer.open(file.c_str(), 1024*1024*1024l);
    file = path + "/hit_value.bin";
    _hit_value_buffer.open(file.c_str(), 1024*1024*1024l);

    file = path + "/dst_by_time_index.bin";
    _dst_by_time_index_buffer.open(file.c_str(), 1024*1024*1024l);
    file = path + "/dst_by_time_value.bin";
    _dst_by_time_value_buffer.open(file.c_str(), 1024*1024*1024l);

    _hits = new hit_vector( hit_value_allocator(_hit_value_allocate_manager), hit_index_allocator(_hit_index_allocate_manager) );
    _hits->restore( _hit_index_allocate_manager.begin(), _hit_index_allocate_manager.end() );

    _dst_by_time = new dst_by_time_index(
      dst_by_time_vector(
        dst_by_time_value_allocator(_dst_by_time_value_allocate_manager),
        dst_by_time_index_allocator(_dst_by_time_index_allocate_manager)
      ),
      comparator<hit, hit_value_buffer, f_dst_by_time>(_hit_value_buffer)
    );
    
    _dst_by_time->get_container().restore( _dst_by_time_index_allocate_manager.begin(), _dst_by_time_index_allocate_manager.end() );
    
  };
    
  void insert(const hit& h)
  {
    _hits->push_back( h );
    offset_type offset = reinterpret_cast<const char*>( &(_hits->back()) ) -  _hit_value_buffer.addr();
    //_hit_value_allocate_manager.offset( &(_hits->back()) );
    _dst_by_time->insert( offset );
    this->sync(true);
  }

  size_t hits_main_index_size()
  {
    return _hits->main_index_size();
  }

  size_t dst_by_time_main_index_size()
  {
    return _dst_by_time->get_container().main_index_size();
  }

private:
  hit_index_buffer _hit_index_buffer;
  hit_value_buffer _hit_value_buffer;
  hit_index_allocate_manager _hit_index_allocate_manager;
  hit_value_allocate_manager _hit_value_allocate_manager;
  hit_vector* _hits;

  dst_by_time_index_buffer _dst_by_time_index_buffer;
  dst_by_time_value_buffer _dst_by_time_value_buffer;
  dst_by_time_index_allocate_manager _dst_by_time_index_allocate_manager;
  dst_by_time_value_allocate_manager _dst_by_time_value_allocate_manager;
  dst_by_time_index* _dst_by_time;
};

#define MAX_HITS ( 700 * 3600 * 24 * 30 )

#define MAX_THREAD 5




//#define MAX_HITS ( 1000000 )
int main(int argc, char* argv[])
{
  hitlist* h = new hitlist;
  h->restore("./hitlist");

  if ( h->size() == 0 )
  {
    fas::nanospan start = fas::nanotime();
    fas::nanospan current_start = fas::nanotime();
    size_t total = 0;
    size_t current = 0;
    time_t now = time(0);
    time_t start_time = now - 3600 * 24 * 30;
    for (int i = start_time ; i < now ; ++ i )
    {
      if ( i%100 == 0)
      {
        time_t day = ( i - start_time )/3600/24;
        time_t hour = ( i - start_time )/3600 - ( ( i - start_time )/3600/24 ) * 24;
      
        //std::cout << "day: " << day << ":" << hour << " total: " << total << " time "<< i - start_time << ", " << fas::nanotime() - start << ", rate: " << fas::rate(fas::nanotime() - current_start)*int(current) << std::endl;
        std::cout << total << "; " << fas::nanotime() - current_start << "; " << fas::rate(fas::nanotime() - current_start)*int(current)
                  << "; " << h->hits_main_index_size()
                  << "; " << h->dst_by_time_main_index_size()
                  << std::endl;
        current_start = fas::nanotime();
        current = 0;
      }
      // 700 persec
      for (int j = 0 ; j < 700; ++j )
      {
        if ( total >  MAX_HITS ) exit(0);
        
        hit hi;
        hi.src = rand() % 100000;
        hi.dst = rand() % 100000;
        hi.hittime = i;
        hi.type_id = 0;
        h->insert(hi);
        ++total;
        ++current;
      }
    }
  }

  
  /*
  typedef std::vector<int> vector_type;
  sorted_vector< vector_type, std::less<int>> sv;
  */
  return 0;
}
