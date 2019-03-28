#include "service_layer_client_sync.h"

#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace chirpsystem {
class ServiceLayerClientTest : public ::testing::Test {
 protected:
  void SetUp() override { service_layer_client_.Init(); }
  ServiceLayerClient service_layer_client_;
};

// Registeruser should succeed
TEST_F(ServiceLayerClientTest, RegisteruserShouldWork) {
  EXPECT_TRUE(service_layer_client_.RegisterUser("test"));
}

// Register empty user should fail
TEST_F(ServiceLayerClientTest, RegisterEmptyUserShouldFail) {
  EXPECT_FALSE(service_layer_client_.RegisterUser(""));
}

// Register existing user should fail
TEST_F(ServiceLayerClientTest, RegisterExistingUserShouldFail) {
  EXPECT_TRUE(service_layer_client_.RegisterUser("aUniqueName"));
  EXPECT_FALSE(service_layer_client_.RegisterUser("aUniqueName"));
}

// Follow a existing user should succeed
TEST_F(ServiceLayerClientTest, FollowShouldWork) {
  ASSERT_TRUE(service_layer_client_.RegisterUser("test2"));
  ASSERT_TRUE(service_layer_client_.RegisterUser("test1"));
  EXPECT_TRUE(service_layer_client_.Follow("test2", "test1"));
}

// Follow a not existing user should fail
TEST_F(ServiceLayerClientTest, FollowNotExistingUserShoudFail) {
  ASSERT_TRUE(service_layer_client_.RegisterUser("followTest3"));
  EXPECT_FALSE(
      service_layer_client_.Follow("followTest3", "not existing user"));
}

// Using a not existing user to follow should fail
TEST_F(ServiceLayerClientTest, UsingNotExistingUserToFollowShouldFail) {
  ASSERT_TRUE(service_layer_client_.RegisterUser("followTest4"));
  EXPECT_FALSE(
      service_layer_client_.Follow("not existing user", "followTest4"));
}

// Using not exsitng user to follow not existng user should fail
TEST_F(ServiceLayerClientTest,
       UsingNotExistingUserToFollowNotExistingUserShouldFail) {
  EXPECT_FALSE(
      service_layer_client_.Follow("not existing user", "not existing usesr2"));
}

// Create a chirp should succeed
TEST_F(ServiceLayerClientTest, ChirpShouldWork) {
  ASSERT_TRUE(service_layer_client_.RegisterUser("test3"));
  const std::string& id =
      service_layer_client_.SendChirp("test3", "test content");
  EXPECT_NE("", id);
}

// Read a chirp should succeed
TEST_F(ServiceLayerClientTest, ReadChirpShouldWork) {
  ASSERT_TRUE(service_layer_client_.RegisterUser("test4"));
  const std::string& id =
      service_layer_client_.SendChirp("test4", "test content");
  ASSERT_NE("", id);
  std::vector<Chirp> chirps = service_layer_client_.Read(id);
  ASSERT_NE(0, chirps.size());
  EXPECT_EQ("test content", chirps[0].text());
}
}  // namespace chirpsystem
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}