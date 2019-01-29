#include "key_value_store_client_sync.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class KeyValueStoreClientSyncTest : public ::testing::Test {
 protected:
  void SetUp() override { client.Init(); }
  KeyValueStoreClient client;
};

// `Put` should store corresponding key value pair
TEST_F(KeyValueStoreClientSyncTest, PutShouldStoreValue) {
  EXPECT_TRUE(client.Put("test", "test"));
}

// Givent a existed key `Get` should retrive its corresponding value
TEST_F(KeyValueStoreClientSyncTest, GetShouldRetriveSingleValue) {
  client.Put("test", "test");
  std::vector<std::string> keys = {"test"};
  std::string result = "";
  client.Get(keys, [&result](std::string value) { result = value; });
  EXPECT_EQ("test", result);
}

// After `DeleteKey` was called for a key, `Get` should received empty value for
// the key.
TEST_F(KeyValueStoreClientSyncTest, DeleteKeyShouldRemoveItemForKey) {
  // Store should have ("test", "test")
  EXPECT_TRUE(client.Put("test", "test"));
  std::vector<std::string> keys = {"test"};
  std::string result = "";
  client.Get(keys, [&result](std::string value) { result = value; });
  EXPECT_EQ("test", result);
  // After `DeleteKey`, Get should retrive empty string
  EXPECT_TRUE(client.DeleteKey("test"));
  client.Get(keys, [&result](std::string value) { result = value; });
  EXPECT_EQ("", result);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
