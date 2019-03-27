#include "command_client_core.h"

#include <iostream>
#include <memory>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>

using std::cout;
using std::endl;

DEFINE_string(register, "", "Registers the given username");
DEFINE_string(user, "", "Logs in as the given username");
DEFINE_string(chirp, "", "Creates a new chirp with the given text");
DEFINE_string(reply, "",
              "Indicates that the new chirp is a reply to the given id");
DEFINE_string(follow, "", "Starts following the given username");
DEFINE_string(read, "", "Reads the chirp thread starting at the given id");
DEFINE_bool(monitor, false, "Streams new chirps from those currently followed");

int main(int argc, char** argv) {
  // Start gflags and glog
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  CommandClientCore command_client_core;
  command_client_core.Run(FLAGS_register, FLAGS_user, FLAGS_chirp, FLAGS_reply,
                          FLAGS_follow, FLAGS_read, FLAGS_monitor);
}