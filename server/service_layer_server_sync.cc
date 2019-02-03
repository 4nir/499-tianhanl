#include <chrono>
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
  ServiceLayerServiceImpl() {
    store_adapter_ = std::unique_ptr<StoreAdapter>(new StoreAdapter);
    store_adapter_->Init();
  }

  // Registers the given non-blank username
  Status registeruser(ServerContext* context, const RegisterRequest* request,
                      RegisterReply* reply) override {
    UserInfo new_user_info;
    new_user_info.set_username(request->username());
    // Ownership of timestamp pointer is transferred to new_user_info
    new_user_info.set_allocated_timestamp(MakeCurrentTimestamp());
    bool ok = store_adapter_->StoreUserInfo(new_user_info);
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
    // Prepares chirp
    Chirp chirp;
    Timestamp* current_timestamp = MakeCurrentTimestamp();
    chirp.set_username(request->username());
    chirp.set_text(request->text());
    chirp.set_parent_id(request->parent_id());
    chirp.set_id(request->username() +
                 std::to_string(current_timestamp->seconds()));
    chirp.set_allocated_timestamp(current_timestamp);

    bool chirp_ok = store_adapter_->StoreChirp(chirp);

    // When chirp is stored, updates corresponding UserInfo for monitoring
    if (chirp_ok) {
      UserInfo curr_user_info =
          store_adapter_->GetUserInfo(request->username());
      curr_user_info.add_chirp_id_s(chirp.id());
      curr_user_info.set_allocated_timestamp(MakeCurrentTimestamp());
      bool user_ok = store_adapter_->StoreUserInfo(curr_user_info);
      if (user_ok) {
        return Status::OK;
      } else {
        return Status(StatusCode::INVALID_ARGUMENT,
                      "Cannot associate chirp with current user");
      }
    } else {
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Cannot store chirp with given input");
    }
  }

  // Starts following a given user
  Status follow(ServerContext* context, const FollowRequest* request,
                FollowReply* response) override {
    UserInfo current_user_info =
        store_adapter_->GetUserInfo(request->username());
    current_user_info.add_following(request->to_follow());
    // update last modified time
    current_user_info.set_allocated_timestamp(MakeCurrentTimestamp());
    bool ok = store_adapter_->StoreUserInfo(current_user_info);
    if (ok) {
      return Status::OK;
    } else {
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Cannot follow with given input");
    }
  }

  // Reads a chirp thread from the given id
  Status read(ServerContext* context, const ReadRequest* request,
              ReadReply* response) {}
  // Streams chirps from all followed users
  Status monitor(ServerContext* context, const MonitorRequest* request,
                 ServerWriter<MonitorReply>* writer) {}

 private:
  // Creates a Timestamp object populated with current UNIX timestamp.
  // The caller of this function should handle management of returned pointer
  // to timestamp instance.
  Timestamp* MakeCurrentTimestamp() {
    Timestamp* timestamp = new Timestamp;
    auto current_time = system_clock::now().time_since_epoch();
    timestamp->set_seconds((duration_cast<seconds>(current_time)).count());
    timestamp->set_useconds(
        (duration_cast<microseconds>(current_time)).count());
    return timestamp;
  }
  //  Interface to communicate with store server
  std::unique_ptr<StoreAdapter> store_adapter_;
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