#ifndef SET_HPP
#define SET_HPP

#include <functional>
#include <memory>
#include <map>

//#include <pmi/vector/set_item.hpp>
#include <pmi/allocator.hpp>
#include <pmi/vset_iterator.hpp>
#include <pmi/sorted_array.hpp>

template<typename S, typename VT>
inline void restore(S& s, std::allocator<VT>)
{
  //std::cout << "inline void restore(S& s, std::allocator<VT>)" << std::endl;
}

template<typename S, typename VT, typename CM>
inline void restore(S& s, allocator<VT, CM> a)
{
  //std::cout << "inline void restore(S& s, allocator<VT, CM> a)" << std::endl;
  auto beg = a->begin();
  auto end = a->end();
  for (;beg!=end;++beg)
  {
    //std::cout << "restore: size="<< beg->size() << " [0]=" << beg->at(0) << std::endl;
    s._tree.insert( std::make_pair( std::make_pair(beg->front(), beg->back() ), beg) );
    s._size += beg->size();
  }
    
}

struct not_impl: std::exception {};

template<
  typename _Key,
  typename _Compare,
  typename _Alloc
>
class vset;

struct alloc_type
{
  typedef enum {
    inmemmory,
    persistent
  } type;
};

template<
  typename _Key,
  typename _Compare = std::less<_Key>,
  alloc_type::type AllocType = alloc_type::inmemmory,
  size_t ArraySize = 1024
>
struct vset_helper;

template<
  typename _Key,
  typename _Compare,
  size_t ArraySize
>
struct vset_helper<_Key, _Compare, alloc_type::persistent, ArraySize >
{
  typedef _Compare comparator;
  typedef sorted_array<_Key, ArraySize, _Compare> array_type;
  typedef mmap_buffer buffer_type;
  typedef chain_memory<array_type, buffer_type> memory_manager;
  typedef allocator< array_type, memory_manager > allocator_type;
  typedef vset<_Key, _Compare, allocator_type> vset_type;
};


template<typename _Compare>
struct tree_comparator
{
  typedef _Compare comparator;

  tree_comparator(const comparator& cmp): _comparator(cmp) {}

  template<typename V>
  bool operator()(const V& a, const V& b)
  {
    if ( _comparator( a.first, b.first ) )
      return true;

    if ( _comparator( b.first, a.first ) )
      return false;

    if ( _comparator( a.second, b.second ) )
      return true;

    return false;
  }
  
  const comparator& _comparator;
};

template<
  typename _Key,
  typename _Compare,
  size_t ArraySize
>
struct vset_helper<_Key, _Compare, alloc_type::inmemmory, ArraySize >
{
  typedef _Compare comparator;
  // typedef vset<int, std::less<int>, std::allocator< sorted_array<int, 1024, std::less<int> > > > set_type;
  typedef sorted_array<_Key, ArraySize, _Compare> array_type;
  typedef std::allocator< array_type > allocator_type;
  typedef vset<_Key, _Compare, allocator_type> vset_type;
};

/*
template<
  typename _Key,
  typename _Compare = std::less<_Key>
>
struct helper
{
  typedef sorted_array<_Key, 1024, _Compare> array_type;
  typedef allocator< array_type, chain_memory<array_type, mmap_buffer> > allocator_type;
  typedef std::multimap< _Key,  typename allocator_type::pointer, _Compare > array_tree_type;
};
*/


template<
  typename _Key,
  typename _Compare = std::less<_Key>,
  typename _Alloc = typename vset_helper<_Key, _Compare>::allocator_type
>
class vset
{
  typedef vset<_Key, _Compare, _Alloc > self;
  friend void restore<>(self&, _Alloc);
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
  //typedef helper<_Key, _Compare> help;
  // typedef typename help::array_tree_type array_tree_type;
  typedef std::pair<_Key, _Key> tree_key_type;
  typedef tree_comparator<value_compare> tree_key_compare;
  typedef std::multimap< tree_key_type,  typename allocator_type::pointer, tree_key_compare > array_tree_type;
  array_tree_type _tree;
  size_t _size;
  
public:
  
  // typedef typename help::array_type array_type;
  typedef typename allocator_type::value_type array_type;

  typedef typename array_type::iterator array_iterator;
  typedef typename array_type::const_iterator array_const_iterator;

  typedef typename array_tree_type::iterator tree_iteartor;
  typedef typename array_tree_type::const_iterator const_tree_iteartor;

    // Временное решение
  typedef vset_iterator<tree_iteartor, value_type> iterator;
  //typedef vset_iterator<tree_iteartor> const_iterator;
  //typedef vset_iterator<const tree_iteartor> const_iterator;
  typedef vset_iterator<const_tree_iteartor, const value_type> const_iterator;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  //typedef std::reverse_iterator<const iterator> const_reverse_iterator;

  vset()
    : _comparator()
    , _allocator()
    , _tree( tree_key_compare(_comparator) )
    , _size(0)
  {}

  explicit vset(const value_compare& comp,const allocator_type& alloc = allocator_type() )
    : _comparator(comp)
    , _allocator(alloc)
    , _tree( tree_key_compare(_comparator) )
    , _size(0)
  {
    restore(*this, _allocator);
  }

  template<typename InputIterator>
  vset(InputIterator beg, InputIterator end)
    : _comparator()
    , _allocator()
    , _tree( tree_key_compare(_comparator) )
    , _size(0)
  {
    this->insert(beg, end);
  }

  template<typename InputIterator>
  vset(InputIterator beg, InputIterator end, const value_compare& comp, const allocator_type&  alloc= allocator_type() )
    : _comparator(comp)
    , _allocator(alloc)
    , _tree( tree_key_compare(_comparator) )
    , _size(0)
  {
    restore(_allocator);
    this->insert(beg, end);
  }

  vset(const vset& )
  {
    // Запретить копирование если работаем с файлом
    _size = 0;
    throw not_impl();
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  vset(vset&& __x)
  {
    _size = 0;
    throw not_impl();
    // TODO: : _M_t(std::move(__x._M_t))
  }

  vset( std::initializer_list<value_type> il, const value_compare& comp= value_compare(), const allocator_type&  alloc= allocator_type())
    : _comparator(comp)
    , _allocator(alloc)
    , _tree()
    , _size(0)
  {
    this->insert( il.begin(), il.end() );
  }

#endif

  vset&  operator=(const vset& __x)
  {
    _size = 0;
    throw not_impl();
    return *this;
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  vset& operator=(vset&& __x)
  {
    _size = 0;
    throw not_impl();
  // NB: DR 1204.
  // NB: DR 675.
  //  this->clear();
  //  this->swap(__x);
    return *this;
  }

  vset& operator=( std::initializer_list<value_type> il)
  {
    
    this->clear();
    this->insert( il.begin(), il.end() );
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
    return const_iterator(_tree.cbegin(), 0);
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
    /*if (std::distance(this->cbegin(), this->cend())!=(difference_type)_size )
      throw std::logic_error("size_type size() const");*/
    return _size;
  }

  size_type max_size() const
  {
    return _tree.max_size();
  }

  void swap( vset& s )
  {
    std::swap(s._comparator, _comparator);
    std::swap(s._allocator, _allocator);
    std::swap(s._tree, _tree);
    std::swap(s._size, _size);
  }


  size_t capacity() const
  {
    return _tree.size() * array_type::dimension;
  }

  bool check()
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
    return true;
  }

  iterator insert(const value_type& value)
  {
    return _insert(value);
  }

  tree_iteartor _lower_node(const value_type& value)
  {
    tree_iteartor itr = _tree.lower_bound( std::make_pair(value, value_type() ) );
    if ( itr==_tree.end() && !_tree.empty() )
      itr = (++_tree.rbegin()).base();
    
    if ( itr!=_tree.end() && itr!=_tree.begin() && _comparator(value, itr->first.first) )
      --itr;
    return  itr;
  }

  tree_iteartor _upper_node(const value_type& value)
  {
     
    tree_iteartor itr = _tree.lower_bound( std::make_pair(value, value_type() ) );
    for ( ; itr!=_tree.end() && !_comparator( value, itr->first.first ); ++itr );
    if ( itr!=_tree.end() )
    {
      if ( itr!=_tree.begin() &&  _comparator( value, itr->first.first ) )
        --itr;
    }
    else
    {
      if ( !_tree.empty() )
        itr = (++_tree.rbegin()).base();
    }
    return  itr;
  }



  template<typename TreeIterator>
  TreeIterator _re_node(TreeIterator treeitr)
  {
    array_pointer arr = treeitr->second;
    if ( (!_comparator(arr->front(), treeitr->first.first)
         && !_comparator(treeitr->first.first, arr->front()))
         || (!_comparator(arr->back(), treeitr->first.second)
         && !_comparator(treeitr->first.second, arr->back()))
       )
    {
      _tree.erase(treeitr);
      treeitr = _tree.insert( std::make_pair( std::make_pair( arr->front(), arr->back() ), arr) );
    }
    return treeitr;
  }

  template<typename TreeIterator>
  void _merge_node(TreeIterator itr)
  {
    
    if ( itr == _tree.begin() || itr == _tree.end())
      return;

    auto arr = itr->second;
    TreeIterator left = itr;
    --left;
    auto arrleft = left->second;

    if ( arr->size() > (arrleft->capacity() - arrleft->size())/2 )
      return;

    typename array_type::const_iterator beg = arr->begin();
    typename array_type::const_iterator end = arr->end();

    for ( ;beg!=end ; ++beg )
      arrleft->push_back(*beg);

    _re_node(left);

    _tree.erase(itr);
    _allocator.deallocate(arr, 1);

    
    
    /*
    TreeIterator left = itr;
    if (left!=_tree.begin()) --left;
    TreeIterator right = itr;
    ++right;
    size_t left_overage=0;
    if (left!=itr)
      left_overage = left->second->capacity() - left->second->size();

    size_t right_overage=0;
    if ( right!=_tree.end() )
      right_overage = right->second->capacity() - right->second->size();

    if ( itr->second->size() > (left_overage + right_overage)/2 )
      return;

    typename array_type::const_iterator beg = itr->second->begin();
    typename array_type::const_iterator end = itr->second->end();

    for ( ;beg!=end && left_overage!=0; ++beg, --left_overage )
      left->second->push_back(*beg);

    for ( ;beg!=end && right_overage!=0; ++beg, --right_overage )
      right->second->push_back(*beg);

    assert(beg==end);

    if (left!=itr )
      _re_node(left);
    
    if (right!=_tree.end() )
      _re_node(right);

    auto arrayptr = itr->second;
    _tree.erase(itr);
    _allocator.deallocate(arrayptr, 1);
    */

    
  }

  
  
  template<typename VT>
  iterator _insert(VT& value)
  {
    
    tree_iteartor treeitr = _upper_node( value );
    array_iterator aitr;

    if ( treeitr == _tree.end() )
    {
    
      array_pointer arr = _allocator.allocate(1);
      _allocator.construct(arr, array_type() );
      aitr = arr->insert(value);
      treeitr = _tree.insert( std::make_pair( std::make_pair(value, value), arr) );
    }
    else
    {
      if ( treeitr->second->filled() )
      {
        
        // Расщипляем массив
        array_pointer arr1 = treeitr->second;
        array_pointer arr2 = _allocator.allocate(1);
        size_t offset = arr1->size()/2;

        // [3,3,4] делиться на ([3,4],[3]) что нарушет последовательность
        /*if ( arr1->at(offset) == arr1->at(0) && arr1->at(offset)!=arr1->at(arr1->size() - 1) )
          for(;arr1->at(offset)==arr1->at(0); ++offset);
          */

        
        arr2->assign( arr1->begin() + offset, arr1->end() );
        
        arr1->resize( offset );
        
        _tree.insert( treeitr, std::make_pair( std::make_pair( arr1->front(), arr1->back() ), arr1) );
        _tree.insert( treeitr, std::make_pair( std::make_pair( arr2->front(), arr2->back() ), arr2) );
        _tree.erase( treeitr );
        // TDOD: Избавиться от рекурсии
        return insert(value);
      }
      else
      {
        aitr = treeitr->second->insert(value);
        treeitr = _re_node(treeitr);
        /*
        if ( _comparator(value, treeitr->first.first) || _comparator(treeitr->first.second, value) )
        {
          // Вставляем элемент меньше первого в массиве
          array_pointer arr = treeitr->second;
          _tree.erase(treeitr);
          aitr = arr->insert(value);
          treeitr = _tree.insert( std::make_pair( std::make_pair( arr->front(), arr->back() ), arr) );
        }
        else
        {
          // Для всех остальных (больше первого )
          aitr = treeitr->second->insert(value);
        }*/
      }
    }

    ++_size;
    return iterator( treeitr, std::distance(treeitr->second->begin(), aitr) );

    
    // treeitr
    
    /*
    //1. Находим последний элемент куда будем вставлять
    auto treeitr = _tree.end();
    array_iterator aitr;

    if ( !_tree.empty() )
    {
      // std::cout << "1.1" << std::endl;
      treeitr = _tree.upper_bound(value);
      if ( treeitr == _tree.end() )
      {
        //std::cout << "1.1.1" << std::endl;
        if ( value <= _tree.begin()->first )
          treeitr = _tree.begin();
        else
          treeitr = (++_tree.rbegin()).base();
      }
      else if ( _comparator(value, treeitr->first) )
      {
        // std::cout << "1.1.2" << std::endl;
        if ( treeitr != _tree.begin() )
          --treeitr;
      }
    }

    // std::cout << "2" << std::endl;
    if ( treeitr == _tree.end() )
    {
      // std::cout << "2.1" << std::endl;
      // Новый массив
      array_pointer arr = _allocator.allocate(1);
      _allocator.construct(arr, array_type() );
      aitr = arr->insert(value);
      treeitr = _tree.insert( std::make_pair( value, arr) );
    }
    else
    {
      // std::cout << "2.2" << std::endl;
      if ( treeitr->second->filled() )
      {
        // std::cout << "2.2.1" << std::endl;
        // Расщипляем массив
        array_pointer arr1 = treeitr->second;
        array_pointer arr2 = _allocator.allocate(1);
        size_t offset = arr1->size()/2;

        // [3,3,4] делиться на ([3,4],[3]) что нарушет последовательность
        if ( arr1->at(offset) == arr1->at(0) && arr1->at(offset)!=arr1->at(arr1->size() - 1) )
          for(;arr1->at(offset)==arr1->at(0); ++offset);
            
        arr2->assign( arr1->begin() + offset, arr1->end() );
        arr1->resize( offset );

        _tree.insert( treeitr, std::make_pair((*arr2)[0], arr2) );
        // TDOD: Избавиться от рекурсии
        //std::cout << "^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
        return insert(value);
      }
      else
      {
        // std::cout << "2.2.2" << std::endl;
        if ( _comparator(value, treeitr->first) )
        {
          // std::cout << "2.2.2.1" << std::endl;
          // Вставляем элемент меньше первого в массиве
          array_pointer arr = treeitr->second;
          _tree.erase(treeitr);
          aitr = arr->insert(value);
          treeitr = _tree.insert( std::make_pair( value, arr) );
        }
        else
        {
          // std::cout << "2.2.2.2" << std::endl;
          // Для всех остальных (больше первого )
          aitr = treeitr->second->insert(value);
        }
      }
    }
    // std::cout << "3" << std::endl;

    ++_size;
    return iterator( treeitr, std::distance(treeitr->second->begin(), aitr) );
    */
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

/*#ifdef __GXX_EXPERIMENTAL_CXX0X__

  void erase(const_iterator itr)
  {
  }

#else*/

  void  erase(iterator itr)
  {
    // Не можем работать с const_iterator т.к. перекидываем данные
    auto treeitr = itr.get_tree_iteartor();
    auto arrayptr = treeitr->second;
    arrayptr->erase( arrayptr->begin() + itr.get_position());
    if ( arrayptr->empty() )
    {
      _tree.erase(treeitr);
      _allocator.deallocate(arrayptr, 1);
    }
    else
    {
      treeitr = _re_node(treeitr);
      _merge_node( treeitr );
    }
    --_size;
  }

//#endif

  size_type erase(const key_type& key)
  {
    auto range = this->equal_range(key);
    return this->erase(range.first, range.second);
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__

  size_type erase(const_iterator first, const_iterator last)
  {
    size_type offset = std::distance(this->cbegin(), first);
    size_type limit = std::distance(first, last);
    for ( size_type i = 0; i < limit; ++i)
      this->erase( this->begin() +  offset );
    return limit;
    /*
    // TODO: оптимизировать
    size_type count = 0;
    for (;first!=last; ++count)
      this->erase(first++);
    return count;
    */
  }

#else

  size_type erase(iterator first, iterator first)
  {
    size_type offset = std::distance(this->begin(), first);
    size_type limit = std::distance(first, last);
    for ( size_type i = 0; i < limit; ++i)
      this->erase( this->begin() +  offset );
    return limit;
    /*
    // TODO: оптимизировать
    size_type count = 0;
    for (;first!=last; ++count)
      this->erase(first++);
    return count;
    */
  }

#endif

  void clear()
  {
    this->erase( this->begin(), this->end() );
  }

  size_type count(const key_type& key) const
  {
    auto range = this->equal_range(key);
    return std::distance(range.first, range.second);
  }

  iterator find(const key_type& key)
  {
    iterator itr = this->lower_bound(key);
    return itr == this->end() || *itr!=key ? this->end() : itr;
  }

  const_iterator find(const key_type& key ) const
  {
    const_iterator itr = this->lower_bound(key);
    return itr == this->end() || *itr!=key ? this->end() : itr;
  }

  iterator lower_bound(const key_type& key)
  {
    /*std::cout << "lower_bound " << key << std::endl;
    std::for_each( _tree.begin(), _tree.end(), []( const typename array_tree_type::value_type& node) {
      std::cout << "(" << node.first.first << "," << node.first.second << ")" << std::endl;
    } );*/
    tree_iteartor treeitr = _lower_node(key);
    if ( treeitr == _tree.end() )
      return this->end();

    //std::cout << "{" << treeitr->first.first << "," << treeitr->first.second << "}" << std::endl;

    auto arrayitr = std::lower_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );

    if ( arrayitr == treeitr->second->end() )
      return this->end();

    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr));

    /*
    auto treeitr = _tree.lower_bound(key);

    if ( treeitr == _tree.end() )
    {
      if ( _tree.empty() )
        return this->end();
      --treeitr;
    }

    auto arrayitr = std::lower_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );

    if ( arrayitr == treeitr->second->end() )
      return this->end();

    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr));
    */
  }

  const_iterator lower_bound(const key_type& key) const
  {
    
    tree_iteartor treeitr = _lower_node(key);
    if ( treeitr == _tree.end() )
      return this->end();

    auto arrayitr = std::lower_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );

    if ( arrayitr == treeitr->second->end() )
      return this->end();

    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr));

    /*
    auto treeitr = _tree.lower_bound(key);
    if ( treeitr == _tree.end() )
    {
      if ( _tree.empty() )
        return this->end();
      --treeitr;
    }
    auto arrayitr = std::lower_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );
    if ( arrayitr == treeitr->second->end() )
      return this->end();

    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr));
    */
  }

  iterator upper_bound(const key_type& key)
  {
    tree_iteartor treeitr = _upper_node(key);
    if ( treeitr == _tree.end() )
      return this->end();

    auto arrayitr = std::upper_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );

    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr) );
    
    /*
    if ( _tree.empty() )
      return this->end();

    auto treeitr = _tree.upper_bound(key);

    if ( treeitr != _tree.begin() )
      --treeitr;

    if ( treeitr == _tree.end() )
      return this->end();

    auto arrayitr = std::upper_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );
    if ( arrayitr == treeitr->second->end() )
      return iterator( ++treeitr, 0 );

    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr) );
    */
  }

  const_iterator upper_bound(const key_type& key) const
  {
    tree_iteartor treeitr = _upper_node(key);
    if ( treeitr == _tree.end() )
      return this->end();

    auto arrayitr = std::upper_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );

    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr) );

    /*
    if ( _tree.empty() )
      return this->end();

    auto treeitr = _tree.upper_bound(key);

    if ( treeitr != _tree.begin() )
      --treeitr;

    if ( treeitr == _tree.end() )
      return this->end();

    auto arrayitr = std::upper_bound( treeitr->second->begin(), treeitr->second->end(), key, _comparator );
    if ( arrayitr == treeitr->second->end() )
      return iterator( ++treeitr, 0 );

    return iterator(treeitr, std::distance(treeitr->second->begin(), arrayitr) );
    */
  }

  std::pair<iterator, iterator> equal_range(const key_type& x)
  {
    
    iterator itr = this->lower_bound(x);
    if ( itr == this->end() )
      return std::make_pair( this->end(), this->end() );

    if ( *itr != x )
      return std::make_pair( this->end(), this->end() );

    
    return std::make_pair(itr, this->upper_bound(x) );
  }

  std::pair<const_iterator, const_iterator> equal_range(const key_type& x) const
  {
    iterator itr = this->lower_bound(x);
    if ( itr == this->end() )
      return std::make_pair( this->end(), this->end() );

    if ( *itr != x )
      return std::make_pair( this->end(), this->end() );

    return std::make_pair(itr, this->upper_bound(x) );
  }


  template<typename _K1, typename _C1, typename _A1>
  friend bool operator==(const vset<_K1, _C1, _A1>&, const vset<_K1, _C1, _A1>&);

  template<typename _K1, typename _C1, typename _A1>
  friend bool operator<(const vset<_K1, _C1, _A1>&, const vset<_K1, _C1, _A1>&);

};

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator==(const vset<_Key, _Compare, _Alloc>& __x, const vset<_Key, _Compare, _Alloc>& __y)
{
  throw not_impl();
  // return __x._M_t == __y._M_t;
  return bool();
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator< (const vset<_Key, _Compare, _Alloc>& __x, const vset<_Key, _Compare, _Alloc>& __y)
{
  throw not_impl();
  // return __x._M_t < __y._M_t;
  return bool();
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool
operator!=(const vset<_Key, _Compare, _Alloc>& __x, const vset<_Key, _Compare, _Alloc>& __y)
{
  return !(__x == __y);
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator>(const vset<_Key, _Compare, _Alloc>& __x, const vset<_Key, _Compare, _Alloc>& __y)
{
  return __y < __x;
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator<=(const vset<_Key, _Compare, _Alloc>& __x, const vset<_Key, _Compare, _Alloc>& __y)
{
  return !(__y < __x);
}

template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator>=(const vset<_Key, _Compare, _Alloc>& __x, const vset<_Key, _Compare, _Alloc>& __y)
{
  return !(__x < __y);
}

template<typename _Key, typename _Compare, typename _Alloc>
inline void swap(vset<_Key, _Compare, _Alloc>& __x, vset<_Key, _Compare, _Alloc>& __y)
{
  __x.swap(__y);
}

#endif
