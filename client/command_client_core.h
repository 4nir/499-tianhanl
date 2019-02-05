#ifndef COMMAND_CLIENT_CORE
#define COMMAND_CLIENT_CORE

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../server/service_layer_client_sync.h"

using std::cout;
using std::endl;

// A core implements command client's features for easier testing
class CommandClientCore {
 public:
  void Run(const std::string& register_user, const std::string& user,
           const std::string& chirp, const std::string& reply,
           const std::string& follow, const std::string& read, bool monitor);

 private:
  // client instance used to communicate with server
  ServiceLayerClient service_layer_client_;
};

#endif