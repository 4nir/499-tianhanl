#include "store_adapter.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

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

// Using `GetChirpThread` should return the thread of chirps
TEST_F(StoreAdapterTest, GetChirpThreadShouldSucceed) {
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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}