#include <pmi/vset.hpp>
#include <stdexcept>
#include <sstream>
#include <set>
#include <list>
#include <stack>

std::stack<std::string> test_stack;

void raise(const std::string& text, const std::string& file = __FILE__, int pos = __LINE__  )
{
  while ( !test_stack.empty() )
  {
    std::cout << test_stack.top() << std::endl;
    test_stack.pop();
  }
  std::stringstream ss;
  ss << file << ":" << pos << " " << std::endl << text;
  throw std::logic_error(ss.str());
}

struct init_sequence
{
  int count;
  int first;
  int last;

  std::vector<int> vect;
  
  init_sequence(int count, int first, int last)
    : count(count)
    , first(first)
    , last(last)
  {
    double step = double(last - first)/count;
    double curr = first;

    for (;count!=0; curr+= step, --count)
      vect.push_back( static_cast<int>(curr) );

  }

  template<typename Container>
  void operator()(Container& cnt) const
  {
    std::copy(vect.begin(), vect.end(), std::back_inserter(cnt));
  }
};

struct init_random
{
  int count;
  int first;
  int last;

  std::vector<int> vect;

  init_random(int count, int first, int last)
    : count(count)
    , first(first)
    , last(last)
  {
    int dist = last - first;
    for (;count!=0; --count)
      vect.push_back( first + std::rand()%dist );

  }

  template<typename Container>
  void operator()(Container& cnt)  const
  {

    std::copy(vect.begin(), vect.end(), std::back_inserter(cnt));
  }
};


template<typename Container1, typename Container2>
bool equal( const Container1& cnt1, const Container2& cnt2)
{
  if (cnt1.size() != cnt2.size() )
    return false;

  typename Container1::const_iterator cur1 = cnt1.cbegin();
  auto cur2 = cnt2.cbegin();

  /*std::cout << "equal size " << cnt1.size() << std::endl;
  std::cout << "equal size " << cnt2.size() << std::endl;*/
  for (size_t i=0; i < cnt1.size(); ++i, ++cur1, ++cur2)
  {
    //std::cout << "equal " << *cur1 << "=="<< *cur2 << std::endl;
    if ( *cur1 != *cur2 )
      return false;
  }
  return true;
}

template<int ArraySize>
class persist_container
{
public:
  typedef typename vset_helper<int, std::less<int>, alloc_type::persistent, ArraySize >::vset_type set_type;
  //typedef vset<int> set_type;

  typedef typename set_type::value_compare value_compare;
  typedef typename set_type::allocator_type allocator_type;
  typedef typename allocator_type::allocation_manager allocation_manager;
  typedef typename allocation_manager::buffer buffer_type;

  persist_container(const std::string& filename, bool clear)
  {
    _buffer = new buffer_type;
    _buffer->open(filename.c_str(), 16);
    if (clear)
      _buffer->clear();
    _manager = new allocation_manager(*_buffer);
    _vset = new set_type( std::less<int>(), *_manager );
  }

  ~persist_container()
  {
    delete _vset;
    delete _manager;
    delete _buffer;
  }

  set_type& operator*() const
  {
    return *_vset;
  }

  set_type* operator->() const 
  {
    return _vset;
  }

private:
  buffer_type* _buffer;
  allocation_manager* _manager;
  set_type* _vset;
};

template<int ArraySize>
class non_persist_container
{
public:
  // typedef vset<int, std::less<int>, std::allocator< sorted_array<int, 1024, std::less<int> > > > set_type;
  
  typedef typename vset_helper<int, std::less<int>, alloc_type::inmemmory, ArraySize >::vset_type set_type;

  non_persist_container(const std::string& filename, bool clear)
  {
    _vset = new set_type;
  }

  ~non_persist_container()
  {
    delete _vset;
  }

  set_type& operator*() const 
  {
    return *_vset;
  }
  
  set_type* operator->() const
  {
    return _vset;
  }

private:
  set_type* _vset;
};


template<typename Container, typename F>
void test_insert1(const Container& cnt, const F& init, bool onlyCheck)
{
  std::stringstream ss;
  if ( onlyCheck ) test_stack.push("test_insert1 onlyCheck");
  else test_stack.push("test_insert1");
  std::vector<int> values;
  init(values);
  if ( !onlyCheck )
    cnt->insert( values.begin(), values.end() );
  if ( values.size() != cnt->size() )
  {
    std::cout << values.size() << std::endl;
    std::cout << cnt->size() << std::endl;
    raise("values1.size() != cnt->size()", __FILE__, __LINE__);
  }
  std::sort(values.begin(), values.end());
  if (!equal(*cnt, values))
  {
    auto itr = cnt->begin();
    for (size_t i = 0; i < values.size(); ++i, ++itr )
    {
      std::cout << "[" << values[i] << "=" << /**(cnt->begin() + i) << */"=" << *itr<< "],";
      if ( values[i]!= *itr/**(cnt->begin() + i)*/)
      {
        std::cout << "<<<";
        /*for (size_t j = 1; j < 10; ++j )
          std::cout << "[" << values[j+1] << "=" << *(cnt->begin() + j+1) << "],";
        i = values.size()-10;
        */
      }
    }
      
    std::cout << std::endl;
    
    //cnt->check();
    std::cout << "END CHECK" << std::endl;
    raise("test_insert1", __FILE__, __LINE__);
    
  }
  test_stack.pop();
}


template<typename Container>
void test_insert(bool testPersist)
{
  test_stack.push("test_insert");
  
  test_insert1( Container("test_insert.bin", true), init_sequence(1, 1, 1), false );
  if ( testPersist ) test_insert1(Container("test_insert.bin", false), init_sequence(1, 1, 1), true );

  test_insert1( Container("test_insert.bin", true), init_sequence(10, 1, 5), false );
  if ( testPersist ) test_insert1(Container("test_insert.bin", false), init_sequence(10, 1, 5), true );

  test_insert1( Container("test_insert.bin", true), init_sequence(256, 1, 5), false );
  if ( testPersist ) test_insert1(Container("test_insert.bin", false), init_sequence(256, 1, 5), true );

  test_insert1( Container("test_insert.bin", true), init_sequence(2048, 1, 500), false );
  if ( testPersist ) test_insert1(Container("test_insert.bin", false), init_sequence(2048, 1, 500), true );

  init_random rnd(100000, 1, 1000);
  std::cout << "------------1-----------" << std::endl;
  test_insert1( Container("test_insert1.bin", true), rnd, false );
  std::cout << "------------2-----------" << std::endl;
  if ( testPersist ) test_insert1(Container("test_insert1.bin", false), rnd, true );

  test_stack.pop();
}

template<typename Container, typename F>
void test_erase1(const Container& cnt, const F& init, bool onlyCheck)
{
  std::cout << "void test_erase1(const Container& cnt, const F& init, bool onlyCheck) " << init.count << std::endl;
  std::stringstream ss;
  if ( onlyCheck ) test_stack.push("test_insert1 onlyCheck");
  else test_stack.push("test_erase1");
  std::list<int> values1, values2;
  init(values1);

  if ( !onlyCheck )
    cnt->insert( values1.begin(), values1.end() );
  
  std::list<int>::iterator beg = values1.begin();
  std::list<int>::iterator end = values1.end();

  
  // Перемещаем каждый второй
  
  for ( ;beg!=end; ++beg)
  {
    values2.push_back(*beg);
    values1.erase(beg++);
  }
  
  
  if ( !onlyCheck )
  {
    for ( auto beg = values2.begin(); beg!=values2.end(); ++beg )
    {
      
      /*std::cout << *beg << " [";
      for (auto tmpi = cnt->begin(); tmpi != cnt->end(); ++tmpi )
        std::cout << *tmpi <<",";
      std::cout << "] -> " ;*/

      cnt->erase( *beg );
      
      /*std::cout << " [";
      for (auto tmpi = cnt->begin(); tmpi != cnt->end(); ++tmpi )
        std::cout << *tmpi <<",";
      std::cout << "]" << std::endl;
      */
      
    }
    // cnt->check();
  }

  if ( values1.size() != cnt->size() )
  {
    std::cout << values1.size() << std::endl;
    std::cout << values2.size() << std::endl;
    std::cout << cnt->size() << std::endl;
    raise("values1.size() != cnt->size()", __FILE__, __LINE__);
  }

  if (!equal(*cnt, values1))
  {
    auto itr = cnt->begin();
    beg = values1.begin();
    for (size_t i = 0; i < values1.size(); ++i, ++itr, ++beg )
    {
      std::cout << "[" << *beg << "=" << /**(cnt->begin() + i) << */"=" << *itr<< "],";
      if ( *beg != *itr /**(cnt->begin() + i)*/)
      {
        std::cout << "<<<";
        /*for (size_t j = 1; j < 10; ++j )
          std::cout << "[" << values[j+1] << "=" << *(cnt->begin() + j+1) << "],";
        i = values.size()-10;*/
      }
    }

    std::cout << std::endl;

    raise("test_erase1", __FILE__, __LINE__);
  }
  test_stack.pop();
}

template<typename Container>
void test_erase(bool testPersist)
{
  test_stack.push("test_erase");

  
  test_erase1( Container("test_erase.bin", true), init_sequence(1, 1, 1), false );
  if ( testPersist ) test_erase1(Container("test_erase.bin", false), init_sequence(1, 1, 1), true );
  

  test_erase1( Container("test_erase.bin", true), init_sequence(10, 1, 20), false );
  
  if ( testPersist ) test_erase1(Container("test_erase.bin", false), init_sequence(10, 1, 20), true );

  test_erase1( Container("test_erase.bin", true), init_sequence(256, 1, 512), false );
  if ( testPersist ) test_erase1(Container("test_erase.bin", false), init_sequence(256, 1, 512), true );

  test_erase1( Container("test_erase.bin", true), init_sequence(2048, 0, 2048), false );
  if ( testPersist ) test_erase1(Container("test_erase.bin", false), init_sequence(2048, 0, 2048), true );

  init_sequence init(10000, 0, 100000);
  test_erase1( Container("test_erase.bin", true), init, false );
  if ( testPersist ) test_erase1(Container("test_erase.bin", false), init, true );
  

  test_stack.pop();
}


template<typename Container>
void test_all(bool testPersist)
{
  test_stack.push("test_all");
  test_insert<Container>(testPersist);
  test_erase<Container>(testPersist);
  test_stack.pop();
}

template<int BlockSize>
void test_persist()
{
  std::cout << "------------------- test_persist ------------------- " << BlockSize << std::endl;
  test_stack.push("test_persist");
  typedef persist_container<BlockSize> container;
  test_all<container>(true);
  test_stack.pop();
  // test_all(persist_container("set.bin", true), !clear);
}

template<int BlockSize>
void test_non_persist()
{
  std::cout << "------------------- test_non_persist ------------------- " << BlockSize << std::endl;
  test_stack.push("test_non_persist");
  typedef non_persist_container<BlockSize> container;
  test_all<container>(false);
  test_stack.pop();
}

void test_all_persist()
{
  std::cout << "------------------- test_all_persist -------------------" << std::endl;
  test_stack.push("test_all_persist");
  
  test_persist<3>();
  test_persist<4>();
  test_persist<5>();
  test_persist<6>();
  test_persist<7>();
  test_persist<13>();
  test_persist<32>();
  test_persist<64>();
  test_persist<128>();
  test_persist<256>();
  test_persist<500>();
  test_persist<1000>();
  
  test_persist<1024>();
  test_persist<4000>();
  
  
  test_stack.pop();
}

void test_all_non_persist()
{
  std::cout << "------------------- test_all_non_persist -------------------" << std::endl;
  test_stack.push("test_all_non_persist");
  
  test_non_persist<3>();
  test_non_persist<4>();
  test_non_persist<5>();
  test_non_persist<6>();
  test_non_persist<7>();
  test_non_persist<13>();
  test_non_persist<32>();
  test_non_persist<64>();
  test_non_persist<128>();
  test_non_persist<256>();
  test_non_persist<500>();
  test_non_persist<1000>();
  test_non_persist<1024>();
  test_non_persist<4000>();
  
  test_stack.pop();
}



/*
  vset()
  explicit vset(const value_compare& comp,const allocator_type& alloc = allocator_type() )

  template<typename InputIterator>
  vset(InputIterator, InputIterator)

  template<typename InputIterator>
  vset(InputIterator, InputIterator, const value_compare& , const allocator_type&  = allocator_type() )

  vset(const vset& )

  vset(vset&& __x)

  vset( std::initializer_list<value_type>, const value_compare& = value_compare(), const allocator_type&  = allocator_type())

  vset&  operator=(const vset& __x)

  vset& operator=(vset&& __x)

  vset& operator=( std::initializer_list<value_type> )
  */




int main()
{
  test_all_non_persist();
  test_all_persist();
  
  /*
  typedef vset<int> set_type;
  typedef set_type::value_compare value_compare;
  typedef set_type::allocator_type allocator_type;
  typedef allocator_type::allocation_manager allocation_manager;
  typedef allocation_manager::buffer buffer_type;

  buffer_type buffer;
  buffer.open("set.bin", 16);
  buffer.clear();
  allocation_manager manager(buffer);

  /// explicit vset(const value_compare& comp,const allocator_type& alloc = allocator_type() )
  
  vset<int> *s = new vset<int>(value_compare(), allocator_type(manager)) ;
  if ( !s->check() )
    raise("check", __FILE__, __LINE__);
  delete s;

  /// template<typename InputIterator>
  /// vset(InputIterator, InputIterator, const value_compare& , const allocator_type&  = allocator_type() )
  std::vector<int> v;
  for (int i = 0; i < 10; ++i)
    v.push_back(i);
  s = new vset<int>( v.begin(), v.end(), value_compare(), allocator_type(manager) );
  if ( !s->check() )
    raise("check", __FILE__, __LINE__);
  int next = 0;
  std::for_each(v.begin(), v.end(), [&next](int v){
    if (v!=next)
      raise("v!=next", __FILE__, __LINE__);
    ++next;
  });

  /// vset( std::initializer_list<value_type>, const value_compare& = value_compare(), const allocator_type&  = allocator_type())
  *s = {3,4,5,6,7,8,9,10,11,12};
  if ( !s->check() )
    raise("check", __FILE__, __LINE__);
  next = 0;
  std::for_each(v.begin(), v.end(), [&next](int v){
    if (v!=next)
      raise("v!=next", __FILE__, __LINE__);
    ++next;
  });
  */

  

}