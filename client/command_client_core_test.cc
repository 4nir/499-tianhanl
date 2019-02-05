#include "command_client_core.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using ::testing::HasSubstr;

class CommandClientCoreTest : public ::testing::Test {
 protected:
  void SetUp() override {
    default_register_user = "";
    default_user = "";
    default_chirp = "";
    default_reply = "";
    default_follow = "";
    default_read = "";
    default_monitor = false;
  }
  CommandClientCore command_client_core_;
  std::string default_register_user;
  std::string default_user;
  std::string default_chirp;
  std::string default_reply;
  std::string default_follow;
  std::string default_read;
  bool default_monitor;
};

// Users must have usernames for any operation
TEST_F(CommandClientCoreTest, UserMustProvideAnUsername) {
  testing::internal::CaptureStdout();
  command_client_core_.Run(default_register_user, default_user, default_chirp,
                           default_reply, default_follow, default_read,
                           default_monitor);
  std::string output = testing::internal::GetCapturedStdout();
  EXPECT_THAT(output, HasSubstr("username is required"));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}