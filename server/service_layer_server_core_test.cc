#include "service_layer_server_core.h"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

// TODO: Add tests for error cases
namespace chirpsystem {
class ServiceLayerServerCoreTest : public ::testing::Test {
 protected:
  //  Truen on dev
  void SetUp() override { service_layer_server_core_.Init(true); }
  ServiceLayerServerCore service_layer_server_core_;
};

//   `RegisterUser` should work
TEST_F(ServiceLayerServerCoreTest, RegisterUserShouldWork) {
  // Should be able to register a non-existing user
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test"));
}

// `RegisterUser` should fail for empty username
TEST_F(ServiceLayerServerCoreTest, RegisterUserShouldFailForEmptyUsername) {
  // Username must not be blank
  EXPECT_FALSE(service_layer_server_core_.RegisterUser(""));
}

// `RegisterUser` should fail for duplicated username
TEST_F(ServiceLayerServerCoreTest,
       RegisterUserShouldFailForDuplicatedUsername) {
  const std::string name_to_duplicate = "aNameToBeDuplicated";
  EXPECT_TRUE(service_layer_server_core_.RegisterUser(name_to_duplicate));
  // Username must not be blank
  // Same username cannot be registered twice
  EXPECT_FALSE(service_layer_server_core_.RegisterUser(name_to_duplicate));
}

//  `Follow` should work
TEST_F(ServiceLayerServerCoreTest, FollowShouldWork) {
  // Should be able to follow a existing user
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test1"));
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test2"));
  EXPECT_TRUE(service_layer_server_core_.Follow("test1", "test2"));
}

// `Follow` using not registerd `useranme` or `to_follow` should return false
TEST_F(ServiceLayerServerCoreTest, FollowShouldReturnFalseForUnregisteredUser) {
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test3"));
  // existing username and not existng to_follow
  EXPECT_FALSE(service_layer_server_core_.Follow("test3", "invalid_user"));
  // not existing username and exsting to_follow
  EXPECT_FALSE(service_layer_server_core_.Follow("invalid_user", "test3"));
  /// not existing useranme and not existing to_follow
  EXPECT_FALSE(
      service_layer_server_core_.Follow("invalid_user", "invalid_user2"));
}

// `Follow` should not allow same  `username` and `to_follow`
TEST_F(ServiceLayerServerCoreTest, FollowShouldReturnFalseForSelfFollowing) {
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test4"));
  // `username` and `to_follow` should be different
  EXPECT_FALSE(service_layer_server_core_.Follow("test4", "invalid_user4"));
}

// `SendChirp` should work for all valid input
TEST_F(ServiceLayerServerCoreTest, ChirpShouldWork) {
  // A exsting user should be able to chirp
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test3"));
  Chirp start_chirp;
  ServiceLayerServerCore::ChirpStatus status =
      service_layer_server_core_.SendChirp(start_chirp, "test3", "hohoho");
  EXPECT_EQ(ServiceLayerServerCore::CHIRP_SUCCEED, status);
  EXPECT_NE("", start_chirp.id());

  // A exsting user should be able to reply
  Chirp reply_chirp;
  status = service_layer_server_core_.SendChirp(reply_chirp, "test3", "hohoho",
                                                start_chirp.id());
  EXPECT_EQ(ServiceLayerServerCore::CHIRP_SUCCEED, status);
  EXPECT_NE("", reply_chirp.id());
}

// `SendChirp` should return CHIRP_FAILED when username is invalid
TEST_F(ServiceLayerServerCoreTest, ChirpShouldFailForInvalidUsername) {
  // Empty useranme should fail
  Chirp start_chirp;
  ServiceLayerServerCore::ChirpStatus status =
      service_layer_server_core_.SendChirp(start_chirp, "", "hohoho");
  EXPECT_EQ(ServiceLayerServerCore::CHIRP_FAILED, status);

  // Non-existing username should fail
  Chirp reply_chirp;
  status = service_layer_server_core_.SendChirp(
      reply_chirp, "not existing username", "hohoho");
  EXPECT_EQ(ServiceLayerServerCore::CHIRP_FAILED, status);
}

// `SendChirp` should return CHIRP_FAILED for a reply whose parent_id does not
// exist
TEST_F(ServiceLayerServerCoreTest, ChirpShouldFailForInvalidParentId) {
  // Invalid parent_id should fail
  Chirp start_chirp;
  ServiceLayerServerCore::ChirpStatus status =
      service_layer_server_core_.SendChirp(start_chirp, "", "hohoho",
                                           "aNotExistingId");
  EXPECT_EQ(ServiceLayerServerCore::CHIRP_FAILED, status);
}

// `Read` should work
TEST_F(ServiceLayerServerCoreTest, ReadShouldWork) {
  // Should be able to read a existing chirp
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test4"));
  Chirp start_chirp;
  ServiceLayerServerCore::ChirpStatus status =
      service_layer_server_core_.SendChirp(start_chirp, "test4", "hohoho");
  std::vector<Chirp> chirps = service_layer_server_core_.Read(start_chirp.id());
  EXPECT_EQ(1, chirps.size());
  EXPECT_EQ(start_chirp.text(), chirps[0].text());
  // Should be able to read a thread
  Chirp reply_chirp;
  status = service_layer_server_core_.SendChirp(reply_chirp, "test4", "hihihi",
                                                start_chirp.id());
  chirps = service_layer_server_core_.Read(start_chirp.id());
  EXPECT_EQ(2, chirps.size());
  EXPECT_EQ(start_chirp.text(), chirps[0].text());
  EXPECT_EQ(reply_chirp.text(), chirps[1].text());
}

// `Read` should return empty vector for invalid id
TEST_F(ServiceLayerServerCoreTest, ReadShouldReturnEmptyVectorForInvalidId) {
  // Should fail to read a empty id
  std::vector<Chirp> chirps = service_layer_server_core_.Read("");
  EXPECT_EQ(0, chirps.size());
  // Should fail to read a not exisitng id
  chirps = service_layer_server_core_.Read("aNotExistingId");
  EXPECT_EQ(0, chirps.size());
}

// `Monitor` should work
TEST_F(ServiceLayerServerCoreTest, MonitorShouldWork) {
  // After a folling user post a chrip, the follower monitoring should be able
  // receive it.
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test5"));
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test6"));
  EXPECT_TRUE(service_layer_server_core_.Follow("test6", "test5"));
  Chirp output_chirp;
  std::thread monitoring([this, &output_chirp]() {
    service_layer_server_core_.Monitor("test6",
                                       [&output_chirp](Chirp chirp) {
                                         output_chirp = chirp;
                                         return false;
                                       },
                                       0, 0);
  });
  Chirp parent_chirp;
  service_layer_server_core_.SendChirp(parent_chirp, "test5", "hohoho");
  // Wait for monitoring to end, after it is end outpu_chirp should equal to
  // parent_chirp
  monitoring.join();
  EXPECT_EQ(parent_chirp.id(), output_chirp.id());
}

// `Monitor` should return false for invalid username and unregistered user
TEST_F(ServiceLayerServerCoreTest, MonitorShouldReturnFalseOnInvalidInput) {
  // After a folling user post a chrip, the follower monitoring should be able
  // receive it.
  bool empty_ok = service_layer_server_core_.Monitor(
      "empty_user", [](Chirp chirp) { return false; });
  EXPECT_FALSE(empty_ok);
  bool unregistered_ok = service_layer_server_core_.Monitor(
      "unregistered_user", [](Chirp chirp) { return false; });
  EXPECT_FALSE(unregistered_ok);
}

// `Monitor` should return false for less than 0 interval
TEST_F(ServiceLayerServerCoreTest, MonitorShouldReturnFalseOnInvalidInterval) {
  bool zero_ok = service_layer_server_core_.Monitor(
      "empty_user", [](Chirp chirp) { return false; }, 0);
  EXPECT_FALSE(zero_ok);
}

}  // namespace chirpsystem

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}