#include "store.h"

#include <gtest/gtest.h>

TEST(StoreTest, GetShouldReturnEmptyStringIfKeyDoesNotExist) {
  Store store;
  // Since store is initially empty, "test" key is not exist. Get("test") should
  // return empty string;
  EXPECT_EQ("", store.Get("test"));
}

TEST(StoreTest, GetShouldReturnCorrespondingValueIfKeyExist) {
  Store store;
  store.Put("test", "test");
  // After user have put ("test", "test") key value pair, Get("test") should
  // return "test"
  EXPECT_EQ("test", store.Get("test"));
}

TEST(StoreTest, PutShouldPutValueToKeyLocation) {
  Store store;
  store.Put("test", "test1");
  // After user have put ("test", "test") key value pair, Get("test") should
  // return "test"
  EXPECT_EQ("test1", store.Get("test"));
}

TEST(StoreTest, RemoveShouldReturnFalseWhenKeyNotExist) {
  Store store;
  // Since store is initially empty, "test" key is not exit. Remove("test")
  // should return false because it cannot find the key.
  EXPECT_EQ(false, store.Remove("test"));
}

TEST(StoreTest, RemoveShouldRemoveCorrespondingValueWhenKeyExist) {
  Store store;
  store.Put("test", "test1");
  EXPECT_EQ("test1", store.Get("test"));
  store.Remove("test");
  // After Remove("test") has been called, Get("test") should not be able to
  // find the key and return empty string.
  EXPECT_EQ("", store.Get("test"));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}