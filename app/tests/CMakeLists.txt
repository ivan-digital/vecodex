FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
FetchContent_MakeAvailable(googletest)

set(UT_DIR ${TEST_DIRECTORY}/ut)

file(GLOB SRC_UT_LIST ${UT_DIR}/*.cpp)

add_executable(vecodex_app_ut ${SRC_UT_LIST})
target_link_libraries(vecodex_app_ut PRIVATE 
  gtest_main 
  etcd-cpp-api
  nlohmann_json::nlohmann_json
  ${_PROTOBUF_LIBPROTOBUF} 
  ${_REFLECTION} ${_GRPC_GRPCPP} 
  proto-objects
)
target_include_directories(vecodex_app_ut PRIVATE 
  ${PROJECT_SOURCE_DIR}/include
  ${PROTO_BINARY_DIR} 
  ${PROJECT_SOURCE_DIR}/external/etcd-cpp-apiv3
)
add_test(NAME vecodex_app_ut COMMAND tests)  