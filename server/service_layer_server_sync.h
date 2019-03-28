#include "service_layer_server_core.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <glog/logging.h>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include "dist/service_layer.grpc.pb.h"

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
using chirp::Timestamp;
using chirp::UserInfo;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;
using namespace std::chrono;

namespace chirpsystem {
const std::string kSERVICE_SERVER_ADDRESS("0.0.0.0:50002");

// Implementation of ServiceLayerService
class ServiceLayerServiceImpl final : public ServiceLayer::Service {
 public:
  ServiceLayerServiceImpl() { service_layer_server_core_.Init(); }
  // Registers the given non-blank username
  Status registeruser(ServerContext* context, const RegisterRequest* request,
                      RegisterReply* reply) override;

  // Posts a new chirp (optionally as a reply)
  Status chirp(ServerContext* context, const ChirpRequest* request,
               ChirpReply* reply) override;
  // Starts following a given user
  Status follow(ServerContext* context, const FollowRequest* request,
                FollowReply* response) override;

  // Reads a chirp thread for the given id
  // Thread order will from curr_chirp_id to its replies:
  // Chirp(curr_chirp_id)->Chirp(reply_id)
  Status read(ServerContext* context, const ReadRequest* request,
              ReadReply* response);

  /*
   Streams chirps from all followed users
   The order of reponse will from oldest chirp to latest chirp
  */
  Status monitor(ServerContext* context, const MonitorRequest* request,
                 ServerWriter<MonitorReply>* writer) override;

 private:
  // Clones the content of chirp into mutable_chirp_pointer
  void CloneChirp(const Chirp& chirp, Chirp* mutable_chirp);

  // core that implements the actual functions of the server
  ServiceLayerServerCore service_layer_server_core_;
};
};  // namespace chirpsystem