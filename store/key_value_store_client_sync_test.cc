#include "key_value_store_client_sync.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class KeyValueStoreClientSyncTest : public ::testing::Test {
 protected:
  void SetUp() override { client.init(); }
  KeyValueStoreClient client;
};

TEST_F(KeyValueStoreClientSyncTest, PutShouldStoreValue) {
  // `Put` should store corresponding key value pair
  EXPECT_TRUE(client.Put("test", "test"));
}

TEST_F(KeyValueStoreClientSyncTest, GetShouldRetriveSingleValue) {
  // Givent a existed key `Get` should retrive its corresponding value
  client.Put("test", "test");
  std::vector<std::string> keys = {"test"};
  std::string result = "";
  client.Get(keys, [&result](std::string value) { result = value; });
  EXPECT_EQ("test", result);
}

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
