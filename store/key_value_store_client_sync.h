#ifndef KEY_VALUE_STORE_CLIENT_SYNC
#define KEY_VALUE_STORE_CLIENT_SYNC
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "./dist/key_value_store.grpc.pb.h"
#include "./key_value_store_client_interface.h"

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
using grpc::ClientReaderWriter;
using grpc::Status;

const std::string STORE_SERVER_ADDRESS("0.0.0.0:50000");

// Client interface to communicate with key_value_store_server_sync.
class KeyValueStoreClient : public KeyValueStoreClientInterface {
 public:
  // Initializes the client stub_. Should be called before `Put`, `Get` and
  // `DeleteKey` were called.
  void Init() override;

  // Puts key-value pair into store, return true if succeed
  bool Put(const std::string& key, const std::string& value) override;

  // Gets each item for key in keys. `hanle_response` will be called each time
  // `Get` receives a new reponse from stream.
  bool Get(const std::vector<std::string>& keys,
           std::function<void(std::string)> handle_response) override;

  // Deletes the key-value pair in store for the key, return true if succeed
  bool DeleteKey(const std::string& key) override;

 private:
  // Makes a get request object with the key
  GetRequest MakeGetRequest(const std::string& key);
  //  Interface for RPCs to call
  std::unique_ptr<KeyValueStore::Stub> stub_;
};
#endif