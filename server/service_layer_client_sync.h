#ifndef SERVICE_LAYER_CLIENT_SYNC
#define SERVICE_LAYER_CLIENT_SYNC

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "./dist/service_layer.grpc.pb.h"

using chirp::Chirp;
using chirp::ChirpReply;
using chirp::ChirpRequest;
using chirp::FollowReply;
using chirp::FollowRequest;
using chirp::MonitorReply;
using chirp::MonitorRequest;
using chirp::ReadReply;
using chirp::ReadRequest;
using chirp::RegisterReply;
using chirp::RegisterRequest;
using chirp::ServiceLayer;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

namespace chirpsystem {
const std::string kSERVICE_SERVER_ADDRESS("0.0.0.0:50002");

// Client interface to communicate with service layer server.
class ServiceLayerClient {
 public:
  // Initializes the client stub_. Should be called before all RPC calls
  // methods
  void Init();

  // Registers a new user with given username.
  // Username must be unique.
  bool RegisterUser(const std::string& username);

  // Follows user `to_follow` for user `username`
  bool Follow(const std::string& username, const std::string& to_follow);

  // Creates a `Chirp`, id and timestamp will be supplied by service layer.
  // Returns the id of the stored chirp.
  // If id is "", the request failed.
  // parent_id: If this chirp is an reply, parent_id is the id of the chrip it
  // is replying to.
  std::string SendChirp(const std::string& username, const std::string& text,
                        const std::string& parent_id = "");

  // Reads a chirp thread from the given id.
  // Returns the vector of chirp thread.
  // If vector length is 0, the request failed.
  // Thread order will from curr_chirp_id to its replies
  // Chirp(curr_chirp_id) -> Chirp(reply_id)
  // EX: foo->bar->baz, Read(foo) returns [foo, bar, baz]
  std::vector<Chirp> Read(const std::string& chirp_id);

  // Get new chirps from users who current user is following.
  // When a new chirp is received handle_response will be called with the chirp.
  // The order of reponse will from oldest chirp to latest chirp
  bool Monitor(const std::string& username,
               std::function<void(Chirp)> handle_response);

 private:
  // Interface for RPC calls
  std::unique_ptr<ServiceLayer::Stub> stub_;
};
}  // namespace chirpsystem

#endif