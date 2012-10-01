#ifndef SET_HPP
#define SET_HPP

#include <functional>
#include <memory>
#include <map>

//#include <pmi/vector/set_item.hpp>
#include <pmi/allocator.hpp>
#include <pmi/map_iterator.hpp>
#include <pmi/vector/sorted_array.hpp>

struct not_impl: std::exception {};

template<
  typename _Key,
  typename _Compare = std::less<_Key>
>
struct helper
{
  typedef sorted_array<_Key, 1024, _Compare> array_type;
  //typedef array_tree< array_type, _Key, _Compare > array_tree_type;
  typedef allocator< array_type, chain_memory<array_type, mmap_buffer> > allocator_type;
  typedef std::multimap< _Key,  typename allocator_type::pointer, _Compare > array_tree_type;
};


template<
  typename _Key,
  typename _Compare = std::less<_Key>,
  typename _Alloc = typename helper<_Key, _Compare>::allocator_type
  //typename _Alloc = std::allocator< sorted_array<_Key, 1024, _Compare> >
>
class set
{
  /*
  typedef sorted_array<_Key, 1024, _Compare> array_type;
  typedef array_tree< array_type > array_tree_type;
  typedef allocator< chain_memory<array_type>, mmap_buffer > allocator_type;
  */

public:
  typedef _Key     key_type;
  typedef _Key     value_type;
  typedef _Compare key_compare;
  typedef _Compare value_compare;
  typedef _Alloc   allocator_type;

public:

  typedef typename allocator_type::pointer             array_pointer;
  typedef typename allocator_type::const_pointer       array_const_pointer;
  typedef typename allocator_type::reference           array_reference;
  typedef typename allocator_type::const_reference     array_const_reference;


  typedef typename allocator_type::size_type           size_type;
  typedef typename allocator_type::difference_type     difference_type;

private:
  value_compare _comparator;
  allocator_type _allocator;
  typedef helper<_Key, _Compare> help;
  typedef typename help::array_tree_type array_tree_type;
  array_tree_type _tree;
  size_t _size;
public:
  typedef typename help::array_type array_type;

  typedef typename array_type::iterator array_iterator;
  typedef typename array_type::const_iterator array_const_iterator;
  
  typedef typename array_tree_type::iterator tree_iteartor;
  typedef typename array_tree_type::const_iterator const_tree_iteartor;

    // Временное решение
  typedef map_iterator<tree_iteartor> iterator;
  typedef const map_iterator<tree_iteartor> const_iterator;
  //typedef map_iterator<const_tree_iteartor> const_iterator;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef const std::reverse_iterator<iterator> const_reverse_iterator;

  set()
    : _comparator()
    , _allocator()
    , _tree()
    , _size(0)
  {

  }

  explicit set(const value_compare& comp,const allocator_type& alloc = allocator_type() )
    : _comparator(comp)
    , _allocator(alloc)
    , _tree()
    , _size(0)
  {

  }

  template<typename InputIterator>
  set(InputIterator, InputIterator)
  {
    _size = 0;
    throw not_impl();
  }

  template<typename InputIterator>
  set(InputIterator, InputIterator, const value_compare& , const allocator_type&  = allocator_type() )
  {
    _size = 0;
    throw not_impl();
  }

  set(const set& )
  {
    _size = 0;
    throw not_impl();
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  set(set&& __x)
  {
    _size = 0;
    throw not_impl();
    // TODO: : _M_t(std::move(__x._M_t))
  }

  set( std::initializer_list<value_type>, const value_compare& = value_compare(), const allocator_type&  = allocator_type())
  {
    _size = 0;
    throw not_impl();
  }

#endif

  set&  operator=(const set& __x)
  {
    _size = 0;
    throw not_impl();
    return *this;
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  set& operator=(set&& __x)
  {
    _size = 0;
    throw not_impl();
  // NB: DR 1204.
  // NB: DR 675.
//  this->clear();
//  this->swap(__x);
    return *this;
  }

  set& operator=( std::initializer_list<value_type> )
  {
    throw not_impl();
    /*this->clear();
    this->insert(__l.begin(), __l.end());
    */
    return *this;
  }
#endif

  // accessors:

  const key_compare& key_comp() const
  {
    return _comparator;
  }

  const value_compare& value_comp() const
  {
    return _comparator;
  }

  const allocator_type& get_allocator() const
  {
    return _allocator;
  }

  iterator  begin()
  {
    return iterator(_tree.begin(), 0);
  }

  iterator end()
  {
    return iterator(_tree.end(), 0 );
  }

  const_iterator  begin() const
  {
    return const_iterator(_tree.begin(), 0);
  }

  const_iterator end() const
  {
    return const_iterator( _tree.end(), 0 );
  }

  reverse_iterator rbegin()
  {
    return reverse_iterator( this->end() );
  }

  reverse_iterator rend()
  {
    return reverse_iterator( this->begin() );
  }

  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator( this->end() );
  }

  const_reverse_iterator rend() const
  {
    return const_reverse_iterator( this->begin() );
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  const_iterator  cbegin() const
  {
    return const_iterator(_tree.begin(), 0);
  }

  const_iterator cend() const
  {
    return const_iterator(_tree.end(), 0 );
  }

  const_reverse_iterator crbegin() const
  {
    return const_reverse_iterator( this->end() );
  }

  reverse_iterator crend() const
  {
    return const_reverse_iterator( this->begin() );
  }

#endif

  bool empty() const
  {
    return _size==0;
  }

  size_type size() const
  {
    return _size;
  }

  size_type max_size() const
  {
    return _tree.max_size();
  }

  void swap( set& s )
  {
    std::swap(s._comparator, _comparator);
    std::swap(s._allocator, _allocator);
    std::swap(s._tree, _tree);
    std::swap(s._size, _size);
  }

  void check()
  {
    int current = 0;
    std::cout << "================= check =====================" << std::endl;
    std::for_each( _tree.begin(), _tree.end(), [&](  const typename decltype(_tree)::value_type& v)
    {
      std::cout << v.first  << "!" << std::endl;
      std::for_each( v.second->begin(), v.second->end(), [&](int vv)
      {
        std::cout << vv  << " ";
        std::cout.flush();
        if (current > vv)
          abort();
        current = vv;
      });
      std::cout.flush();
      std::cout << "<->" << std::endl;;
    });
    std::cout << std::endl;
    std::cout << "================= end check =====================" << std::endl;
  }

  iterator insert(const value_type& value)
  {
    return _insert(value);
  }
  template<typename VT>
  iterator _insert(VT& value)
  {
    //1. Находим последний элемент куда будем вставлять
    auto treeitr = _tree.end();
    array_iterator aitr;
    
    if ( !_tree.empty() )
    {
      treeitr = _tree.upper_bound(value);
      if ( treeitr == _tree.end() )
      {
        if ( value <= _tree.begin()->first )
          treeitr = _tree.begin();
        else
          treeitr = (++_tree.rbegin()).base();
      }
      else if ( _comparator(value, treeitr->first) )
      {
        if ( treeitr != _tree.begin() )
          --treeitr;
      }
    }

    if ( treeitr == _tree.end() )
    {
      // Новый массив
      array_pointer arr = _allocator.allocate(1);
      aitr = arr->insert(value);
      treeitr = _tree.insert( std::make_pair( value, arr) );
    }
    else
    {
      if ( treeitr->second->filled() )
      {
        // Расщипляем массив
        array_pointer arr1 = treeitr->second;
        array_pointer arr2 = _allocator.allocate(1);
        arr2->assign( arr1->begin() + arr1->size()/2, arr1->end() );
        arr1->resize( arr1->size()/2 );

        _tree.insert( treeitr, std::make_pair((*arr2)[0], arr2) );
        // TDOD: Избавиться от рекурсии
        return insert(value);
      }
      else
      {
        if ( _comparator(value, treeitr->first) )
        {
          // Вставляем элемент меньше первого в массиве
          array_pointer arr = treeitr->second;
          _tree.erase(treeitr);
          aitr = arr->insert(value);
          treeitr = _tree.insert( std::make_pair( value, arr) );
        }
        else
        {
          // Для всех остальных (больше первого )
          aitr = treeitr->second->insert(value);
        }
      }
    }

    //check();

    //if ( !flag ) abort();
    ++_size;
    return iterator( treeitr, std::distance(treeitr->second->begin(), aitr) );
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  iterator insert(value_type&& value)
  {
    return this->_insert(value);
    //throw not_impl();
    /*
    std::pair<typename _Rep_type::iterator, bool> __p = _M_t._M_insert_unique(std::move(__x));
    return std::pair<iterator, bool>(__p.first, __p.second);
    */
    //return std::pair<iterator, bool>();
  }
#endif

  iterator  insert(const_iterator, const value_type& value)
  {
    // TDOD: оптимизировать
    return this->_insert(value);
    /*throw not_impl();
    return iterator();
    */
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  iterator insert(const_iterator, value_type&& value)
  {
    // TDOD: оптимизировать
    // return insert( std::move(value) );
    return this->_insert(value);
  }
#endif

  template<typename InputIterator>
  void insert(InputIterator beg, InputIterator end)
  {
    std::for_each(beg, end, [this](const value_type& value){ this->insert(value);} );
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  void insert( std::initializer_list<value_type> lst)
  {
    this->insert( lst.begin(), lst.end() );
  }

#endif

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  /*iterator*/void erase(const_iterator itr)
  {
    auto arrayptr = itr.get_tree_iteartor()->second;
    arrayptr->erase( arrayptr->begin() + itr.get_position());
    //(*(itr.get_tree_iteartor()->second)).erase(10)/*(*itr)*/;
    //throw not_impl();
    /*(*itr)->second.erase( itr.get_position() );
    if ( itr->empty() )
      _tree->erase(itr);
    */
    // throw not_impl();
    //return iterator();
  }

#else

  void  erase(iterator)
  {
    auto arrayptr = itr.get_tree_iteartor()->second;
    arrayptr->erase( arrayptr->begin() + itr.get_position());
  }

#endif

  size_type erase(const key_type& )
  {
    throw not_impl();
    return size_type();
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  /*iterator*/void erase(const_iterator first, const_iterator last)
  {
    for (;first!=last; ++first)
      this->erase(first);
    // throw not_impl();
   // return iterator();
  }

#else


  void erase(iterator first, iterator first)
  {
    for (;first!=last; ++first)
      this->erase(first);
  }

#endif

  void clear()
  {
    throw not_impl();
  }

  size_type count(const key_type& ) const
  {
    throw not_impl();
    return size_type();
  }

  iterator find(const key_type&)
  {
    throw not_impl();
    // return iterator();
  }

  const_iterator find(const key_type& ) const
  {
    throw not_impl();
    // return const_iterator();
  }

  iterator lower_bound(const key_type& key)
  {
    auto treeitr = _tree.lower_bound(key);
    //std::cout << "lower_bound 1 key=" << key << " first=" << (_tree.begin() + 1)->first << std::endl;
    if ( treeitr == _tree.end() )
      treeitr = _tree.begin();
    std::cout << "lower_bound 1 key=" << key << " first=" << treeitr->first << std::endl;
      //return this->end();
    std::cout << "lower_bound 2 " << treeitr->second->empty() << std::endl;
    auto arrayitr = std::lower_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );
    std::cout << "lower_bound 2.1"<< std::endl;
    if ( arrayitr == treeitr->second->end() )
    {
      std::cout << "lower_bound fail"<< std::endl;
      for (auto itr = treeitr->second->begin(); itr!=treeitr->second->end(); ++itr )
      {
        std::cout << ":::" << *itr << std::endl;
      }
      return this->end();
    }
    std::cout << "lower_bound 3"<< std::endl;
    std::cout << "lower_bound: " << std::distance(treeitr->second->begin(), arrayitr) << std::endl;
    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr));
  }

  const_iterator lower_bound(const key_type& key) const
  {
    auto treeitr = _tree.lower_bound(key);
    if ( treeitr == _tree.end() )
      return this->end();
    auto arrayitr = std::lower_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );
    if ( arrayitr == treeitr->second->end() )
      return this->end();
    std::cout << "lower_bound: " << std::distance(treeitr->second->begin(), arrayitr) << std::endl;
    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr));
  }

  iterator upper_bound(const key_type& key)
  {
    if ( _tree.empty() )
      return this->end();
    
    auto treeitr = _tree.upper_bound(key);
    
    if ( treeitr != _tree.begin() )
      --treeitr;
    
    auto arrayitr = std::upper_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );
    
    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr) );
  }

  const_iterator upper_bound(const key_type& key) const
  {
    if ( _tree.empty() )
      return this->end();

    auto treeitr = _tree.upper_bound(key);

    if ( treeitr == _tree.begin() )
      --treeitr;

    auto arrayitr = std::upper_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );

    return const_iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr) );
  }

  std::pair<iterator, iterator> equal_range(const key_type& x)
  {
    return std::make_pair(
      this->lower_bound(x),
      this->upper_bound(x)
    );
  }

  std::pair<const_iterator, const_iterator> equal_range(const key_type& x) const
  {
    return std::make_pair(
      this->lower_bound(x),
      this->upper_bound(x)
    );
  }


  template<typename _K1, typename _C1, typename _A1>
  friend bool operator==(const set<_K1, _C1, _A1>&, const set<_K1, _C1, _A1>&);

  template<typename _K1, typename _C1, typename _A1>
  friend bool operator<(const set<_K1, _C1, _A1>&, const set<_K1, _C1, _A1>&);

};

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator==(const set<_Key, _Compare, _Alloc>& __x, const set<_Key, _Compare, _Alloc>& __y)
{
  throw not_impl();
  // return __x._M_t == __y._M_t;
  return bool();
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator< (const set<_Key, _Compare, _Alloc>& __x, const set<_Key, _Compare, _Alloc>& __y)
{
  throw not_impl();
  // return __x._M_t < __y._M_t;
  return bool();
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool
operator!=(const set<_Key, _Compare, _Alloc>& __x, const set<_Key, _Compare, _Alloc>& __y)
{
  return !(__x == __y);
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator>(const set<_Key, _Compare, _Alloc>& __x, const set<_Key, _Compare, _Alloc>& __y)
{
  return __y < __x;
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator<=(const set<_Key, _Compare, _Alloc>& __x, const set<_Key, _Compare, _Alloc>& __y)
{
  return !(__y < __x);
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator>=(const set<_Key, _Compare, _Alloc>& __x, const set<_Key, _Compare, _Alloc>& __y)
{
  return !(__x < __y);
}

template<typename _Key, typename _Compare, typename _Alloc>
inline void swap(set<_Key, _Compare, _Alloc>& __x, set<_Key, _Compare, _Alloc>& __y)
{
  __x.swap(__y);
}

#endif
