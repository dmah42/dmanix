#include "bitset.h"

#include "gtest/gtest.h"

#define N 1024

TEST(bitset, ctor) {
  bitset bitset(N);
  EXPECT_EQ(0U, bitset.next());
  for (uint32_t i = 0; i < N; ++i) {
    EXPECT_FALSE(bitset.test(i));
  }
}

TEST(bitset, set) {
  bitset bitset(N);
  bitset.set(0U);
  for (uint32_t i = 0; i < N; ++i) {
    EXPECT_EQ(i == 0U, bitset.test(i));
  }
}

TEST(bitset, clear) {
  bitset bitset(N);
  bitset.set(2);
  EXPECT_TRUE(bitset.test(2));
  bitset.clear(0);
  EXPECT_TRUE(bitset.test(2));
  bitset.clear(2);
  EXPECT_FALSE(bitset.test(2));
}

TEST(bitset, next) {
  bitset bitset(N);
  EXPECT_EQ(0U, bitset.next());
  bitset.set(0);
  EXPECT_EQ(1U, bitset.next());
}

