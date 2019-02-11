# 499-tianhanl

name: Tianhang Liu
email: tianhanl@usc.edu

## Project Description

Chrip is clone of Twiter with storage component, service component, and frontend component using gRPC.

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

## Testing

Networked components have been separated into interfaces and implementations, and components should use dev implementations when unittesting.

Unittesting have been register with `ctest`, and you can run all of them via:

```bash
# After make
make test
```

`*_client_sync_test.cc` is mean to be integration test which requires a running server inatnace
