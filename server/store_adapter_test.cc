#include "store_adapter.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class StoreAdapterTest : public ::testing::Test {
 protected:
  void SetUp() override { store_adapter_.Init(); }
  StoreAdapter store_adapter_;
};

//  `StoreUserInfo` should store the UserInfo into store server
TEST_F(StoreAdapterTest, StoreUserInfoShouldStore) {
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

// `StoreChirp` should store the Chirp into store server
TEST_F(StoreAdapterTest, StoreChirpShouldStore) {
  Chirp chirp;
  chirp.set_username("test");
  chirp.set_text("test");
  chirp.set_id("2");
  chirp.set_parent_id("1");
  Timestamp* test_timestamp(chirp.mutable_timestamp());
  test_timestamp->set_seconds(0);
  test_timestamp->set_useconds(0);
  EXPECT_TRUE(store_adapter_.StoreChirp(chirp));
  // Get stored chirp for the id
  Chirp fetched_chirp = store_adapter_.GetChirp("2");
  Timestamp* fetched_timestamp(fetched_chirp.mutable_timestamp());
  EXPECT_EQ(chirp.username(), fetched_chirp.username());
  EXPECT_EQ(chirp.text(), fetched_chirp.text());
  EXPECT_EQ(chirp.id(), fetched_chirp.id());
  EXPECT_EQ(chirp.parent_id(), fetched_chirp.parent_id());
  EXPECT_EQ(0, fetched_timestamp->seconds());
  EXPECT_EQ(0, fetched_timestamp->useconds());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}