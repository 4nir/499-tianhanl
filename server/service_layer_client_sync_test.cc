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

TEST_F(ServiceLayerClientTest, RegisteruserShouldWork) {
  EXPECT_TRUE(service_layer_client_.RegisterUser("test"));
}

TEST_F(ServiceLayerClientTest, FollowShouldWork) {
  ASSERT_TRUE(service_layer_client_.RegisterUser("test2"));
  ASSERT_TRUE(service_layer_client_.RegisterUser("test1"));
  EXPECT_TRUE(service_layer_client_.Follow("test2", "test1"));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}