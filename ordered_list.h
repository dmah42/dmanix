#ifndef ORDERED_LIST_H
#define ORDERED_LIST_H

#include "assert.h"
#include "kalloc.h"
#include "memory.h"
#include "screen.h"

template<typename T>
bool DefaultPredicate(const T& a, const T& b) { return a < b; }

template<typename T>
class OrderedList {
 public:
  typedef bool (*Predicate)(const T&, const T&);
  
//  OrderedList(void* array, uint32_t capacity, Predicate predicate);
  static OrderedList Create(void* array,
                            uint32_t capacity,
                            Predicate predicate);

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
template<typename T>
OrderedList<T> OrderedList<T>::Create(void* array,
                                      uint32_t capacity,
                                      Predicate predicate) {
  OrderedList<T> list;
  list.array = (T*) array;
  memory::set(array, 0, capacity * sizeof(T));
  list.size = 0;
  list.capacity = capacity;
  list.predicate = predicate;
  return list;
}

//template<typename T>
//OrderedList<T>::OrderedList(void* array, uint32_t capacity, Predicate predicate)
//    : array_((T*) array),
//      predicate_(predicate),
//      size_(0),
//      capacity_(capacity) {
//  for (uint32_t i = 0; i < capacity; ++i)
//    array_[i] = T();
//}

template<typename T>
void OrderedList<T>::Insert(const T& item) {
  uint32_t iterator = 0;
  while (iterator < size && predicate(array[iterator], item))
    ++iterator;
  if (iterator == size)
    array[size++] = item;
  else {
    T& tmp = array[iterator];
    array[iterator] = item;
    while (iterator < size) {
      ++iterator;
      const T& tmp2 = array[iterator];
      array[iterator] = tmp;
      tmp = tmp2;
    }
    ++size;
  }

  if (size >= capacity)
    PANIC("Ordered list at capacity");
}

template<typename T>
const T& OrderedList<T>::Lookup(uint32_t i) const {
  if (i >= size) {
    PANIC("out of bounds");
  }
  return array[i];
}

template<typename T>
void OrderedList<T>::Remove(uint32_t i) {
  while (i < size) {
    array[i] = array[i+1];
    ++i;
  }
  --size;
}

#endif  // ORDERED_LIST_H
