#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <stdlib.h>

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

#include "memory/kalloc.h"

// Fixed capacity vector.
template <typename T, uint32_t N>
class vector {
 public:
  vector();
  ~vector();

  void push_front(const T& t);
  void push_back(const T& t);
  T pop_front();
  T pop_back();

  void insert(size_t index, const T& t);
  void remove(size_t index);

  const T& at(size_t index) const;

  const T& front() const { return at(0); }
  const T& back() const { return at(size_ - 1); }

  size_t size() const { return size_; }
  bool empty() const { return size() == 0U; }

 private:
  T* array_;
  uint32_t size_;
};

template <typename T, uint32_t N>
vector<T, N>::vector()
    : array_((T*) kalloc(N)),
      size_(0) {
}

template <typename T, uint32_t N>
vector<T, N>::~vector() {
  kfree(array_);
}

template <typename T, uint32_t N>
void vector<T, N>::push_front(const T& t) {
  insert(0, t);
}

template <typename T, uint32_t N>
void vector<T, N>::push_back(const T& t) {
  ASSERT(size_ < N);
  array_[size_] = t;
  ++size_;
}

template <typename T, uint32_t N>
T vector<T, N>::pop_front() {
  T ret = front();
  remove(0);
  return ret;
}
  
template <typename T, uint32_t N>
T vector<T, N>::pop_back() {
  ASSERT(size > 0);
  --size_;
  return array_[size_];
}

template <typename T, uint32_t N>
void vector<T, N>::insert(size_t index, const T& t) {
  ASSERT(index < N);
  ASSERT(size_ < N);
  for (uint32_t i = size_; i > index; --i) {
    array_[i] = array_[i - 1];
  }
  array_[index] = t;
  ++size;
}

template <typename T, uint32_t N>
void vector<T, N>::remove(size_t index) {
  ASSERT(index < N);
  ASSERT(size > 0);
  for (uint32_t i = index; i < size_; ++i) {
    array_[i] = array_[i + 1];
  }
  --size;
}

template <typename T, uint32_t N>
const T& vector<T, N>::at(size_t index) const {
  ASSERT(index < size_);
  return array_[index];
}

#endif  // VECTOR_H
