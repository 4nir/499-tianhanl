#include "./service_layer_server_core.h"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// TODO: Add tests for error cases

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
  // Username must not be blank
  EXPECT_FALSE(service_layer_server_core_.RegisterUser(""));
}

//  `Follow` should work
TEST_F(ServiceLayerServerCoreTest, FollowShouldWork) {
  // Should be able to follow a existing user
  EXPECT_TRUE(service_layer_server_core_.RegisterUser("test2"));
  EXPECT_TRUE(service_layer_server_core_.Follow("test", "test2"));
}

// `Chirp` should work
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
  status = service_layer_server_core_.SendChirp(reply_chirp, "test3", "hohoho");
  EXPECT_EQ(ServiceLayerServerCore::CHIRP_SUCCEED, status);
  EXPECT_NE("", reply_chirp.id());
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
                                       10);
  });
  Chirp parent_chirp;
  service_layer_server_core_.SendChirp(parent_chirp, "test5", "hohoho");
  // Wait for monitoring to end, after it is end outpu_chirp should equal to
  // parent_chirp
  monitoring.join();
  EXPECT_EQ(parent_chirp.id(), output_chirp.id());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}