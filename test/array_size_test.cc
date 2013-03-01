#include "base/array_size.h"

#include "gtest/gtest.h"

TEST(ARRAY_SIZE, ARRAY_SIZE) {
  static const size_t sz = 128;
  char buff[sz];
  int ibuff[sz];

  EXPECT_EQ(sz, ARRAY_SIZE(buff));
  EXPECT_EQ(sz, ARRAY_SIZE(ibuff));
}
