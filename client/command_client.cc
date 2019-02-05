#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <string>
#include "../server/service_layer_client_sync.h"

using std::cout;
using std::endl;

DEFINE_bool(monitor, false, "Streams new chirps from those currently followed");
DEFINE_string(register, "", "Registers the given username");
DEFINE_string(user, "", "Logs in as the given username");
DEFINE_string(chirp, "", "Creates a new chirp with the given text");
DEFINE_string(reply, "",
              "Indicates that the new chirp is a reply to the given id");
DEFINE_string(follow, "", "Starts following the given username");
DEFINE_string(read, "", "Reads the chirp thread starting at the given id");

int main(int argc, char** argv) {
  // Start gflags and glog
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  // Creates a client instance used to communicate with server
  std::unique_ptr<ServiceLayerClient> service_layer_client(
      new ServiceLayerClient);
  service_layer_client->Init();

  // Retrives current user's username.
  // Clients must have a username to be authorized for all activities
  std::string username = "";
  if (FLAGS_register != "") {
    username = FLAGS_register;
    // Register this username
    bool ok = service_layer_client->RegisterUser(username);
    if (!ok) {
      cout << "username: " << username << " cannot be registered" << endl;
      return 1;
    }
  } else if (FLAGS_user != "") {
    username = FLAGS_user;
  } else {
    cout
        << "A username is required to chirp. Please use --register or --user to"
        << " provide a username." << endl;
    return 1;
  }

  // use must reply with a chirp
  if (FLAGS_reply != "" && FLAGS_chirp == "") {
    cout << "--reply must be used with --chirp" << endl;
  }

  // Creates a chirp if has one.
  if (FLAGS_chirp != "") {
    const std::string& id =
        service_layer_client->SendChirp(username, FLAGS_chirp, FLAGS_reply);
    if (id == "") {
      cout << "fail to chirp with inputted information" << endl;
    } else {
      cout << "chirp id is: " << id << endl;
    }
  }

  // TODO: Add monitor
}