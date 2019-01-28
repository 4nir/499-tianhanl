#include <iostream>
#include <memory>
#include <string>

#include "./dist/service_layer.grpc.pb.h"
#include "store_adapter.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

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
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

const std::string SERVICE_SERVER_ADDRESS("0.0.0.0:50002");

class ServiceLayerServiceImpl final : public ServiceLayer::Service {
 public:
  ServiceLayerServiceImpl() {
    store_adapter_ = std::unique_ptr<StoreAdapter>(new StoreAdapter);
  }
  // Registers the given non-blank username
  Status registeruser(ServerContext* context, const RegisterRequest* request,
                      const RegisterReply* reply) {}
  // Posts a new chirp (optionally as a reply)
  Status chirp(ServerContext* context, const ChirpRequest* request,
               const ChirpReply* reply) {}
  // Starts following a given user
  Status follow(ServerContext* context, const FollowRequest* request,
                FollowReply* response) {}
  // Reads a chirp thread from the given id
  Status read(ServerContext* context, const ReadRequest* request,
              ReadReply* response) {}
  // Streams chirps from all followed users
  Status monitor(ServerContext* context, const MonitorRequest* request,
                 ServerWriter<MonitorReply>* writer) {}

 private:
  //  Interface to communicate with store server
  std::unique_ptr<StoreAdapter> store_adapter_;
};

void RunServer() {
  ServiceLayerServiceImpl service;
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(STORE_SERVER_ADDRESS,
                           grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}