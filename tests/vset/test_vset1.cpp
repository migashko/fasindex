#include <pmi/vset.hpp>
#include <stdexcept>
#include <sstream>
#include <set>
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
  ss << file << ":" << pos << " " << text;
  throw std::logic_error(ss.str());
}

struct init_sequence
{
  int count;
  int first;
  int last;
  
  init_sequence(int count, int first, int last)
    : count(count)
    , first(first)
    , last(last)
  {}

  template<typename Container>
  void operator()(Container& cnt)
  {
    double step = double(last - first)/count;
    double curr = first;

    for (;count!=0; curr+= step, --count)
      cnt.insert( static_cast<typename Container::value_type>(curr) );
  }
};

struct init_random
{
  int count;
  int first;
  int last;

  init_random(int count, int first, int last)
    : count(count)
    , first(first)
    , last(last)
  {}

  template<typename Container>
  void operator()(Container& cnt)
  {
    int dist = last - first;
    for (;count!=0; --count)
      cnt.insert( first + std::rand()%dist );
  }
};


template<typename Container1, typename Container2>
bool equal( const Container1& cnt1, const Container2& cnt2)
{
  if (cnt1.size() != cnt2.size() )
    return false;

  typename Container1::const_iterator cur1 = cnt1.cbegin();
  auto cur2 = cnt2.cbegin();

  for (size_t i=0; i < cnt1.size(); ++i, ++cur1, ++cur2)
  {
    //std::cout << "equal" << *cur1 << std::endl;
    if ( *cur1 != *cur2 )
      return false;
  }
  return true;
}

class persist_container
{
public:
  typedef vset<int> set_type;

  typedef set_type::value_compare value_compare;
  typedef set_type::allocator_type allocator_type;
  typedef allocator_type::allocation_manager allocation_manager;
  typedef allocation_manager::buffer buffer_type;

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

class non_persist_container
{
public:
  typedef vset<int, std::less<int>, std::allocator< sorted_array<int, 1024, std::less<int> > > > set_type;

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
void test_insert1(const Container& cnt, F init, bool onlyCheck)
{
  std::stringstream ss;
  if ( onlyCheck ) test_stack.push("test_insert1 onlyCheck");
  else test_stack.push("test_insert1");
  std::multiset<int> values;
  init(values);
  if ( !onlyCheck )
    cnt->insert( values.begin(), values.end() );
  if (!equal(*cnt, values))
    raise("test_insert1", __FILE__, __LINE__);
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

  test_insert1( Container("test_insert-big.bin", true), init_sequence(10000000, 1, 10000000), false );
  if ( testPersist ) test_insert1(Container("test_insert-big.bin", false), init_sequence(10000000, 1, 10000000), true );

  test_stack.pop();
}


template<typename Container>
void test_all(bool testPersist)
{
  test_stack.push("test_all");
  test_insert<Container>(testPersist);
  test_stack.pop();
}

template<int BlockSize>
void test_persist()
{
  test_stack.push("test_persist");
  typedef persist_container container;
  test_all<container>(true);
  test_stack.pop();
  // test_all(persist_container("set.bin", true), !clear);
}

template<int BlockSize>
void test_non_persist()
{
  test_stack.push("test_non_persist");
  typedef non_persist_container container;
  test_all<container>(false);
  test_stack.pop();
}

void test_all_persist()
{
  std::cout << "------------------- test_all_persist -------------------" << std::endl;
  test_stack.push("test_all_persist");
  test_persist<16>();
  test_stack.pop();
}

void test_all_non_persist()
{
  std::cout << "------------------- test_all_non_persist -------------------" << std::endl;
  test_stack.push("test_all_non_persist");
  test_non_persist<16>();
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
  test_all_persist();
  test_all_non_persist();
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