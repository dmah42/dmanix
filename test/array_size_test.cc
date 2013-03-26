#include "base/array_size.h"

#include "gtest/gtest.h"

TEST(ARRAY_SIZE, ARRAY_SIZE) {
  static const size_t kSz = 128;
  char buff[kSz];
  int ibuff[kSz];

  EXPECT_EQ(kSz, ARRAY_SIZE(buff));
  EXPECT_EQ(kSz, ARRAY_SIZE(ibuff));
}
