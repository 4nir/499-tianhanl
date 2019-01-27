#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "./dist/key_value_store.grpc.pb.h"

using chirp::PutReply;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::Status;

//  CallData object used to maintain context of a rpc call
struct PutClientCall {
  // Put reply object
  PutReply reply;

  // Context for the rpc
  ClientContext context;

  // Status for the rpc
  Status status;

  // Means to read response from the server
  std::unique_ptr<ClientAsyncResponseReader<PutReply>> response_reader;
};