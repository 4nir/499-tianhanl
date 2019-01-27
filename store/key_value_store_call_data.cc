#include <iostream>
#include <memory>
#include <string>

#include "store.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include "./dist/key_value_store.grpc.pb.h"

using chirp::KeyValueStore;
using chirp::PutReply;
using chirp::PutRequest;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

// Base for all CallDatas for polymorphism in
// KeyValueStoreServerImpl::HandleRcps Defines basic state machine CREATE ->
// PROCESS -> FINISH
// NOTICE: Pointer is expected to be self-destructing, and caller should not
// delete it.
class BaseCallData {
 public:
  BaseCallData() : status_(CREATE) {}

  // Will be called to move status_ into next status, and perform processing
  // according to current status.
  virtual void Proceed() = 0;

  // Initialize call data to process initial status.
  void Init() { Proceed(); }

 protected:
  enum CallStatus { CREATE, PROCESS, FINISH };
  CallStatus status_;  // The current serving state.
};

// CallData for put method will put user supplied key and value into store
class PutCallData : public BaseCallData {
 public:
  PutCallData(KeyValueStore::AsyncService *service, ServerCompletionQueue *cq,
              Store *store)
      : service_(service), cq_(cq), responder_(&ctx_), store_(store) {}

  void Proceed() override {
    if (status_ == CREATE) {
      // Make this instance progress to the PROCESS state.
      status_ = PROCESS;

      // request proccing put and use this (memory address) as unique tag
      service_->Requestput(&ctx_, &request_, &responder_, cq_, cq_, this);
    } else if (status_ == PROCESS) {
      // Spawn a new CallData instance to serve new clients while we process
      // the one for this CallData. The instance will deallocate itself as
      // part of its FINISH state.
      PutCallData *next = new PutCallData(service_, cq_, store_);
      next->Init();

      const std::string &key = request_.key();
      const std::string &value = request_.value();
      bool ok = store_->Put(key, value);
      status_ = FINISH;
      if (ok) {
        responder_.Finish(reply_, Status::OK, this);
      } else {
        responder_.Finish(reply_,
                          Status(StatusCode::INVALID_ARGUMENT, "Unable to put"),
                          this);
      }
    } else {
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

  // Store instance used to store key value pairs
  Store *store_;
};
