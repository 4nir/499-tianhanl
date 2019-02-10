#include "key_value_store_client_sync.h"

void KeyValueStoreClient::Init() {
  this->stub_ = KeyValueStore::NewStub(grpc::CreateChannel(
      STORE_SERVER_ADDRESS, grpc::InsecureChannelCredentials()));
}

bool KeyValueStoreClient::Put(const std::string& key,
                              const std::string& value) {
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

bool KeyValueStoreClient::Get(
    const std::vector<std::string>& keys,
    const std::function<void(std::string)>& handle_response) {
  ClientContext context;
  // Stream used to communicate with server
  std::shared_ptr<ClientReaderWriter<GetRequest, GetReply>> stream(
      stub_->get(&context));

  // Send request for each key to server in a separate thread
  std::thread writer([stream, &keys, this]() {
    for (const std::string& key : keys) {
      stream->Write(MakeGetRequest(key));
    }
    stream->WritesDone();
  });

  // Read response from server and call handle_reponse with retrived value
  GetReply reply;
  while (stream->Read(&reply)) {
    handle_response(reply.value());
  }

  // writer should now finished
  writer.join();
  Status status = stream->Finish();
  return status.ok();
}

bool KeyValueStoreClient::DeleteKey(const std::string& key) {
  // Set up container for key to delete
  DeleteRequest request;
  request.set_key(key);

  // Set up container for server response
  DeleteReply reply;

  ClientContext context;
  Status status = stub_->deletekey(&context, request, &reply);
  return status.ok();
}

GetRequest KeyValueStoreClient::MakeGetRequest(const std::string& key) {
  GetRequest request;
  request.set_key(key);
  return request;
}