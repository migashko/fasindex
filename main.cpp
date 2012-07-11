#include <functional>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>

template<typename T>
class sorted_vector_iterator
{
public:
  typedef typename T::size_type size_type;
  sorted_vector_iterator(T& t, size_type position)
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
  typedef std::pair<chain_iterator, cell_iterator> pair_iterator;
  typedef typename cell_type::size_type size_type;
  typedef sorted_vector_iterator<self> iterator;
  typedef sorted_vector_iterator<const self> const_iterator;

  sorted_vector(): _size(0)
  {
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

int main()
{
  
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
}