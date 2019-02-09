cmake_minimum_required(VERSION 3.3)

#Proto file
get_filename_component(store_proto "../protos/key_value_store.proto" ABSOLUTE)
get_filename_component(store_proto_path "${store_proto}" PATH)

# Generated sources
set(DIST_DIR "${CMAKE_CURRENT_BINARY_DIR}/dist")
file(MAKE_DIRECTORY ${DIST_DIR})
set(store_proto_srcs "${DIST_DIR}/key_value_store.pb.cc")
set(store_proto_hdrs "${DIST_DIR}/key_value_store.pb.h")
set(store_grpc_srcs "${DIST_DIR}/key_value_store.grpc.pb.cc")
set(store_grpc_hdrs "${DIST_DIR}/key_value_store.grpc.pb.h")
add_custom_command(
      OUTPUT "${store_proto_srcs}" "${store_proto_hdrs}" "${store_grpc_srcs}" "${store_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${DIST_DIR}"
        --cpp_out "${DIST_DIR}"
        -I "${store_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${store_proto}"
      DEPENDS ${store_proto})

# Include generated *.pb.h files
include_directories("${DIST_DIR}")

add_library(store STATIC store.cc store.h)
add_executable(store_test store_test.cc)
target_link_libraries(store_test PUBLIC
    gtest
    gtest_main
    store
)
add_test(
    NAME store_test
    COMMAND store_test
)

add_executable(key_value_store_server_sync key_value_store_server_sync.cc
    ${store_proto_srcs}
    ${store_grpc_srcs})
target_link_libraries(key_value_store_server_sync
    ${_GRPC_GRPCPP_UNSECURE}
    ${_PROTOBUF_LIBPROTOBUF}
    store)
    
add_library(key_value_store_client_sync key_value_store_client_sync.cc
    ${store_proto_srcs}
    ${store_grpc_srcs})
target_link_libraries(key_value_store_client_sync
    ${_GRPC_GRPCPP_UNSECURE}
    ${_PROTOBUF_LIBPROTOBUF})

add_executable(key_value_store_client_sync_test key_value_store_client_sync_test.cc)
target_link_libraries(key_value_store_client_sync_test PUBLIC
    gtest
    gtest_main
    key_value_store_client_sync
)

