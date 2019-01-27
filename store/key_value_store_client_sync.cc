#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "./dist/key_value_store.grpc.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using chirp::DeleteReply;
using chirp::DeleteRequest;
using chirp::GetReply;
using chirp::GetRequest;
using chirp::KeyValueStore;
using chirp::PutReply;
using chirp::PutRequest;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

const std::string STORE_SERVER_ADDRESS("0.0.0.0:50000");

class KeyValueStoreClient {
 public:
  KeyValueStoreClient(std::shared_ptr<Channel> channel)
      : stub_(KeyValueStore::NewStub(channel)) {}

  bool Put(const std::string& key, const std::string& value) {
    // Set up container for data to store
    PutRequest request;
    request.set_key(key);
    request.set_value(value);

    // Set up container for server reponse
    PutReply reply;

    // Context for the client
    ClientContext context;

    // Calling put RPC
    Status status = stub_->put(&context, request, &reply);

    return status.ok();
  }

  bool Get(const std::vector<std::string>& keys,
           std::function<void(std::string)> handle_response) {
    ClientContext context;
    // Stream used to communicate with server
    std::shared_ptr<ClientReaderWriter<GetRequest, GetReply>> stream(
        stub_->get(&context));

    // Send request for each key to server in a separate thread
    std::thread writer([stream, keys, this]() {
      for (const std::string& key : keys) {
        stream->Write(MakeGetRequest(key));
      }
      stream->WritesDone();
    });

    GetReply reply;
    while (stream->Read(&reply)) {
      handle_response(reply.value());
    }
    writer.join();
    Status status = stream->Finish();
    return status.ok();
  }

  bool DeleteKey(const std::string& key) {
    // Set up container for key to delete
    DeleteRequest request;
    request.set_key(key);

    // Set up container for server response
    DeleteReply reply;

    ClientContext context;
    Status status = stub_->deletekey(&context, request, &reply);
    return status.ok();
  }

 private:
  GetRequest MakeGetRequest(const std::string& key) {
    GetRequest request;
    request.set_key(key);
    return request;
  }
  //  Interface for RPCs to call
  std::unique_ptr<KeyValueStore::Stub> stub_;
};

int main(int argc, char** argv) {
  KeyValueStoreClient store(grpc::CreateChannel(
      STORE_SERVER_ADDRESS, grpc::InsecureChannelCredentials()));
}