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

#include <iostream>
#include <memory>
#include <string>
#include "store.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include "./dist/key_value_store.grpc.pb.h"

using chirp::DeleteReply;
using chirp::DeleteRequest;
using chirp::GetReply;
using chirp::GetRequest;
using chirp::KeyValueStore;
using chirp::PutReply;
using chirp::PutRequest;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using grpc::StatusCode;

const std::string STORE_SERVER_ADDRESS("0.0.0.0:50000");

class KeyValueStoreServiceImpl final : public KeyValueStore::Service {
 public:
  KeyValueStoreServiceImpl() { store_ = new Store; }
  ~KeyValueStoreServiceImpl() { delete store_; }

  // Stores key and value specified in request, return
  // `StatusCode::INVALID_ARGUMENT` when supplied key and value cannot be
  // stored.
  Status put(ServerContext* context, const PutRequest* request,
             PutReply* reply) override {
    const std::string& key = request->key();
    const std::string& value = request->value();
    bool ok = store_->Put(key, value);
    if (ok) {
      return Status::OK;
    } else {
      return Status(StatusCode::INVALID_ARGUMENT, "Unable to put");
    }
  }

  Status get(ServerContext* context,
             ServerReaderWriter<GetReply, GetRequest>* stream) override {
    GetRequest request;
    while (stream->Read(&request)) {
      const std::string& key = request.key();
      const std::string& value = store_->Get(key);
      GetReply reply;
      reply.set_value(value);
      stream->Write(reply);
    }
    return Status::OK;
  }

  // Deletes specified keyed item in store if exist.
  Status deletekey(ServerContext* context, const DeleteRequest* request,
                   DeleteReply* reply) override {
    const std::string& key = request->key();
    bool ok = store_->Remove(key);
    if (ok) {
      return Status::OK;
    } else {
      return Status(StatusCode::INVALID_ARGUMENT, "Key is not exist in store");
    }
  }

 private:
  // Store instance used to store key value pairs
  Store* store_;
};

void RunServer() {
  KeyValueStoreServiceImpl service;
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(STORE_SERVER_ADDRESS,
                           grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}