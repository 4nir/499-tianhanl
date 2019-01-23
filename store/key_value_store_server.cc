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

#include <memory>
#include <iostream>
#include <string>
#include <thread>

#include "store.h"

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
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
using grpc::StatusCode;

class BaseCallData
{
public:
  BaseCallData()
      : status_(CREATE)
  {
  }

  virtual void Proceed() = 0;

protected:
  enum CallStatus
  {
    CREATE,
    PROCESS,
    FINISH
  };
  CallStatus status_; // The current serving state.
};

class PutCallData : public BaseCallData
{
public:
  PutCallData(KeyValueStore::AsyncService *service, ServerCompletionQueue *cq,
              Store *store)
      : service_(service), cq_(cq), responder_(&ctx_), store_(store)
  {
    Proceed();
  }
  void Proceed() override
  {
    if (status_ == CREATE)
    {
      // Make this instance progress to the PROCESS state.
      status_ = PROCESS;

      // request proccing put and use this (memory address) as unique tag
      service_->Requestput(&ctx_, &request_, &responder_, cq_, cq_, this);
    }
    else if (status_ == PROCESS)
    {
      // Spawn a new CallData instance to serve new clients while we process
      // the one for this CallData. The instance will deallocate itself as
      // part of its FINISH state.
      new PutCallData(service_, cq_, store_);

      std::string key = request_.key();
      std::string value = request_.value();
      bool put_result = store_->Put(key, value);
      status_ = FINISH;
      if (put_result)
      {
        responder_.Finish(reply_, Status::OK, this);
      }
      else
      {
        responder_.Finish(reply_,
                          Status(StatusCode::INVALID_ARGUMENT, "Unable to put"),
                          this);
      }
    }
    else
    {
      GPR_ASSERT(status_ == FINISH);
      // Once in the FINISH state, deallocate ourselves (CallData).
      delete this;
    }
  }

private:
  // The means of communication with the gRPC runtime for an asynchronous
  // server.
  KeyValueStore::AsyncService *service_;
  // The producer-consumer queue where for asynchronous server notifications.
  ServerCompletionQueue *cq_;
  // Context for the rpc, allowing to tweak aspects of it such as the use
  // of compression, authentication, as well as to send metadata back to the
  // client.
  ServerContext ctx_;

  // What we get from the client.
  PutRequest request_;
  // What we send back to the client.
  PutReply reply_;

  // The means to get back to the client.
  ServerAsyncResponseWriter<PutReply> responder_;
  Store *store_;
};

class KeyValueStoreServerImpl final
{
public:
  KeyValueStoreServerImpl()
  {
    store_ = new Store;
  }
  ~KeyValueStoreServerImpl()
  {
    server_->Shutdown();
    cq_->Shutdown();
    delete store_;
  }

  void Run()
  {
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
  void HandleRpcs()
  {
    new PutCallData(&service_, cq_.get(), store_);
    void *tag;
    bool ok;
    while (true)
    {
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<BaseCallData *>(tag)->Proceed();
    }
  }
  std::unique_ptr<ServerCompletionQueue> cq_;
  KeyValueStore::AsyncService service_;
  std::unique_ptr<Server> server_;
  Store *store_;
};

int main(int argc, char **argv)
{
  KeyValueStoreServerImpl server;
  server.Run();
  return 0;
}
