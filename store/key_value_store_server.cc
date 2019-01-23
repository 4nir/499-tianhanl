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
/*
  modified based on hello world async exmaple
*/

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "key_value_store_call_data.cc"
#include "store.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include "./dist/key_value_store.grpc.pb.h"

using chirp::KeyValueStore;
using chirp::PutReply;
using chirp::PutRequest;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

class KeyValueStoreServerImpl final {
 public:
  KeyValueStoreServerImpl() { store_ = new Store; }
  ~KeyValueStoreServerImpl() {
    server_->Shutdown();
    cq_->Shutdown();
    delete store_;
  }

  void Run() {
    std::string server_address("0.0.0.0:50000");
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;
    // Proceed to the server's main loop.
    HandleRpcs();
  }

 private:
  void HandleRpcs() {
    new PutCallData(&service_, cq_.get(), store_);
    void *tag;
    bool ok;
    while (true) {
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<BaseCallData *>(tag)->Proceed();
    }
  }
  // The producer-consumer queue where for asynchronous server notifications
  std::unique_ptr<ServerCompletionQueue> cq_;
  // The means of communication with the gRPC runtime for an asynchronous
  // server.
  KeyValueStore::AsyncService service_;
  // Server instance used to manage server related operations
  std::unique_ptr<Server> server_;
  // Store instance used to store key value pairs
  Store *store_;
};

int main(int argc, char **argv) {
  KeyValueStoreServerImpl server;
  server.Run();
  return 0;
}
