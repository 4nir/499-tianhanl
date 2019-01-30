#ifndef SERVICE_LAYER_CLIENT_SYNC
#define SERVICE_LAYER_CLIENT_SYNC

#include <iostream>
#include <memory>
#include <string>

#include "./dist/service_layer.grpc.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using chirp::FollowReply;
using chirp::FollowRequest;
using chirp::RegisterReply;
using chirp::RegisterRequest;
using chirp::ServiceLayer;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::Status;

const std::string SERVICE_SERVER_ADDRESS("0.0.0.0:50002");

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

 private:
  // Interface for RPC calls
  std::unique_ptr<ServiceLayer::Stub> stub_;
};

#endif