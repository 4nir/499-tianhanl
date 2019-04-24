#include "service_layer_server_sync.h"

namespace chirpsystem {
// Registers the given non-blank username
Status ServiceLayerServiceImpl::registeruser(ServerContext* context,
                                             const RegisterRequest* request,
                                             RegisterReply* reply) {
  LOG(INFO) << "Received registeruser request for username: "
            << request->username();
  bool ok = service_layer_server_core_.RegisterUser(request->username());
  if (ok) {
    LOG(INFO) << "Registered User: " << request->username();
    return Status::OK;
  }

  LOG(ERROR) << "Cannot register username: " << request->username();
  return Status(StatusCode::INVALID_ARGUMENT,
                "Cannot register username: " + request->username());
}

// Posts a new chirp (optionally as a reply)
Status ServiceLayerServiceImpl::chirp(ServerContext* context,
                                      const ChirpRequest* request,
                                      ChirpReply* reply) {
  LOG(INFO) << "Received chirp request with following information: "
            << "Username: " << request->username() << "\n"
            << "Text: " << request->text() << "\n"
            << "Parent ID: " << request->parent_id() << "\n";
  Chirp chirp;
  ServiceLayerServerCore::ChirpStatus chirp_status =
      service_layer_server_core_.SendChirp(
          chirp, request->username(), request->text(), request->parent_id());
  // return `Status` according to `chirp_status`
  switch (chirp_status) {
    case ServiceLayerServerCore::CHIRP_SUCCEED: {
      LOG(INFO) << "Chirp succeed";
      CloneChirp(chirp, reply->mutable_chirp());
      return Status::OK;
    }
    case ServiceLayerServerCore::CHIRP_FAILED: {
      LOG(ERROR) << "Cannot store chirp with givien input";
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Cannot store chirp with given input");
    }
    case ServiceLayerServerCore::UPDATE_USER_FAILED: {
      LOG(ERROR) << "Cannot associate chirp with current user";
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
Status ServiceLayerServiceImpl::follow(ServerContext* context,
                                       const FollowRequest* request,
                                       FollowReply* response) {
  LOG(INFO) << "Received follow request for username: " << request->username()
            << " to follow: " << request->to_follow();
  bool ok = service_layer_server_core_.Follow(request->username(),
                                              request->to_follow());
  if (ok) {
    return Status::OK;
  }

  LOG(ERROR) << "Cannot complete follow for: " << request->username()
             << " to follow: " << request->to_follow();
  return Status(StatusCode::INVALID_ARGUMENT, "Cannot follow with given input");
}

// Reads a chirp thread for the given id
// Thread order will from curr_chirp_id to its replies:
// Chirp(curr_chirp_id)->Chirp(reply_id)
Status ServiceLayerServiceImpl::read(ServerContext* context,
                                     const ReadRequest* request,
                                     ReadReply* response) {
  LOG(INFO) << "Received read request for id: " << request->chirp_id();
  std::vector<Chirp> chirp_thread =
      service_layer_server_core_.Read(request->chirp_id());
  if (chirp_thread.size() < 1) {
    LOG(ERROR) << "Cannot read chirps for: " << request->chirp_id();
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
Status ServiceLayerServiceImpl::monitor(ServerContext* context,
                                        const MonitorRequest* request,
                                        ServerWriter<MonitorReply>* writer) {
  LOG(INFO) << "Received monitor request for user: " << request->username();
  bool monitor_ok = service_layer_server_core_.Monitor(
      request->username(), [writer, this](Chirp chirp) {
        MonitorReply reply;
        CloneChirp(chirp, reply.mutable_chirp());
        // If stream is closed, terminate thread
        return writer->Write(reply);
      });

  if (monitor_ok) {
    return Status::OK;
  }

  LOG(INFO) << "Cannot monitor for user: " << request->username();
  return Status(StatusCode::INVALID_ARGUMENT,
                "Cannot monitor for current user");
}

Status ServiceLayerServiceImpl::stream(ServerContext* context,
                                        const StreamRequest* request,
                                        ServerWriter<StreamReply>* writer) {
  LOG(INFO) << "Received stream request for user: " << request->hashtag();
  bool stream_ok = service_layer_server_core_.Stream(
      request->hashtag(), [writer, this](Chirp chirp) {
        StreamReply reply;
        CloneChirp(chirp, reply.mutable_chirp());
        // If stream is closed, terminate thread
        return writer->Write(reply);
      });

  if (stream_ok) {
    return Status::OK;
  }

  LOG(INFO) << "Cannot stream for hashtag: " << request->hashtag();
  return Status(StatusCode::INVALID_ARGUMENT,
                "Cannot stream for current user");
  
}
// Clones the content of chirp into mutable_chirp_pointer
void ServiceLayerServiceImpl::CloneChirp(const Chirp& chirp,
                                         Chirp* mutable_chirp) {
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

void RunServer() {
  ServiceLayerServiceImpl service;
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(kSERVICE_SERVER_ADDRESS,
                           grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << kSERVICE_SERVER_ADDRESS << std::endl;
  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}
}  // namespace chirpsystem

int main(int argc, char** argv) {
  FLAGS_log_dir = "./";
  FLAGS_alsologtostderr = 2;
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Service Layer Server started \n";
  chirpsystem::RunServer();
  return 0;
}