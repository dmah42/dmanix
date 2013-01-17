#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <stdlib.h>

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

// Fixed capacity vector.
template <typename T, uint32_t N>
class vector_base {
 public:
  vector_base();
  ~vector_base();

  void push_front(const T& t);
  void push_back(const T& t);
  T pop_front();
  T pop_back();

  const T& at(size_t index) const;

  const T& front() const { return at(0); }
  const T& back() const { return at(size_ - 1); }

  size_t size() const { return size_; }
  bool empty() const { return size() == 0U; }

 protected:
  T* array_;

 private:
  uint32_t size_;
};

template <typename T, uint32_t N>
vector_base<T, N>::vector_base()
    : array_(NULL),
      size_(0) {
}

template <typename T, uint32_t N>
vector_base<T, N>::~vector_base() {
}

template <typename T, uint32_t N>
void vector_base<T, N>::push_front(const T& t) {
  ASSERT(size_ < N);
  for (uint32_t i = size_; i > 0; --i) {
    array_[i] = array_[i - 1];
  }
  array_[0] = t;
  ++size_;
}

template <typename T, uint32_t N>
void vector_base<T, N>::push_back(const T& t) {
  ASSERT(size_ < N);
  array_[size_] = t;
  ++size_;
}

template <typename T, uint32_t N>
T vector_base<T, N>::pop_front() {
  T ret = array_[0];
  for (uint32_t i = 0; i < size_ - 1; ++i) {
    array_[i] = array_[i + 1];
  }
  --size_;
  return ret;
}
  
template <typename T, uint32_t N>
T vector_base<T, N>::pop_back() {
  --size_;
  return array_[size_];
}

template <typename T, uint32_t N>
const T& vector_base<T, N>::at(size_t index) const {
  ASSERT(index < size_);
  return array_[index];
}

// vector specialized with allocator
template <typename T, uint32_t N, typename A>
class vector : public vector_base<T, N> {
 public:
  typedef vector_base<T, N> super;

  vector() { super::array_ = (T*) A::alloc(N); }
  ~vector() { A::free(super::array_); }
};

#endif  // VECTOR_H
