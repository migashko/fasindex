#ifndef SET_HPP
#define SET_HPP

#include <functional>
#include <memory>

#include <pmi/vector/set_item.hpp>
#include <pmi/allocator.hpp>

struct not_impl: std::exception {};

template<
  typename _Key,
  typename _Compare = std::less<_Key>
>
struct helper
{
  typedef sorted_array<_Key, 1024, _Compare> array_type;
  typedef array_tree< array_type > array_tree_type;
  typedef allocator< array_type, chain_memory<array_type, mmap_buffer> > allocator_type;
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

private:
  value_compare _comparator;
  allocator_type _allocator;
  typedef typename helper<_Key, _Compare>::array_type array_type;
  array_tree< array_type > _tree;
public:

  typedef typename allocator_type::pointer             array_pointer;
  typedef typename allocator_type::const_pointer       array_const_pointer;
  typedef typename allocator_type::reference           array_reference;
  typedef typename allocator_type::const_reference     array_const_reference;

  // Временное решение
  typedef _Key* iterator;
  typedef const _Key* const_iterator;
  typedef _Key* reverse_iterator;
  typedef const _Key* const_reverse_iterator;

  typedef typename allocator_type::size_type           size_type;
  typedef typename allocator_type::difference_type     difference_type;

  set()
    : _comparator()
    , _allocator()
    , _tree()
  {

  }

  explicit set(const value_compare& comp,const allocator_type& alloc = allocator_type() )
    : _comparator(comp)
    , _allocator(alloc)
    , _tree()
  {

  }

  template<typename InputIterator>
  set(InputIterator, InputIterator)
  {
    throw not_impl();
  }

  template<typename InputIterator>
  set(InputIterator, InputIterator, const value_compare& , const allocator_type&  = allocator_type() )
  {
    throw not_impl();
  }

  set(const set& )
  {
    throw not_impl();
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  set(set&& __x)
  {
    throw not_impl();
    // TODO: : _M_t(std::move(__x._M_t))
  }

  set( std::initializer_list<value_type>, const value_compare& = value_compare(), const allocator_type&  = allocator_type())
  {
    throw not_impl();
  }

#endif

  set&  operator=(const set& __x)
  {
    throw not_impl();
    return *this;
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  set& operator=(set&& __x)
  {
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

  key_compare key_comp() const
  {
    throw not_impl();
    return key_compare();
  }

  value_compare value_comp() const
  {
    throw not_impl();
    return value_compare();
  }

  allocator_type  get_allocator() const
  {
    throw not_impl();
    return allocator_type();
  }

  iterator  begin() const
  {
    throw not_impl();
    return iterator();
  }

  iterator end() const
  {
    throw not_impl();
    return iterator();
  }

  reverse_iterator rbegin() const
  {
    throw not_impl();
    return reverse_iterator();
  }

  reverse_iterator rend() const
  {
    throw not_impl();
    return reverse_iterator();
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  iterator  cbegin() const
  {
    throw not_impl();
    return iterator();
  }

  iterator cend() const
  {
    throw not_impl();
    return iterator();
  }

  reverse_iterator crbegin() const
  {
    throw not_impl();
    return reverse_iterator();
  }

  reverse_iterator crend() const
  {
    throw not_impl();
    return reverse_iterator();
  }

#endif

  bool empty() const
  {
    throw not_impl();
    return bool();
  }

  size_type size() const
  {
    throw not_impl();
    return size_type();
  }

  size_type max_size() const
  {
    throw not_impl();
    return size_type();
  }

  void swap(set& )
  {
    throw not_impl();
  }

  std::pair<iterator, bool> insert(const value_type& value)
  {
    /// TODO: сделать копию первого элемента массива в ОЗУ!!
    auto treeitr = _tree.lower_bound(value);
    if ( !_tree.empty() && treeitr == _tree.end()  )
    {

      treeitr = --_tree.end();
      /// ??? здесь косяк (пьян)
      if ( (*treeitr)->filled() && _comparator( (*treeitr)->back(), value) )
        treeitr = _tree.end();

      /*
      std::cout << "set::insert " << "??" << std::endl;
      treeitr = --_tree.end();
      if ( _comparator( (*treeitr)->back(), value) )
        treeitr = _tree.end();
      else if ( (*treeitr)->filled() )
      {
        // TODO: split
        throw std::domain_error("split");
      }
      */

    }

    if ( treeitr == _tree.end() )
    {
      std::cout << "new array" << std::endl;
      array_pointer arr = _allocator.allocate(1);
      arr->insert(value);
      // TODO: сделать в offset pointer operator type* &&
      treeitr = _tree.insert( arr.operator -> () ).first;
    }
    else
    {
      if ( (**treeitr)[0]!=value )
      {
        (*treeitr)->insert(value);
      }
      else
      {
        array_type *arr = *treeitr;
        _tree.erase(treeitr--);
        arr->insert(value);
        // TODO: сделать проверку
        treeitr = _tree.insert(treeitr, arr);
      }
    }

    // TODO: это multiset
    //throw not_impl();
    // TODO: сделать!! итераторы
    return std::pair<iterator, bool>(iterator(), true);
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  std::pair<iterator, bool> insert(value_type&& )
  {
    throw not_impl();
    /*
    std::pair<typename _Rep_type::iterator, bool> __p = _M_t._M_insert_unique(std::move(__x));
    return std::pair<iterator, bool>(__p.first, __p.second);
    */
    return std::pair<iterator, bool>();
  }
#endif

  iterator  insert(const_iterator , const value_type&)
  {
    throw not_impl();
    return iterator();
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  iterator insert(const_iterator , value_type&& )
  {
    throw not_impl();
    return iterator();
  }
#endif

  template<typename InputIterator>
  void insert(InputIterator, InputIterator)
  {
    throw not_impl();
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  void insert( std::initializer_list<value_type> lst)
  {
    throw not_impl();
    this->insert(lst.begin(), lst.end());
  }

#endif

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  iterator erase(const_iterator )
  {
    throw not_impl();
    return iterator();
  }

#else

  void  erase(iterator)
  {
    throw not_impl();
  }

#endif

  size_type erase(const key_type& )
  {
    throw not_impl();
    return size_type();
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  iterator erase(const_iterator , const_iterator )
  {
    throw not_impl();
    return iterator();
  }

#else


  void erase(iterator , iterator)
  {
    throw not_impl();
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
    return iterator();
  }

  const_iterator find(const key_type& ) const
  {
    throw not_impl();
    return const_iterator();
  }

  iterator lower_bound(const key_type& )
  {
    throw not_impl();
    return iterator();
  }

  const_iterator lower_bound(const key_type& ) const
  {
    throw not_impl();
    return const_iterator();
  }

  iterator upper_bound(const key_type& )
  {
    throw not_impl();
    return iterator();
  }

  const_iterator upper_bound(const key_type& ) const
  {
    throw not_impl();
    return const_iterator();
  }

  std::pair<iterator, iterator> equal_range(const key_type&)
  {
    throw not_impl();
    return std::pair<iterator, iterator>();
  }

  std::pair<const_iterator, const_iterator> equal_range(const key_type& __x) const
  {
    throw not_impl();
    return std::pair<const_iterator, const_iterator>();
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
