#include "store.h"
#include <gtest/gtest.h>

TEST(StoreTest, GetShouldReturnEmptyStringIfKeyDoesNotExist)
{
  Store store;
  EXPECT_EQ("", store.Get("test"));
}

TEST(StoreTest, GetShouldReturnCorrespondingValueIfKeyExist)
{
  Store store;
  store.Put("test", "test");
  EXPECT_EQ("test", store.Get("test"));
}

TEST(StoreTest, PutShouldPutValueToKeyLocation)
{
  Store store;
  store.Put("test", "test1");
  EXPECT_EQ("test1", store.Get("test"));
}

TEST(StoreTest, RemoveShouldReturnFalseWhenKeyNotExist)
{
  Store store;
  EXPECT_EQ(false, store.Remove("test"));
}

TEST(StoreTest, RemoveShouldRemoveCorrespondingValueWhenKeyExist)
{
  Store store;
  store.Put("test", "test1");
  EXPECT_EQ("test1", store.Get("test"));
  store.Remove("test");
  EXPECT_EQ("", store.Get("test"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}