#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
       
#include <functional>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <memory>
#include <iterator>

class mmap_base
{
public:
  mmap_base(): _addr(0), _size(0),_capacity(0)
  {
  }

  mmap_base(char *addr, size_t size): _addr(addr), _size(size),_capacity(size)
  {
  }

  mmap_base(char *addr, size_t size, size_t capacity): _addr(addr), _size(size),_capacity(capacity)
  {
  }

  size_t size() const { return _size; }
  
  size_t capacity() const { return _capacity; }
  
  char *addr() const { return _addr;}

  template<typename T>
  T* get(size_t offset) const
  {
    return reinterpret_cast<T*>(_addr + offset);
  }

  template<typename T>
  size_t offset(T* p)
  {
    if ( p==0 )
      return static_cast<size_t>(-1);
    return reinterpret_cast<char*>(p) - _addr;
  }
  
protected:
  char *_addr;
  size_t _size;
  size_t _capacity;
};

  template<typename T>
  struct offset_pointer
  {
    mmap_base *mmm;
    size_t offset;

    offset_pointer()
      : mmm(0)
      , offset(static_cast<size_t>(-1))
    { }

    offset_pointer(mmap_base* mmm, size_t offset = static_cast<size_t>(-1) )
      : mmm(mmm)
      , offset(offset)
    {}

    offset_pointer(mmap_base* mmm, T* ptr )
      : mmm(mmm)
      , offset(mmm->offset(ptr))
    {
    }

    T& operator*() { return *(mmm->get<T>(offset));}
    const T& operator*() const { return *(mmm->get<T>(offset));}

    T* operator->() { return mmm->get<T>(offset);}
    const T* operator->() const { return mmm->get<T>(offset);}

    void operator = ( size_t offset ) { this->offset = offset; }
    void operator = ( T* t ) { this->offset = mmm->offset(t); }
    operator size_t () const { return offset;}

    operator bool () const { return offset != static_cast<size_t>(-1);}
  };


  

class mmap_manager
  : public mmap_base

{
public:
  ~mmap_manager()
  {
    munmap( _addr, _size );
    close( _fd );
  }
  
  mmap_manager(): mmap_base(), _fd(-1)
  {
  }

  
  bool open(const char* path, size_t size = 0)
  {
    struct stat sb = { 0 };
    stat(path, &sb);
    /*if ( -1 == stat(path, &sb) )
      return false;**/
    
    _fd = ::open(path, O_RDWR | O_CREAT, (mode_t)0600);
    
    if ( _fd == -1)
      return -1;

    if ( size < sb.st_size)
      size = sb.st_size;
    
    if ( size > sb.st_size )
    {
      int lseek_result = lseek(_fd, size-1, SEEK_SET);
      write(_fd, "", 1);
    }
    
    _addr = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
    
    if (_addr == MAP_FAILED)
    {
      std::cout << "mmap failed" << std::endl;
      _addr = 0;
      close( _fd );
      _fd = -1;
      return false;
    }
    
    _size = size;
    msync(_addr, _size, MS_SYNC);
    return true;
  }
  
  void resize(size_t size)
  {
    if (size > _size)
    {
      int lseek_result = lseek(_fd, size-1, SEEK_SET);
      write(_fd, "", 1); 
    }
    else if (size < _size)
    {
      // TODO: урезать файл
    }
    _addr = (char*)mremap( _addr, _size, size, MREMAP_MAYMOVE);
    _size = size;
    if ( _addr == (void*)(-1) )
      throw;
  }

  /*
  size_t size() const { return _size; }
  
  char *addr() const { return _addr;}

  template<typename T>
  T* get(size_t offset) const
  {
    return reinterpret_cast<T*>(_addr + offset);
  }

  template<typename T>
  size_t offset(T* p)
  {
    if ( p==0 )
      return static_cast<size_t>(-1);
    return reinterpret_cast<char*>(p) - _addr;
  }
  */
  operator bool () const 
  {
    return _fd != -1;
  }
private:
  int _fd;
  /*
  char *_addr;
  size_t _size;
  */
};


template<typename T>
struct chunk
{
  size_t bits;
  T data[sizeof(size_t)*8];

  chunk(): bits(0) {}

  bool filled() const {
    return bits == static_cast<size_t>(-1);
  }
  bool empty() const  { return bits == 0; }
  
  size_t max_count() const { return sizeof(size_t)*8; }
  size_t size() const { return sizeof(this); /*+ sizeof(T)*max_count();*/ }

  void clear() { bits = 0; }
  
  size_t first_free()
  {
    /// биты c права на лево
    /// 000000000 000000000 000000000 000000000 000000000 000000000 000000000 000000001
    /// Первый элемент свободен
    for ( size_t i = 0; i < sizeof(size_t)*8; ++i )
      if ( ! ( bits & ( static_cast<size_t>(1) << i) ) )
        return i;
    return static_cast<size_t>(-1);
        
    /*
var
  x     : integer;
  index : word;
begin
  x := 44; //101100b
  asm
    bsf   ax, x
    mov   index, ax  //index = 2 (биты нумеруются с 0)
  end;
end;
*/
  }

  T* begin()
  {
    return data;
  }

  T* mark()
  {
    size_t index = first_free();
    if ( index == static_cast<size_t>(-1) )
      return 0;
    return mark(index);
  }

  T* mark(size_t index)
  {
    size_t oldbit = bits;
    bits |= ( static_cast<size_t>(1) <<  index );
    
    return begin() + index;
  }

  void free(T* addr)
  {
    size_t index = addr - begin();
    bits &= ~(1<<index);
  }
};

template<typename T>
struct chain
{
  typedef chunk<T> chunk_type;
  size_t size;
  size_t first_free;
  chain(): size(0), first_free(0) {}

  void acquire(size_t count) { size += count;} 
  chunk_type* begin()
  {
    return reinterpret_cast<chunk_type*>(this+1);
  }

  size_t chunk_size() const { return sizeof(chunk_type); }

  chunk_type* find_free()
  {
    
    chunk_type* beg = begin();
    chunk_type* end = beg + size;

    if ( beg!=end && !( beg + first_free)->filled() )
      return beg + first_free;

    
    for ( ;beg!=end; ++beg)
    {
      
      if ( !beg->filled() )
      {
        first_free = beg - begin();
        
        return beg;
      }
    }
    
    return 0;
  }

  T* mark()
  {
    if ( chunk_type* chk = find_free() )
      return chk->mark();
    return 0;
  }

  void free(T* value)
  {
    size_t offset = value - this->begin()->begin();
    offset -= offset%sizeof(chunk_type);
    chunk_type* chk = begin() + offset;
    chk->free(value);
    if ( offset < first_free )
      first_free = offset;
  }
};


template<typename T, typename M = mmap_manager>
class allocate_manager
{
public:
  typedef M memory_manager;
  typedef chunk<T> chunk_type;
  typedef chain<T> chain_type;
  typedef offset_pointer<T> pointer;
  typedef offset_pointer<const T> const_pointer;

  allocate_manager(memory_manager* mm)
    : _memory_manager(mm)
  {}

  bool acquire()
  {
    size_t offset = _memory_manager->size();
    _memory_manager->resize( offset + sizeof(chain_type) + sizeof(chunk_type) );
    char *addr = _memory_manager->addr();
    chain_type* ch = 0;
    if ( offset == 0 )
      ch = new (addr) chain_type;
    else
      ch =reinterpret_cast<chain_type*>(addr);
    ch->acquire(1);
    new (addr+offset)chunk_type;
    return true;
    /*char *addr = _memory_manager->addr() + offset;
    if ( offset == 0 )
    {
      chain_type* ch = new (addr)chain_type;
      addr+=sizeof(chain_type);
    }
    new (addr)chunk_type;*/
  }

  pointer allocate()
  {
    if ( _memory_manager->size() == 0 )
      this->acquire();

    chain_type* chn = (chain_type*)_memory_manager->addr();
    pointer p( _memory_manager );
    p = chn->mark();
    if (!p)
    {
      acquire();
      chn = (chain_type*)_memory_manager->addr();
      p = chn->mark();
      if (!p)
        std::cout << "realloc fail!!!" << std::endl;
    }
    return p;
    
    
    /*if ( _memory_manager->size() == 0 )
    {
      _memory_manager->resize( sizeof(chain_type) );
      chain_type* chn = (chain_type*)_memory_manager->addr();
      chn->size = 0;
      chn->first_free = 0;
    }*/
    
    /*chain_type* chn = (chain_type*)_memory_manager->addr();
    return pointer( _memory_manager, chn->mark() );*/
  }

  void deallocate(pointer ptr)
  {
    chain_type* chn = (chain_type*)_memory_manager->addr();
    chn->free(ptr);
  }
  
private:
  
  memory_manager* _memory_manager;
  
};

template<typename T, typename AllocateManager = allocate_manager<T> >
struct the_allocator
{
  typedef AllocateManager allocate_manager;
  typedef T value_type;
  typedef typename allocate_manager::pointer pointer;
  typedef typename allocate_manager::const_pointer const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef std::ptrdiff_t difference_type;

  enum { data_block_size = sizeof(T)*sizeof(size_t) };
  enum { block_size = sizeof(size_t) + data_block_size };
  
  //rebind
  template <typename U>
  struct rebind {
    typedef the_allocator<U> other;
  };

  pointer address (reference value ) const { return &value; }

  const_pointer address (const_reference value) const { return &value; }
  
  allocate_manager& _mmm;
  
  the_allocator(allocate_manager& mmm): _mmm(mmm) { }

  size_type max_size () const throw() { return ::std::numeric_limits <size_type>::max() / sizeof(T); }

  pointer allocate (size_type num, void *  hint = 0)
  {
    //if (num!=1) throw;
    return _mmm.allocate();
  }

  void construct (pointer p, const_reference value)
  {
      new((void *)p) T(value);  //placement new
  }

  void destroy (pointer p)
  {
    p->~T();
  }

  void deallocate (pointer p, size_type num)
  {
    //if (num!=1) throw;
    _mmm->deallocate(p);
  }
    
  
private:
 
};


template<typename T, size_t N>
class array
{
public:
  typedef T value_type;
  typedef T data_type[N];
  typedef size_t size_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef pointer iterator;
  typedef const_pointer const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const iterator> const_reverse_iterator;
  
  array():_size(0) {};
  
  reference operator[](size_type n) { return _data[n]; }
  const_reference operator[](size_type n) const { return _data[n]; }
  const_reference at ( size_type n ) const { return _data[n]; }
  reference at ( size_type n ) { return _data[n]; }
  reference front ( ){ return _data[0]; }
  const_reference front ( ) const{ return _data[0]; }
  reference back ( ){ return _data[_size-1]; }
  const_reference back ( ) const{ return _data[_size-1]; }

  size_type size() const { return _size;}
  size_type max_size() const { return N;}
  size_type capacity() const { return N;}
  bool empty () const {return _size==0;}
  bool filled () const { return _size == N;}
  void resize ( size_type sz, T value = value_type() ) 
  {
    if (sz > _size)
      std::fill_n( end(), sz - _size, value );
    _size = sz;
  }
  void reserve ( size_type n ) {}
  
  reverse_iterator rbegin() { return reverse_iterator(end()/*+_size-1*/); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(/*begin()+_size-1*/end()); }

  reverse_iterator rend() { return reverse_iterator(/*begin()-1*/begin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(/*begin()-1*/begin()); }

  iterator begin() { return _data;}
  const_iterator begin() const { return _data;}
  iterator end() { return _data + _size;}
  const_iterator end() const { return _data + _size;}
  iterator last() { return _data + _size - 1;}
  const_iterator last() const { return _data  + _size - 1;}
  
  
  template <class InputIterator>
  void assign ( InputIterator first, InputIterator last )
  {
    std::copy( first, last, _data );
    _size = std::distance(first, last);
  }
  
  void assign ( size_type n, const T& u )
  {
    std::fill_n( begin(), n, u );
    _size = n;
  }
  
  void push_back ( const T& x )
  {
    _data[_size++] = x;
  }
  
  void pop_back ( )
  {
    --_size;
  }
  
  iterator insert ( iterator position, const T& x )
  {
    std::copy_backward(position, end(), end()+1);
    *position = x;
    ++_size;
  }
  
  void insert ( iterator position, size_type n, const T& x )
  {
    std::copy_backward(position, end(), end()+n);
    std::fill_n(position, n, x);
    _size+=n;
  }
  
  template <class InputIterator>
  void insert ( iterator position, InputIterator first, InputIterator last )
  {
    std::copy_backward(position, end(), end()+std::distance(first,last) );
    std::copy(first, last, position);
    _size+=std::distance(first,last);
  }
  
private:
  size_type _size;
  data_type _data;
};

template<typename T, int N1, int N2, typename A1 = the_allocator< array<T,N1> >, typename A2 = the_allocator< array<size_t, N2> > >
class vector_list
{
public:
  typedef T value_type;
  typedef A1 value_allocator;
  typedef A2 index_allocator;

  typedef typename index_allocator::pointer index_pointer;
  typedef typename value_allocator::pointer value_pointer;
  
  
  typedef std::vector<index_pointer> index_list;
  typedef typename index_list::iterator index_list_iterator;
  
  vector_list(value_allocator a1, index_allocator a2 )
    : _value_allocator(a1)
    , _index_allocator(a2)
  {
    // value_allocator::pointer value_ptr = _value_allocator.allocate();
    
    
  }

  void push_back( const T& x )
  {
    std::cout << "push_back " << x.index  << std::endl;
    index_list_iterator last_index = _last_index();

    _value_pointer = *((*last_index)->last());

    if (  _value_pointer->filled() )
    {
      if ( (*last_index)->filled() )
      {
        std::cout << "create new chunk index" << std::endl;
        _index_pointer = _index_allocator.allocate(1);
        std::cout << "create new chunk data" << std::endl;
        _value_pointer = _value_allocator.allocate(1);
        _index_pointer->push_back( _value_pointer );
        _index_list.push_back( _index_pointer );
        last_index = _last_index();
      }
      else
      {
        std::cout << "create new chunk data" << std::endl;
        _value_pointer = _value_allocator.allocate(1);
        _value_allocator.construct(_value_pointer, typename value_allocator::value_type() );
        _index_pointer->push_back(_value_pointer);
      }
    }

    _value_pointer->push_back(x);
    
  }
  
protected:
  index_list_iterator _last_index()
  {
    if ( _index_list.empty() )
    {
      //std::cout << "create first chunk index" << std::endl;
      _index_pointer = _index_allocator.allocate(1);
      //std::cout << "create first chunk data" << std::endl;
      _value_pointer = _value_allocator.allocate(1);
      _index_pointer->push_back( _value_pointer );
      _index_list.push_back( _index_pointer );
    }
    return (++_index_list.rbegin()).base();
  }

private:
  index_list      _index_list;
  index_allocator _index_allocator;
  value_allocator _value_allocator;
  index_pointer _index_pointer;
  value_pointer _value_pointer;
};



template<typename T>
struct mmap_allocator
{
  typedef chunk<T> chunk_type;
  
  typedef T value_type;
  typedef T* pointer;
  typedef T& reference;
  typedef const T* const_pointer;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef std::ptrdiff_t difference_type;

  enum { data_block_size = sizeof(T)*sizeof(size_t) };
  enum { block_size = sizeof(size_t) + data_block_size };
  
  //rebind
  template <typename U>
  struct rebind {
    typedef mmap_allocator <U> other;
  };

  pointer address (reference value ) const { return &value; }

  const_pointer address (const_reference value) const { return &value; }
  
  mmap_manager& _mmm;
  
  mmap_allocator(mmap_manager& mmm): _mmm(mmm) { }

  size_type max_size () const throw() { return ::std::numeric_limits <size_type>::max() / sizeof(T); }

  void* _add_block()
  {
    size_t size = _mmm.size();
    _mmm.resize(size + block_size);
    // Обнуляем биты выделения
    *( reinterpret_cast<size_t *>( reinterpret_cast<const char *>(_mmm.addr() ) + size  ) ) = 0;
  }

  //pointer _allocate( char *block,  )

  pointer allocate (size_type num, void *  hint = 0)
  {
    size_t size = 0;
    /*
    if ( _mmm.size() < block_size + sizeof(size_t) )
    {
      _mmm.resize(block_size + sizeof(size_t));
    }
    const char *beg = reinterpret_cast<const char *>(_mmm.addr());
    size_t *bits = reinterpret_cast<size_t *>( beg );
    beg += sizeof(size_t);
    */
    
    /*
      pointer p = (pointer) ::mmap(hint, num, PROT_READ | PROT_WRITE, MAP_ANON, -1, 0);
      int * val = (int *)p;
      if(val && *val == -1)
        p = NULL;
      return p;
    */
  }

  void construct (pointer p, const_reference value)
  {
      new((void *)p) T(value);  //placement new
  }

  void destroy (pointer p)
  {
    p->~T();
  }

  void deallocate (pointer p, size_type num)
  {
      ::munmap((caddr_t) p, num);
  }
    
  /*
  size_t restore(T*** result) const
  {
    if ( _mmm.size() < sizeof(size_t) )
      return 0;
    char *addr = reinterpret_cast<char*>( _mmm.addr() );
    // Количество выделенных ненулквых элементов 
    size_t size = *(reinterpret_cast<size_t*>(addr));
    addr += sizeof(size);
    if (size == 0)
      return 0;
    // *result = new T*[size];

    size_t ipos = 0;
    for ( size_t i = 0; i < size; ++i)
    {
      // Маска на блок 64*sizeof(T)
      size_t bits = *(reinterpret_cast<size_t*>(addr));
      addr += sizeof(size);
      char *next = addr + sizeof(size)*sizeof(T);
      for (size_t b=0; b < sizeof(size_t); ++b )
      {
        if (bits & (1<<b) )
          result[ipos] = new (addr)T;
        addr+= sizeof(T);
        ++ipos;
      }
      addr+=next;
    }
}
    */
  
  
  operator bool () const { return _mmm; }
  
private:
 
};

template<size_t N, typename Allocator = std::allocator<size_t*> >
class big_bitset
{
public:
  typedef size_t size_type;
  enum { bitset_count = N/sizeof(size_type) + (N%sizeof(size_type)!=0) };
  
  typedef Allocator allocator_type;
  
  ~big_bitset()
  {
    if (_bitset)
      _allocator.deallocate(_bitset, bitset_count);
  }
  
  big_bitset()
  {
    _bitset = _allocator.allocate(bitset_count);
  }
  
  bool operator[] ( size_type i ) const { return (*_bitset)[i/sizeof(size_type)] & (1 << i%sizeof(size_type)); }
  
  void set(size_type i)
  {
    (*_bitset)[i/sizeof(size_type)] |= (1 << i%sizeof(size_type));
  }
  
  allocator_type get_allocator() const
  {
    return _allocator;
  }
  
private:
  size_type** _bitset;
  allocator_type _allocator;
};

template<typename ChainItr, typename CellItr>
struct pair_iterator_t
  : std::pair<ChainItr, CellItr>
{
  typedef std::pair<ChainItr, CellItr> super;
  pair_iterator_t(){}
  
  pair_iterator_t(std::pair<ChainItr, CellItr> p): super(p) {}
  
  pair_iterator_t(ChainItr chain_itr, CellItr cell_itr )
    : super(chain_itr, cell_itr)
  {}
  
  
};

template<typename K >
class sorted_vector
{
  typedef sorted_vector< K> self;
public:
  enum
  {
    max_size = 5,
    min_size = 3
  };
  
  typedef K value_type;
  typedef std::vector<K> cell_type;
  typedef std::vector<cell_type*> chain_type;
  typedef typename cell_type::iterator cell_iterator;
  typedef typename chain_type::iterator chain_iterator;
  typedef pair_iterator_t<chain_iterator, cell_iterator> pair_iterator;
  typedef typename cell_type::size_type size_type;
  
  /*typedef pair_iterator iterator;
  typedef sorted_vector_iterator<const self> const_iterator;
  */

  sorted_vector(): _size(0)
  {
    // TODO: при первой вставке 
    _chain.push_back(new cell_type);
  }

  template<typename Compare>
  void insert(const value_type& value, const Compare& compare )
  {
    pair_iterator pitr = _lower_bound(value, compare);
    (*pitr.first)->insert(pitr.second, value );
    ++_size;
    pitr = _smooth(pitr);
  }

  value_type& at(size_type index)
  {
    if ( index >= _size )
      throw std::out_of_range("sorted_vector::at");
    
    chain_iterator beg = _chain.begin();
    chain_iterator end = _chain.end();
    for ( ;index >= (*beg)->size() /*&& beg!=end*/; beg++)
      index-=(*beg)->size();
    return (*beg)->at(index);
  }

  value_type& operator[](size_type index)
  {
    return this->at(index);
  }

  size_type size() const { return _size;}
  
protected:

  pair_iterator _smooth(pair_iterator pair_itr)
  {
    if ( (*pair_itr.first)->size() > max_size )
      return _split(pair_itr);
    else if ( (*pair_itr.first)->size() < min_size )
      return _merge(pair_itr);
    return pair_itr;
  }

  pair_iterator _split(pair_iterator pair_itr)
  {
    cell_type* cell = *pair_itr.first;
    size_type cell_position = std::distance(_chain.begin(), pair_itr.first );

    std::cout << "cell_position1=" << cell_position << std::endl;
    size_t cell_size = cell->size();
    size_t left_size = cell_size/2 ;
    size_t right_size = cell_size/2 + cell_size%2;
    std::cout << "left_size=" << left_size << std::endl;
    std::cout << "right_size=" << right_size << std::endl;
    size_t offset = std::distance( cell->begin(), pair_itr.second);
    size_t item_position  = offset < left_size ? offset : offset - left_size;

    cell_type *new_cell = new cell_type(cell->begin() + left_size, cell->end());
    cell->resize(left_size);
    _chain.insert(pair_itr.first + 1, new_cell );

    if ( offset >= left_size )
      ++cell_position;

    for (int i = 0 ; i < cell->size(); ++i )
      std::cout << cell->at(i) << " ";

    std::cout << std::endl;
    for (int i = 0 ; i < new_cell->size(); ++i )
      std::cout << new_cell->at(i) << " ";
    std::cout << std::endl;

    std::cout << "cell_position2=" << cell_position << std::endl;
    std::cout << "_chain.size()=" << _chain.size() << std::endl;
    pair_itr.first = _chain.begin() + cell_position;
    pair_itr.second = (*pair_itr.first)->begin() + item_position;
    return pair_itr;
  }

  pair_iterator _merge(pair_iterator pair_itr)
  {
    return pair_itr;
  }

  bool _check(pair_iterator itr)
  {
    return itr.first!=_chain.end() && itr.second != (*itr.first)->end();
  }
  

  template<typename Compare>
  pair_iterator _lower_bound(const value_type& value, const Compare& compare)
  {
    chain_iterator chain_itr = _find_cell(value, compare);
    
    // Если не нашли то в последнее звено
    if (chain_itr == _chain.end() )
    {
      chain_itr = _chain.begin();
      std::advance(chain_itr, _chain.size() - 1);
    }
    else if (chain_itr != _chain.begin() )
      --chain_itr;
    
    cell_iterator cell_itr = _lower_bound_item(*chain_itr, value, compare);
    return pair_iterator(chain_itr, cell_itr);
  }

  template<typename Compare>
  cell_iterator _lower_bound_item(cell_type* cell, const value_type& value, const Compare& compare)
  {
    return  std::lower_bound( cell->begin(), cell->end(), value, compare);
  }

  template<typename Compare>
  chain_iterator _find_cell(const value_type& value, const Compare& compare)
  {
    if ( (*_chain.begin())->empty())
      return _chain.begin();
    chain_iterator itr = std::lower_bound(
      _chain.begin(),
      _chain.end(),
      value,
      [&compare]
      ( const cell_type* v, value_type value) -> bool
      {
        if (v->empty())
          throw std::domain_error("sorted_vector::_find_cell");
        return compare( (*v)[0] , value );
      }
    );
  }

  
private:

  // size always > 0
  chain_type _chain;
  size_type _size;
};

#include <array>

struct data
{
  data(): index() { std::fill_n(buffer, 8, 0xABABABABABABABAB); }
  int index;
  size_t buffer[8];
};
int main()
{
  mmap_manager mmm, mmm2;
  
  if ( !mmm.open("./data.bin", 1024) )
    std::cout << "fuck1" << std::endl;
  if (!mmm)
    std::cout << "fuck2" << std::endl;

  if ( !mmm2.open("./index.bin", 1024) )
    std::cout << "fuck1" << std::endl;
  if (!mmm2)
    std::cout << "fuck2" << std::endl;

  std::cout << "OK" << std::endl;

  /*allocate_manager<int> am1((&mmm));
  allocate_manager<int> am2((&mmm2));*/
  
  //the_allocator<int> alloca = the_allocator<int>(am1);

  typedef vector_list<data, 4096, 4096> vector_type;
  
  typedef vector_type::index_allocator index_allocator;
  typedef vector_type::value_allocator value_allocator;

  allocate_manager<value_allocator::value_type> am1((&mmm));
  allocate_manager<index_allocator::value_type> am2((&mmm2));

  vector_type vv(am1, am2);

  data d;
  for ( size_t i=0 ; i < 1024 *  1024 /** 1024*/; i++)
  {
    d.index = i;
    vv.push_back(d);
  }

  
  
  
  return 0;
  /*
  typedef array<int, 100> array_type;
  mmm.resize( sizeof(array_type) );
  array<int, 100> *aa = (array_type*)mmm.addr();
  aa->resize(100);
  for ( size_t i=0 ; i < 100; i++)
  {
    std::cout << (*aa)[i] <<" ";
    aa->insert( aa->end(), i);
    (*aa)[i]=i;
  }
  std::cout <<  "-" << std::endl;
  */
  
  //std::for_each( aa->rbegin(), aa->rend(), [](const int& i) { std::cout << i << " ";} );
  /*std::cout << "-" << aa->begin() << std::endl;
  std::cout << "-" << aa->end() << std::endl;
  std::cout << std::endl;*/
  return 0;

  offset_pointer<int> value(&mmm);

  

  
  typedef chunk<int> chunk_type;
  typedef chain<int> chain_type;
  /*std::cout << sizeof (size_t) << std::endl;
  std::cout << sizeof (chunk_type) << std::endl;
  */
  size_t fullsize = sizeof(chain_type) + 10 * ( sizeof(chain_type) + sizeof(int)*64 );
  
  //mmm.resize( fullsize );
  /*chain_type* chn = (chain_type*)mmm.addr();
  chn->size = 10;
  chn->first_free = 0;
  */
  allocate_manager<int, mmap_manager> am(&mmm);

  for (int i = 0 ; i < 19600; ++i)
  {
    std::cout << i << std::endl;
    /*int* pvalue = chn->mark();
    value = pvalue;
    */
    value = am.allocate();
    
    if ( value )
      *value = /*0x0A0B0C0D*/i;
    else
      std::cout << "NULL" << std::endl;
  }
    
  return 0;  
    
  mmap_allocator<int> mma(mmm);
  int **ii;
  //mma.restore(&ii);
  return 0;
  
  std::bitset<1024> myset;
  big_bitset<1024> mybigset;
  for (int i = 0; i < 1024; i++)
    if ( i%11 == 0) mybigset.set(i);
  for (int i = 0; i < 1024; i++)
  {
    if (i%64==0)
      std::cout << std::endl;
    std::cout << /*'0' + */mybigset[i] /*<< " "*/;
  }
  std::cout << std::endl;
  /*
  typedef std::greater<int> comp;
  sorted_vector<int> v;

  
  for (int i =0 ; i < 10 ; ++i)
    v.insert(i, comp());

  for (int i =0 ; i < v.size() ; ++i)
    std::cout << 0 + v[i] << " " ;
  std::cout << std::endl;
  
  for (int i = 15 ; i > 5 ; --i)
  {
    v.insert(i, comp());
    std::cout << " ---> " ;
    for (int i =0 ; i < v.size() ; ++i)
      std::cout << 0 + v[i] << " " ;
    std::cout << std::endl;

  }

  
  for (int i =0 ; i < 20 ; ++i)
    std::cout << 0 + v[i] << std::endl;
  return 0;
  */
}
