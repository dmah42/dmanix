#ifndef ORDERED_ARRAY_H
#define ORDERED_ARRAY_H

#include "base/assert.h"
#include "memory/kalloc.h"
#include "memory/memory.h"
#include "screen.h"

template<typename T>
bool DefaultPredicate(const T& a, const T& b) { return a < b; }

template<typename T>
class OrderedArray {
 public:
  typedef bool (*Predicate)(const T&, const T&);
  
  OrderedArray(void* array, uint32_t capacity, Predicate predicate);
//  static OrderedArray Create(void* array,
//                            uint32_t capacity,
//                            Predicate predicate);

  void Insert(const T& item);
  const T& Lookup(uint32_t i) const;
  void Remove(uint32_t i);

  uint32_t get_size() const { return size; }

 private:

  T* array;
  Predicate predicate;
  uint32_t size;
  uint32_t capacity;
};

// static
// TODO: specialize for pointer type
/*
template<typename T>
OrderedArray<T> OrderedArray<T>::Create(void* array,
                                      uint32_t capacity,
                                      Predicate predicate) {
  OrderedArray<T> list;
  list.array = (T*) array;
  memory::set(array, 0, capacity * sizeof(T));
  list.size = 0;
  list.capacity = capacity;
  list.predicate = predicate;
  return list;
}
*/

template<typename T>
OrderedArray<T>::OrderedArray(void* array, uint32_t capacity, Predicate predicate)
    : array((T*) array),
      predicate(predicate),
      size(0),
      capacity(capacity) {
  memory::set8((uint8_t*) array, 0, capacity * sizeof(T));
}

template<typename T>
void OrderedArray<T>::Insert(const T& item) {
  uint32_t iterator = 0;
  while (iterator < size && predicate(array[iterator], item))
    ++iterator;
  if (iterator == size)
    array[size++] = item;
  else {
    T tmp = array[iterator];
    array[iterator] = item;
    while (iterator < size) {
      ++iterator;
      const T tmp2 = array[iterator];
      array[iterator] = tmp;
      tmp = tmp2;
    }
    ++size;
  }

  ASSERT(size < capacity);
}

template<typename T>
const T& OrderedArray<T>::Lookup(uint32_t i) const {
  ASSERT(i < size);
  return array[i];
}

template<typename T>
void OrderedArray<T>::Remove(uint32_t i) {
  while (i < size) {
    array[i] = array[i+1];
    ++i;
  }
  --size;
}

#endif  // ORDERED_ARRAY_H
