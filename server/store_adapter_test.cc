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
  EXPECT_TRUE(store_adapter_.StoreUserInfo(test_info));
  UserInfo fetched_info = store_adapter_.GetUserInfo("test");
  EXPECT_EQ(test_info.username(), fetched_info.username());
  EXPECT_EQ(test_info.following(0), fetched_info.following(0));
  EXPECT_EQ(test_info.chirp_id_s(0), fetched_info.chirp_id_s(0));
}

// `StoreChirp` should store the Chirp into store server
TEST_F(StoreAdapterTest, StoreChirpShouldStore) {
  Chirp chirp;
  chirp.set_username("test");
  chirp.set_text("test");
  chirp.set_id("2");
  chirp.set_parent_id("1");
  EXPECT_TRUE(store_adapter_.StoreChirp(chirp));
  Chirp fetched_chirp = store_adapter_.GetChirp("2");
  EXPECT_EQ(chirp.username(), fetched_chirp.username());
  EXPECT_EQ(chirp.text(), fetched_chirp.text());
  EXPECT_EQ(chirp.id(), fetched_chirp.id());
  EXPECT_EQ(chirp.parent_id(), fetched_chirp.parent_id());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}