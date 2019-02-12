# 499-tianhanl

name: Tianhang Liu
email: tianhanl@usc.edu

## Project Description

Chrip is a clone of Twiter with storage component, service component, and commandline component using gRPC.

## Environment

Using Vagrant box:
ubuntu/bionic64 (virtualbox, 20190109.0.0)
Vagrant file is attached.

## Project Dependency

### Globally installed

gRPC
Protobuf
cmake - as build tool

### Git submodule

googletest
gflags
benchmark
glog

## Install

### Install global modules

```bash
#  gRPC
# Pre-requisites
[sudo] apt-get install build-essential autoconf libtool pkg-config
# Install from source
git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
cd grpc
git submodule update --init

# Protobuf
cd grpc/third_party/protobuf
sudo make install   # 'make' should have been run by core grpc

#cmake
# install PPA
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:george-edison55/cmake-3.x
sudo apt-get update
# if cmake is not installed
sudo apt-get install cmake
# else
sudo apt-get upgrade
```

```bash
# install submodule dependency
git submodule init
git submodule update
# in root folder
cmake
make
```

## Usage

```bash
# After installation at root level
# Start store server
./store/key_value_store_server_sync
# Start service layer server
./server/service_layer_server_sync
# Use client
#Register
./client/command_client --register username
# Chirp
./client/command_client --user username --chirp test_content
# Reply
./client/command_client --user username --chirp test_content --reply reply_to_chirp_id
# Read
./client/command_client --user username --read chirp_id
# Follow
./client/command_client --register username2
./client/command_client --user username --follow username2
# Monitor
./client/command_client --user username --monitor
```

## Testing

Networked components have been separated into interfaces and implementations, and components should use dev implementations when unittesting.

Unittesting have been register with `ctest`, and you can run all of them via:

```bash
# After make
make test
```

`*_client_sync_test.cc` is mean to be integration test which requires a running server inatnace

```bash
# start store server
./store/key_value_store_server_sync
# run store client test

```
