/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "key_value_store_server_sync.h"

namespace chirpsystem {

Status KeyValueStoreServiceImpl::put(ServerContext* context,
                                     const PutRequest* request,
                                     PutReply* reply) {
  const std::string& key = request->key();
  const std::string& value = request->value();
  LOG(INFO) << "Received put request for key: " << key << " value: " << value;
  bool ok = store_.Put(key, value);
  if (ok) {
    return Status::OK;
  }

  return Status(StatusCode::INVALID_ARGUMENT, "Unable to put");
}

Status KeyValueStoreServiceImpl::get(
    ServerContext* context, ServerReaderWriter<GetReply, GetRequest>* stream) {
  LOG(INFO) << "Received get request:";
  GetRequest request;
  while (stream->Read(&request)) {
    GetReply reply;
    const std::string& key = request.key();
    LOG(INFO) << "Getting key: " << key;
    std::optional<std::string> value = store_.Get(key);
    if (value) {
      LOG(INFO) << "Value for " << key << " is: " << *value;
      reply.set_value(*value);
    } else {
      LOG(INFO) << "No value for key " << key;
      reply.set_value("");
    }
    stream->Write(reply);
  }
  return Status::OK;
}

Status KeyValueStoreServiceImpl::deletekey(ServerContext* context,
                                           const DeleteRequest* request,
                                           DeleteReply* reply) {
  LOG(INFO) << "Received delete request for key: " << request->key();
  const std::string& key = request->key();
  bool ok = store_.Remove(key);
  if (ok) {
    return Status::OK;
  }
  return Status(StatusCode::INVALID_ARGUMENT, "Key is not exist in store");
}

void RunServer() {
  KeyValueStoreServiceImpl service;
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(kSTORE_SERVER_ADDRESS,
                           grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << kSTORE_SERVER_ADDRESS << std::endl;
  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}
}  // namespace chirpsystem

int main(int argc, char** argv) {
  // set up glog
  FLAGS_log_dir = "./";
  FLAGS_alsologtostderr = 2;
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Key Value Store Server started \n";
  chirpsystem::RunServer();
  return 0;
}