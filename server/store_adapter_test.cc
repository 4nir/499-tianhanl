#include "store_adapter.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

class StoreAdapterTest : public ::testing::Test {
 protected:
  //  Turn on dev key
  void SetUp() override { store_adapter_.Init(true); }
  StoreAdapter store_adapter_;
};

// Create a new chirp using input info, timestamp will be set to all 0
Chirp makeChirp(const std::string& username, const std::string& text,
                const std::string& id, const std::string& parent_id) {
  Chirp chirp;
  chirp.set_username(username);
  chirp.set_text(text);
  chirp.set_id(id);
  chirp.set_parent_id(parent_id);
  Timestamp* test_timestamp(chirp.mutable_timestamp());
  test_timestamp->set_seconds(0);
  test_timestamp->set_useconds(0);
  return chirp;
}

//  `StoreUserInfo` should store the UserInfo into store server
TEST_F(StoreAdapterTest, StoreFullserInfoShouldWork) {
  // Stores a fully filled UserInfo should succeed
  UserInfo test_info;
  test_info.set_username("test");
  test_info.add_following("test2");
  test_info.add_chirp_id_s("1");
  Timestamp* test_timestamp(test_info.mutable_timestamp());
  test_timestamp->set_seconds(0);
  test_timestamp->set_useconds(0);
  EXPECT_TRUE(store_adapter_.StoreUserInfo(test_info));
  // Get stored userinfo for the id
  UserInfo fetched_info = store_adapter_.GetUserInfo("test");
  Timestamp* fetched_timestamp(fetched_info.mutable_timestamp());
  EXPECT_EQ(test_info.username(), fetched_info.username());
  EXPECT_EQ(test_info.following(0), fetched_info.following(0));
  EXPECT_EQ(test_info.chirp_id_s(0), fetched_info.chirp_id_s(0));
  EXPECT_EQ(0, fetched_timestamp->seconds());
  EXPECT_EQ(0, fetched_timestamp->useconds());
}

// Stores a UserInfo without a username should return false
TEST_F(StoreAdapterTest, StoreEmptyInfoShouldFail) {
  UserInfo empty_info;
  EXPECT_FALSE(store_adapter_.StoreUserInfo(empty_info));
}

TEST_F(StoreAdapterTest, GetExistingUsernameShouldWork) {
  // Stores a filled UserInfo
  UserInfo test_info;
  test_info.set_username("test2");
  Timestamp* test_timestamp(test_info.mutable_timestamp());
  test_timestamp->set_seconds(0);
  test_timestamp->set_useconds(0);
  EXPECT_TRUE(store_adapter_.StoreUserInfo(test_info));
  // Get stored userinfo for the id
  UserInfo fetched_info = store_adapter_.GetUserInfo("test2");
  Timestamp* fetched_timestamp(fetched_info.mutable_timestamp());
  EXPECT_EQ(test_info.username(), fetched_info.username());
  EXPECT_EQ(0, fetched_timestamp->seconds());
  EXPECT_EQ(0, fetched_timestamp->useconds());
}

TEST_F(StoreAdapterTest, GetInvalidUsernameShouldReturnEmptyUserInfo) {
  EXPECT_EQ("", store_adapter_.GetUserInfo("").username());
  EXPECT_EQ("", store_adapter_.GetUserInfo("not_registered_user").username());
}

// `StoreChirp` should store the full Chirp into store server
TEST_F(StoreAdapterTest, StoreChirpShouldWork) {
  Chirp chirp = makeChirp("test", "test", "1", "2");
  EXPECT_TRUE(store_adapter_.StoreChirp(chirp));
  // Get stored chirp for the id
  Chirp fetched_chirp = store_adapter_.GetChirp("1");
  Timestamp* fetched_timestamp(fetched_chirp.mutable_timestamp());
  EXPECT_EQ(chirp.username(), fetched_chirp.username());
  EXPECT_EQ(chirp.text(), fetched_chirp.text());
  EXPECT_EQ(chirp.id(), fetched_chirp.id());
  EXPECT_EQ(chirp.parent_id(), fetched_chirp.parent_id());
  EXPECT_EQ(0, fetched_timestamp->seconds());
  EXPECT_EQ(0, fetched_timestamp->useconds());
}

// `StoreChirp` should return false for Chirp without id
TEST_F(StoreAdapterTest, StoreEmptyChirpShouldFail) {
  Chirp invalid_chirp = makeChirp("test", "test", "", "2");
  EXPECT_FALSE(store_adapter_.StoreChirp(invalid_chirp));
}

// Using `GetChirpThread` should return the thread of chirps
TEST_F(StoreAdapterTest, GetExistingChirpThreadShouldSucceed) {
  // Basic Linear Shpae 255->256
  Chirp parent_chirp = makeChirp("test", "parent", "255", "");
  ASSERT_TRUE(store_adapter_.StoreChirp(parent_chirp));
  Chirp child_chirp1 = makeChirp("test", "child", "256", "255");
  ASSERT_TRUE(store_adapter_.StoreChirp(child_chirp1));
  std::vector<Chirp> chirps1 = store_adapter_.GetChirpThread("255");
  ASSERT_EQ(2, chirps1.size());
  EXPECT_EQ("255", chirps1[0].id());
  EXPECT_EQ("256", chirps1[1].id());

  // Basic Tree 255 -> 256
  //                -> 257
  Chirp child_chirp2 = makeChirp("test", "child", "257", "255");
  ASSERT_TRUE(store_adapter_.StoreChirp(child_chirp2));
  std::vector<Chirp> chirps2 = store_adapter_.GetChirpThread("255");
  ASSERT_EQ(3, chirps2.size());
  EXPECT_EQ("255", chirps2[0].id());
  EXPECT_EQ("256", chirps2[1].id());
  EXPECT_EQ("257", chirps2[2].id());
}

// Using `GetChirpThread` with not existing chirp_id should return empty vector
TEST_F(StoreAdapterTest, GetNotExistingChirpThreadShouldReturnEmptyVector) {
  std::vector<Chirp> chirps1 = store_adapter_.GetChirpThread("invalid_id");
  EXPECT_EQ(0, chirps1.size());
}

// `GetChirp` should return the Chirp with valid chirp_id
TEST_F(StoreAdapterTest, GetExistingChirpShouldWork) {
  Chirp chirp = makeChirp("test", "test", "1", "2");
  EXPECT_TRUE(store_adapter_.StoreChirp(chirp));
  // Get stored chirp for the id
  Chirp fetched_chirp = store_adapter_.GetChirp("1");
  Timestamp* fetched_timestamp(fetched_chirp.mutable_timestamp());
  EXPECT_EQ(chirp.username(), fetched_chirp.username());
  EXPECT_EQ(chirp.text(), fetched_chirp.text());
  EXPECT_EQ(chirp.id(), fetched_chirp.id());
  EXPECT_EQ(chirp.parent_id(), fetched_chirp.parent_id());
  EXPECT_EQ(0, fetched_timestamp->seconds());
  EXPECT_EQ(0, fetched_timestamp->useconds());
}

// `GetChirp` should return a empty chirp for invalid chirp_id
TEST_F(StoreAdapterTest, GetInvalidChirpShoulReturnEmptyChirp) {
  Chirp empty_id_chirp = store_adapter_.GetChirp("");
  EXPECT_EQ("", empty_id_chirp.id());
  Chirp invalid_id_chirp = store_adapter_.GetChirp("invalid_id");
  EXPECT_EQ("", invalid_id_chirp.id());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}