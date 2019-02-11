#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "./dist/service_layer.grpc.pb.h"
#include "service_layer_server_core.h"

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
using chirp::UserInfo;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;
using namespace std::chrono;

const std::string SERVICE_SERVER_ADDRESS("0.0.0.0:50002");

// Implementation of ServiceLayerService
class ServiceLayerServiceImpl final : public ServiceLayer::Service {
 public:
  ServiceLayerServiceImpl() { service_layer_server_core_.Init(); }
  // Registers the given non-blank username
  Status registeruser(ServerContext* context, const RegisterRequest* request,
                      RegisterReply* reply) override {
    bool ok = service_layer_server_core_.RegisterUser(request->username());
    if (ok) {
      return Status::OK;
    } else {
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Cannot register with given username");
    }
  }

  // Posts a new chirp (optionally as a reply)
  Status chirp(ServerContext* context, const ChirpRequest* request,
               ChirpReply* reply) {
    Chirp chirp;
    ServiceLayerServerCore::ChirpStatus chirp_status =
        service_layer_server_core_.SendChirp(
            chirp, request->username(), request->text(), request->parent_id());
    // return `Status` according to `chirp_status`
    switch (chirp_status) {
      case ServiceLayerServerCore::CHIRP_SUCCEED: {
        CloneChirp(chirp, reply->mutable_chirp());
        return Status::OK;
      }
      case ServiceLayerServerCore::CHIRP_FAILED: {
        return Status(StatusCode::INVALID_ARGUMENT,
                      "Cannot store chirp with given input");
      }
      case ServiceLayerServerCore::UPDATE_USER_FAILED: {
        return Status(StatusCode::INVALID_ARGUMENT,
                      "Cannot associate chirp with current user");
      }
      default: {
        // `crhip_status` should only be one of the ChirpStatus
        assert(false);
      }
    }
  }
  // Starts following a given user
  Status follow(ServerContext* context, const FollowRequest* request,
                FollowReply* response) override {
    bool ok = service_layer_server_core_.Follow(request->username(),
                                                request->to_follow());
    if (ok) {
      return Status::OK;
    } else {
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Cannot follow with given input");
    }
  }

  // Reads a chirp thread for the given id
  // Thread order will from curr_chirp_id to its replies:
  // Chirp(curr_chirp_id)->Chirp(reply_id)
  Status read(ServerContext* context, const ReadRequest* request,
              ReadReply* response) {
    std::vector<Chirp> chirp_thread =
        service_layer_server_core_.Read(request->chirp_id());
    if (chirp_thread.size() < 1) {
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Cannot find corresponding chirp");
    }
    for (Chirp chirp : chirp_thread) {
      Chirp* mutable_chirp = response->add_chirps();
      CloneChirp(chirp, mutable_chirp);
    }
    return Status::OK;
  }

  /*
   Streams chirps from all followed users
   The order of reponse will from oldest chirp to latest chirp
  */
  Status monitor(ServerContext* context, const MonitorRequest* request,
                 ServerWriter<MonitorReply>* writer) {
    bool monitor_ok = service_layer_server_core_.Monitor(
        request->username(), [writer, this](Chirp chirp) {
          MonitorReply reply;
          CloneChirp(chirp, reply.mutable_chirp());
          // If stream is closed, terminate thread
          return writer->Write(reply);
        });
    if (monitor_ok) {
      return Status::OK;
    } else {
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Cannot monitor for current user");
    }
  }

 private:
  // Clones the content of chirp into mutable_chirp_pointer
  void CloneChirp(Chirp chirp, Chirp* mutable_chirp) {
    Timestamp* timestamp = new Timestamp;
    timestamp->set_seconds(chirp.timestamp().seconds());
    timestamp->set_useconds(chirp.timestamp().useconds());
    mutable_chirp->set_text(chirp.text());
    mutable_chirp->set_id(chirp.id());
    mutable_chirp->set_parent_id(chirp.parent_id());
    mutable_chirp->set_username(chirp.username());
    // Ownership of timestamp transfered to mutable_chirp
    mutable_chirp->set_allocated_timestamp(timestamp);
  }

  // core that implements the actual functions of the server
  ServiceLayerServerCore service_layer_server_core_;
};

void RunServer() {
  ServiceLayerServiceImpl service;
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(SERVICE_SERVER_ADDRESS,
                           grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << SERVICE_SERVER_ADDRESS << std::endl;
  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}