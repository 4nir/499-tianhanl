cmake_minimum_required(VERSION 3.3)

add_executable(command_client command_client.cc)
target_link_libraries(command_client PUBLIC
  service_layer_client_sync
  gflags
  glog
)