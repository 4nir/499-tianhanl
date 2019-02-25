#ifndef COMMAND_CLIENT_CORE
#define COMMAND_CLIENT_CORE

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include "../server/service_layer_client_sync.h"

using std::cout;
using std::endl;

// A core implements command client's features for easier testing
// handles fetching and distplying data
class CommandClientCore {
 public:
  //  Only one operation will be run at each invocation
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

  // Print formatted chirp content to cout
  void PrintChirp(Chirp chirp);

  // client instance used to communicate with server
  ServiceLayerClient service_layer_client_;
};

#endif