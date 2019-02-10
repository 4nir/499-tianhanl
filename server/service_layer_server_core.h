#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "./dist/service_layer.grpc.pb.h"
#include "store_adapter.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

using chirp::Chirp;
using chirp::Timestamp;
using chirp::UserInfo;
using grpc::ServerWriter;
using namespace std::chrono;

// Manges the creation of internal data strcutre and the interaction with store.
class ServiceLayerServerCore {
 public:
  // Initialize store_adapter_
  void Init(bool dev = false);

  // Registers the given non-blank username
  bool RegisterUser(const std::string& username);

  // The return type of SendChirp Function
  enum ChirpStatus { CHIRP_SUCCEED, CHIRP_FAILED, UPDATE_USER_FAILED };

  // Posts a new chirp (optionally as a reply) and updating associated
  // user information. `chirp` will be filled with actual content.
  // CHIRP_SUCCEED: chirp is stored, and current user info has been updated
  // CHIRP_FAILED: chirp cannot be stored
  // UPDATE_USER_FAILED: chirp can be stored, but associated user cannot be
  // updated
  ChirpStatus SendChirp(Chirp& chirp, const std::string& username,
                        const std::string& text,
                        const std::string& parent_id = "");

  // Follows `to_follow` for `username`
  bool Follow(const std::string& username, const std::string& to_follow);

  // Reads a chirp thread for the given id
  // Thread order will from curr_chirp_id to its replies:
  // Chirp(curr_chirp_id)->Chirp(reply_id)
  std::vector<Chirp> Read(const std::string id);

  /*
  Streams chirps from all followed users. Current thread will be blocked after
  this function was called.

  Curent implementation workflow:
  1. Check following users' timestamps which indicate their update times.
  2. If a following user has a timestamp larger than mark time, it has been
  updated after last polling.
  3. If the user has been updated, checks if the user has chirps posted after
  mark time.
  4. Set last_access_seconds to avoid read the same chirp again in next
  polling.
  5. Repeat until client ends.

  The order of reponse will from oldest chirp to latest chirp

  username: the user to monitor
  handle_response: callback function that will be called when a new chirp is
  found. If handle_reponse returns false, the polling will be terminated.
  time_limit: Home many senonds after polling starts should monitor end, -1
  means infinite time.
 */
  bool Monitor(const std::string& username,
               const std::function<bool(Chirp)>& handle_response,
               int time_limit = -1);

 private:
  // Creates a Timestamp object populated with current UNIX timestamp.
  // The caller of this function should handle management of returned pointer
  // to timestamp instance.
  Timestamp* MakeCurrentTimestamp();

  // Determine is lhs chirp older than rhs chirp
  static bool Older(Chirp lhs, Chirp rhs) {
    lhs.timestamp().seconds() < rhs.timestamp().seconds();
  }

  // Get current time in seconds
  int GetCurrentTime();

  //  Interface to communicate with store server
  StoreAdapter store_adapter_;
};