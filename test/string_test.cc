#include "string.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

TEST(string, length) {
  EXPECT_DEATH(string::length(NULL), "s != NULL");
  EXPECT_EQ(0U, string::length(""));
  EXPECT_EQ(3U, string::length("abc"));
}

TEST(string, compare) {
  EXPECT_DEATH(string::compare(NULL, "abc"), "a != NULL");
  EXPECT_DEATH(string::compare("abc", NULL), "b != NULL");
  EXPECT_TRUE(string::compare("abc", "abc"));
  EXPECT_TRUE(string::compare("", ""));
  EXPECT_FALSE(string::compare("abc", "def"));
  EXPECT_FALSE(string::compare("abc", ""));
  EXPECT_FALSE(string::compare("", "abc"));
  EXPECT_FALSE(string::compare("abc", "ABC"));
  EXPECT_FALSE(string::compare("ABC", "abc"));
  EXPECT_FALSE(string::compare("abc", "abcd"));
  EXPECT_FALSE(string::compare("abcd", "abc"));
}

TEST(string, copy) {
  char dest[4] = {0};
  EXPECT_DEATH(string::copy(NULL, "abc"), "dest != NULL");
  EXPECT_DEATH(string::copy(dest, NULL), "source != NULL");
  char* new_dest = string::copy(dest, "abc");
  EXPECT_EQ(dest, new_dest);
  EXPECT_STREQ("abc", dest);
}

TEST(string, find) {
  EXPECT_DEATH(string::find(NULL, 'a'), "s != NULL");
  EXPECT_EQ(static_cast<uint32_t>(-1), string::find("abc", 'd'));
  EXPECT_EQ(0U, string::find("abc", 'a'));
  EXPECT_EQ(1U, string::find("abc", 'b'));
  EXPECT_EQ(static_cast<uint32_t>(-1), string::find("abc", '\0'));
}
