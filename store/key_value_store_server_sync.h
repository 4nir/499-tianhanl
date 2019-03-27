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
#include <optional>
#include <string>

#include <glog/logging.h>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include "dist/key_value_store.grpc.pb.h"
#include "store.h"

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

namespace chirpsystem {
const std::string kSTORE_SERVER_ADDRESS("0.0.0.0:50000");

class KeyValueStoreServiceImpl final : public KeyValueStore::Service {
 public:
  // Stores key and value specified in request, return
  // `StatusCode::INVALID_ARGUMENT` when supplied key and value cannot be
  // stored.
  Status put(ServerContext* context, const PutRequest* request,
             PutReply* reply) override;

  // Bidirectional streaming key and value
  Status get(ServerContext* context,
             ServerReaderWriter<GetReply, GetRequest>* stream) override;

  // Deletes specified keyed item in store if exist.
  Status deletekey(ServerContext* context, const DeleteRequest* request,
                   DeleteReply* reply) override;

 private:
  // Store instance used to store key value pairs
  Store store_;
};
}  // namespace chirpsystem