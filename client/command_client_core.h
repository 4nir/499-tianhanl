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
// handles fetching and distplying data
class CommandClientCore {
 public:
  void Run(const std::string& register_user, const std::string& user,
           const std::string& chirp, const std::string& reply,
           const std::string& follow, const std::string& read, bool monitor);

 private:
  // Sends a chirp
  void SendChirp(const std::string& username, const std::string& text,
                 const std::string& parent_id);

  // Reads chirp thread starting from id
  // for 1->2
  //      ->3
  // output message will be 1->2->3 (Preorder)
  void ReadChirpThread(const std::string& id);

  // client instance used to communicate with server
  ServiceLayerClient service_layer_client_;
};

#endif