#ifndef KEY_VALUE_STORE_CLIENT_SYNC
#define KEY_VALUE_STORE_CLIENT_SYNC
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
using grpc::ClientReaderWriter;
using grpc::Status;

const std::string STORE_SERVER_ADDRESS("0.0.0.0:50000");

class KeyValueStoreClient {
 public:
  // initilize the client stub_
  void init();

  // Put key-value pair into store, return true if succeed
  bool Put(const std::string& key, const std::string& value);

  // Get each item for key in keys, when server respond  `hanlde_response` will
  // be called with the value server retrived for the key.
  bool Get(const std::vector<std::string>& keys,
           std::function<void(std::string)> handle_response);

  // Delete the key-value pair in store for the key, return true if succeed
  bool DeleteKey(const std::string& key);

 private:
  // Make a get request object with the key
  GetRequest MakeGetRequest(const std::string& key);
  //  Interface for RPCs to call
  std::unique_ptr<KeyValueStore::Stub> stub_;
};
#endif