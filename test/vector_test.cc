#include "vector.h"

#include <string.h>
#include "gtest/gtest.h"

class allocator {
 public:
  static void* alloc(uint32_t n) { return ::malloc(n); }
  static void free(void* p) { ::free(p); }
};

typedef vector<int32_t, 10, allocator> test_vector;

TEST(vector, ctor) {
  test_vector v;
  EXPECT_EQ(0U, v.size());
}

TEST(vector, push_back) {
  test_vector v;
  v.push_back(0);
  v.push_back(1);
  v.push_back(2);
  EXPECT_EQ(3U, v.size());
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ((int32_t) i, v.at(i));
  }
}

TEST(vector, pop_back) {
  test_vector v;
  v.push_back(0);
  v.push_back(1);
  v.push_back(2);

  EXPECT_EQ(3U, v.size());
  EXPECT_EQ(2, v.back());
  EXPECT_EQ(2, v.pop_back());
  EXPECT_EQ(2U, v.size());
  EXPECT_EQ(1, v.back());
  EXPECT_EQ(1, v.pop_back());
  EXPECT_EQ(1U, v.size());
  EXPECT_EQ(0, v.back());
  EXPECT_EQ(0, v.pop_back());
  EXPECT_EQ(0U, v.size());
  EXPECT_EQ(true, v.empty());
}

TEST(vector, push_front) {
  test_vector v;
  v.push_front(2);
  v.push_front(1);
  v.push_front(0);
  EXPECT_EQ(3U, v.size());
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ((int32_t) i, v.at(i));
  }
}

TEST(vector, pop_front) {
  test_vector v;
  v.push_back(0);
  v.push_back(1);
  v.push_back(2);

  EXPECT_EQ(3U, v.size());
  EXPECT_EQ(0, v.front());
  EXPECT_EQ(0, v.pop_front());
  EXPECT_EQ(2U, v.size());
  EXPECT_EQ(1, v.front());
  EXPECT_EQ(1, v.pop_front());
  EXPECT_EQ(1U, v.size());
  EXPECT_EQ(2, v.front());
  EXPECT_EQ(2, v.pop_front());
  EXPECT_EQ(0U, v.size());
  EXPECT_EQ(true, v.empty());
}
