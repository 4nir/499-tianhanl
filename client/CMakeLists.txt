cmake_minimum_required(VERSION 3.3)

add_library(command_client_core command_client_core.cc)
target_link_libraries(command_client_core
  service_layer_client_sync
  gflags
  glog
)

add_executable(command_client command_client.cc)
target_link_libraries(command_client PUBLIC
  command_client_core
)