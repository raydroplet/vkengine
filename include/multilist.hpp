#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

template<typename T>
class multilist;

// . . .
namespace
{
  template<typename T>
  class Iterator
  {
  public:
    using ListType = multilist<T>;
    using ValueType = typename multilist<T>::valueType;

    // ListType* container;
    std::vector<size_t> blocksSize;
    std::vector<size_t> blocksCapacity;
    std::vector<T*> blocks;

    size_t block{};
    size_t index{};

    Iterator(const ListType* container, size_t block, size_t index);
    Iterator(const ListType* container, size_t block);
    ~Iterator();

    ValueType& operator*();
    Iterator& operator++();
    Iterator operator++(int);
    Iterator& operator--(int);
    Iterator operator--();
    ValueType& operator[](int index);
    ValueType* operator->();
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;
  };
} // namespace

template<typename T>
class multilist
{
public:
  using valueType = T;
  using iterator = Iterator<T>;
  using index = std::pair<uint32_t, uint32_t>; //<block, index>

  class IndexHash;

  multilist(int blockSize);
  ~multilist();

  void alloc(int size);
  index push(const T& e);
  //  void resize(int size);
  //  void resize();
  Iterator<T> begin();
  Iterator<T> end();
  int size();
  index lastIndex();
  void erase(index i);

  T& operator[](index i);

  // TODO: write a allocator that doubles the capacity of the vector
  std::vector<size_t> blocksSize_m;
  std::vector<size_t> blocksCapacity_m;
  std::vector<T*> blocks_m;

  int tCapacity_m{};
  int tSize_m{};
  int currentBlock{};
};

template<typename T>
class multilist<T>::IndexHash
{
public:
  // template <uint32_t T1, uint32_t T2>
  template<class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2>& pair) const
  {
    // return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    return std::hash<uint32_t>()(pair.first + pair.second);
  }
};

template<typename T>
Iterator<T>::Iterator(const Iterator::ListType* l, size_t block, size_t index) :
  blocksSize{l->blocksSize_m}, blocksCapacity{l->blocksCapacity_m}, blocks{l->blocks_m}, block{block}, index{index}
{
}

template<typename T>
Iterator<T>::Iterator(const Iterator::ListType* l, size_t block) :
  blocksSize{l->blocksSize_m}, blocksCapacity{l->blocksCapacity_m}, blocks{l->blocks_m}, block{block}, index{}
{
}

template<typename T>
Iterator<T>::~Iterator()
{
}

template<typename T>
typename Iterator<T>::ValueType& Iterator<T>::operator*()
{
  // return container->blocks_m[block][index];
  return blocks[block][index];
}

template<typename T>
Iterator<T>& Iterator<T>::operator++()
{
  ++index;

  // if(index >= container->blocksSize_m[block])
  if(index >= blocksSize[block]) {
    ++block;
    index = 0;
  }

  return *this;
}

template<typename T>
Iterator<T> Iterator<T>::operator++(int)
{
  Iterator it = *this;
  ++(*this);
  return it;
}

template<typename T>
Iterator<T>& Iterator<T>::operator--(int)
{
  // if(index >= container->blocksSize_m[block])
  if(index >= blocksSize[block]) {
    --block;
    index = 0;
  }
  --index;

  return *this;
}

template<typename T>
Iterator<T> Iterator<T>::operator--()
{
  Iterator it = *this;
  --(*this);
  return it;
}

template<typename T>
typename Iterator<T>::ValueType& Iterator<T>::operator[](int index)
{
  int block{};
  // for(int i = 0; i < container->blocksSize_m.back(); ++i)
  for(int i = 0; i < blocksSize.back(); ++i) {
    // if(index > container->blocksSize_m[i])
    if(index > blocksSize[i]) {
      ++block;
      // index -= container->blocksSize_m[i];
      index -= blocksSize[i];
    }
  }

  // return container->data_m[block][index];
  return blocks[block][index];
}

template<typename T>
typename Iterator<T>::ValueType* Iterator<T>::operator->()
{
  /*
  int block{};
  for(int i = 0; i < container->blocksSize_m.back(); ++i)
  {
    if(index > container->blocksSize_m[i])
    {
      ++block;
      index -= container->blocksSize_m[i];
    }
  }
*/
  // return &container->data_m[block][index];
  return &blocks[block][index];
}

template<typename T>
bool Iterator<T>::operator==(const Iterator& other) const
{
  // return block == other.block && index == other.index && container == other.container;
  return block == other.block && index == other.index && blocks == other.blocks;
}

template<typename T>
bool Iterator<T>::operator!=(const Iterator& other) const
{
  // return block != other.block && index != other.index;
  return !(*this == other);
}


template<typename T>
multilist<T>::multilist(int blockSize)
{
  blocksSize_m.reserve(8);
  blocksCapacity_m.reserve(8);
  blocks_m.reserve(8);

  alloc(blockSize);
}

template<typename T>
multilist<T>::~multilist()
{
  for(size_t i{}; i < blocks_m.size(); ++i) {
    for(size_t j{}; j < blocksSize_m[i]; ++j) {
      blocks_m[i][j].~T();
    }
  }

  // for(const auto& ptr : blocks_m)
  //   delete ptr;
}

template<typename T>
void multilist<T>::alloc(int size)
{
  blocks_m.push_back(new T[size]);

  tCapacity_m += size;
  blocksSize_m.push_back(0);
  blocksCapacity_m.push_back(size);
}

template<typename T>
typename multilist<T>::index multilist<T>::push(const T& e)
{
  if(!tCapacity_m) {
    alloc(blocksSize_m.back() * 2);
    ++currentBlock;
  }

  /*
  // search the block with the smallest capacity.
  size_t block{};
  size_t smaller{blocksCapacity_m.front()};
  for(size_t i = 1; i < blocksCapacity_m.size(); ++i)
  {
    if(smaller < blocksCapacity_m[i])
    {
      smaller = blocksCapacity_m[i];
      ++block;
    }
  }
  */

  // size_t block = blocks_m.size() - 1;
  size_t index = blocksSize_m.back();

  blocks_m[currentBlock][index] = e;

  ++blocksSize_m[currentBlock];
  ++tSize_m;

  --blocksCapacity_m[currentBlock];
  --tCapacity_m;

  return multilist<T>::index{currentBlock, index};
}

template<typename T>
Iterator<T> multilist<T>::begin()
{
  // return Iterator{};
  return Iterator<T>{this, 0, 0};
}

template<typename T>
Iterator<T> multilist<T>::end()
{
  return Iterator<T>{this, blocks_m.size()};
}

template<typename T>
int multilist<T>::size()
{
  return tSize_m;
}

template<typename T>
typename multilist<T>::index multilist<T>::lastIndex()
{
  // uint32_t block = blocks_m.size() - 1;
  // uint32_t index = blocksSize_m.back() - 1;
  uint32_t index = blocksSize_m[currentBlock] - 1;

  return typename multilist<T>::index{currentBlock, index};
}

template<typename T>
T& multilist<T>::operator[](index i)
{
  return blocks_m[i.first][i.second];
}

template<typename T>
void multilist<T>::erase(index i)
{
  using Index = typename multilist<T>::index;

  assert(i.first <= (static_cast<uint32_t>(currentBlock)) && "Invalid multilist index.");
  assert(i.second <= (blocksSize_m[i.first] - 1) && "Invalid multilist index.");

  Index last_index{lastIndex()};

  if(i != last_index) {
    (*this)[i] = std::move((*this)[last_index]);
  } else {
    (*this)[i].~T();
  }

  // multilist<T>::operator[](last_index).~T();

  ++tCapacity_m;
  ++blocksCapacity_m[last_index.first];
  --tSize_m;
  --blocksSize_m[last_index.first];

  if(blocksSize_m[currentBlock] == 0)
    --currentBlock;
}

///
