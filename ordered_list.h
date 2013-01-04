#ifndef ORDERED_LIST_H
#define ORDERED_LIST_H

#include "kalloc.h"
#include "memory.h"
#include "screen.h"

template<typename T>
bool DefaultPredicate(const T& a, const T& b) { return a < b; }

template<typename T>
class OrderedList {
 public:
  typedef bool (*Predicate)(const T&, const T&);
  
  OrderedList(void* array, uint32_t capacity, Predicate predicate);

  void Insert(const T& item);
  const T& Lookup(uint32_t i) const;
  void Remove(uint32_t i);

  uint32_t size() const { return size_; }

 private:

  T* array_;
  Predicate predicate_;
  uint32_t size_;
  uint32_t capacity_;
};

template<typename T>
OrderedList<T>::OrderedList(void* array, uint32_t capacity, Predicate predicate)
    : array_((T*) array),
      predicate_(predicate),
      size_(0),
      capacity_(capacity) {
  for (uint32_t i = 0; i < capacity; ++i)
    array_[i] = T();
}

template<typename T>
void OrderedList<T>::Insert(const T& item) {
  uint32_t iterator = 0;
  while (iterator < size_ && predicate_(array_[iterator], item))
    ++iterator;
  if (iterator == size_)
    array_[size_++] = item;
  else {
    T& tmp = array_[iterator];
    array_[iterator] = item;
    while (iterator < size_) {
      ++iterator;
      const T& tmp2 = array_[iterator];
      array_[iterator] = tmp;
      tmp = tmp2;
    }
    ++size_;
  }

  if (size_ >= capacity_)
    PANIC("Ordered list at capacity");
}

template<typename T>
const T& OrderedList<T>::Lookup(uint32_t i) const {
  if (i >= size_) {
    PANIC("out of bounds");
  }
  return array_[i];
}

template<typename T>
void OrderedList<T>::Remove(uint32_t i) {
  while (i < size_) {
    array_[i] = array_[i+1];
    ++i;
  }
  --size_;
}

#endif  // ORDERED_LIST_H
