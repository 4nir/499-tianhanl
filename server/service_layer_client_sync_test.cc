#include "service_layer_client_sync.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class ServiceLayerClientTest : public ::testing::Test {
 protected:
  void SetUp() override { service_layer_client_.Init(); }
  ServiceLayerClient service_layer_client_;
};

// Registeruser should succeed
TEST_F(ServiceLayerClientTest, RegisteruserShouldWork) {
  EXPECT_TRUE(service_layer_client_.RegisterUser("test"));
}

// Follow a existing user should succeed
TEST_F(ServiceLayerClientTest, FollowShouldWork) {
  ASSERT_TRUE(service_layer_client_.RegisterUser("test2"));
  ASSERT_TRUE(service_layer_client_.RegisterUser("test1"));
  EXPECT_TRUE(service_layer_client_.Follow("test2", "test1"));
}

// Create a chirp should succeed
TEST_F(ServiceLayerClientTest, ChirpShouldWork) {
  ASSERT_TRUE(service_layer_client_.RegisterUser("test3"));
  EXPECT_TRUE(service_layer_client_.Chirp("test3", "test content"));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}