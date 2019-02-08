#include <algorithm>
#include <chrono>
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
      CloneChirp(chirp, reply->mutable_chirp());
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

  // Reads a chirp thread for the given id
  // Thread order will from curr_chirp_id to its replies:
  // Chirp(curr_chirp_id)->Chirp(reply_id)
  Status read(ServerContext* context, const ReadRequest* request,
              ReadReply* response) {
    std::vector<Chirp> chirp_thread =
        store_adapter_->GetChirpThread(request->chirp_id());
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

  // Streams chirps from all followed users
  Status monitor(ServerContext* context, const MonitorRequest* request,
                 ServerWriter<MonitorReply>* writer) {
    UserInfo curr_user_info = store_adapter_->GetUserInfo(request->username());
    // Starts monitoring
    std::thread monitoring([writer, &curr_user_info, this]() {
      std::vector<Chirp> chirps;
      // Mark latest_access_seconds, only post younger than it should be sent
      auto mark_time = system_clock::now().time_since_epoch();
      int latest_access_seconds = (duration_cast<seconds>(mark_time)).count();
      // Uses polling to check updates.
      while (true) {
        std::vector<Chirp> chirps;
        // Store curent access time
        int mark_seconds = latest_access_seconds;
        // wait 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(2));
        // If the users who current user is following has newer record,
        // store the new chirp to chiprs
        for (const std::string& username : curr_user_info.following()) {
          UserInfo user_info = store_adapter_->GetUserInfo(username);
          // If user record has been updated after mark_time
          if (user_info.timestamp().seconds() > mark_seconds) {
            // Adds chirp posted after mark_time to chirps
            for (int i = user_info.chirp_id_s_size() - 1; i >= 0; i--) {
              Chirp chirp = store_adapter_->GetChirp(user_info.chirp_id_s(i));
              // If the chirp is posted after mark_seconds, it is a new
              // chirp
              if (chirp.timestamp().seconds() > mark_seconds) {
                latest_access_seconds = std::max(
                    latest_access_seconds, int(chirp.timestamp().seconds()));
                chirps.push_back(chirp);
              } else {
                break;
              }
            }
          }
        }
        // Now chirps have all chirps posted after mark time
        for (Chirp chirp : chirps) {
          MonitorReply reply;
          CloneChirp(chirp, reply.mutable_chirp());
          bool ok = writer->Write(reply);
          // If stream is closed, terminate thread
          if (!ok) {
            return;
          }
        }
      }
    });
    // Wait for monitoring to end
    monitoring.join();
    return Status::OK;
  }

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