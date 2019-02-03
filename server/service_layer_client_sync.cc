#include "service_layer_client_sync.h"

void ServiceLayerClient::Init() {
  this->stub_ = ServiceLayer::NewStub(grpc::CreateChannel(
      SERVICE_SERVER_ADDRESS, grpc::InsecureChannelCredentials()));
}

bool ServiceLayerClient::RegisterUser(const std::string& username) {
  RegisterRequest request;
  request.set_username(username);
  RegisterReply reply;
  ClientContext context;
  Status status = stub_->registeruser(&context, request, &reply);
  return status.ok();
}

bool ServiceLayerClient::Follow(const std::string& username,
                                const std::string& to_follow) {
  FollowRequest request;
  request.set_username(username);
  request.set_to_follow(to_follow);
  FollowReply reply;
  ClientContext context;
  Status status = stub_->follow(&context, request, &reply);
  return status.ok();
}

bool ServiceLayerClient::Chirp(const std::string& username,
                               const std::string& text,
                               const std::string& parent_id) {
  ChirpRequest request;
  request.set_username(username);
  request.set_text(text);
  request.set_parent_id(parent_id);
  ChirpReply reply;
  ClientContext context;
  Status status = stub_->chirp(&context, request, &reply);
  return status.ok();
}