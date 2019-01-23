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
  This file is adapted from greeter_async_client.cc's structure
*/

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "./dist/key_value_store.grpc.pb.h"

using chirp::KeyValueStore;
using chirp::PutReply;
using chirp::PutRequest;
using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

const std::string SERVER_ADDRESS = "0.0.0.0:50000";

// TODO: This client should be able to be used as standalone package
class KeyValueStoreClient
{
public:
  explicit KeyValueStoreClient(std::shared_ptr<Channel> channel)
      : stub_(KeyValueStore::NewStub(channel)) {}
  void Put(const std::string &key, const std::string &value)
  {
    PutRequest request;
    request.set_key(key);
    request.set_value(value);
    AsyncClientCall *call = new AsyncClientCall;
    call->response_reader = stub_->PrepareAsyncput(&call->context, request, &cq_);
    call->response_reader->StartCall();
    call->response_reader->Finish(&call->reply, &call->status, (void *)call);
  }

  void AsyncCompleteRpc()
  {
    void *got_tag;
    bool ok = false;
    while (cq_.Next(&got_tag, &ok))
    {
      AsyncClientCall *call = static_cast<AsyncClientCall *>(got_tag);
      GPR_ASSERT(ok);
      if (call->status.ok())
      {
        std::cout << "Put Success" << std::endl;
      }
      else
      {
        std::cout << "Put Failed" << std::endl;
      }
      delete call;
    }
  }

private:
  struct AsyncClientCall
  {
    PutReply reply;
    ClientContext context;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<PutReply>> response_reader;
  };

  std::unique_ptr<KeyValueStore::Stub> stub_;
  CompletionQueue cq_;
};

int main(int argc, char **argv)
{
  KeyValueStoreClient store(grpc::CreateChannel(
      SERVER_ADDRESS, grpc::InsecureChannelCredentials()));
  std::thread thread_ = std::thread(&KeyValueStoreClient::AsyncCompleteRpc, &store);
  store.Put("test", "test");
  thread_.join();
  return 0;
}