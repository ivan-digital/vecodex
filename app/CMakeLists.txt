cmake_minimum_required(VERSION 3.12)

project(vecodex-app)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific configurations
# TBD

# Include directories
include_directories(
    "/usr/include"
    "${PROJECT_SOURCE_DIR}/include"
)

# Common configurations
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# TBD
include(CheckCXXCompilerFlag)
message(STATUS "Enabling AddressSanitizer")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

# Threading
find_package(Threads REQUIRED)

# Google Test Integration
# TBD
if (NOT TARGET gtest)
    # add_subdirectory(external/googletest)
endif()

# ArgParse library
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# Json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# gRPC
option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()
# Build proto files
set(PROTO_BINARY_DIR "${PROJECT_SOURCE_DIR}/generated")
set(PROTO_SOURCE_DIR "${PROJECT_SOURCE_DIR}/proto")
file(GLOB PROTO_FILES ${PROJECT_SOURCE_DIR}/proto/*.proto)
file(MAKE_DIRECTORY ${PROTO_BINARY_DIR})

set(SRCS)
set(HDRS)
foreach(FIL ${PROTO_FILES})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    list(APPEND SRCS "${PROTO_BINARY_DIR}/${FIL_WE}.grpc.pb.cc")
    list(APPEND HDRS "${PROTO_BINARY_DIR}/${FIL_WE}.grpc.pb.h")
    list(APPEND SRCS "${PROTO_BINARY_DIR}/${FIL_WE}.pb.cc")
    list(APPEND HDRS "${PROTO_BINARY_DIR}/${FIL_WE}.pb.h")

    add_custom_command(
      OUTPUT "${PROTO_BINARY_DIR}/${FIL_WE}.grpc.pb.cc"
             "${PROTO_BINARY_DIR}/${FIL_WE}.grpc.pb.h"
             "${PROTO_BINARY_DIR}/${FIL_WE}.pb.cc"
             "${PROTO_BINARY_DIR}/${FIL_WE}.pb.h"
      COMMAND  ${_PROTOBUF_PROTOC}
      ARGS --grpc_out=${PROTO_BINARY_DIR}
           --cpp_out=${PROTO_BINARY_DIR} 
           --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
           --proto_path=${PROTO_SOURCE_DIR}
            ${FIL}
      DEPENDS ${ABS_FIL} ${_PROTOBUF_PROTOC} ${_GRPC_CPP_PLUGIN_EXECUTABLE}
      COMMENT "Running gRPC C++ protocol buffer compiler on ${FIL}"
      VERBATIM
    )
endforeach()

add_library(proto-objects ${SRCS} ${HDRS})
target_link_libraries(proto-objects "${_GRPC_GRPCPP}" "${_PROTOBUF_LIBPROTOBUF}" "${_REFLECTION}")
target_include_directories(proto-objects PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

# etcd-cpp-api
# needs libboost-all-dev libssl-dev cpprestsdk; see https://github.com/etcd-cpp-apiv3/etcd-cpp-apiv3/tree/master
add_subdirectory(external/etcd-cpp-apiv3)

# Main executable
file(GLOB SRC_LIST src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} 
  argparse 
  ${_PROTOBUF_LIBPROTOBUF} 
  ${_REFLECTION} 
  ${_GRPC_GRPCPP} 
  proto-objects 
  etcd-cpp-api 
  nlohmann_json::nlohmann_json
) 
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROTO_BINARY_DIR} ${PROJECT_SOURCE_DIR}/external/etcd-cpp-apiv3)

# Example client
add_executable(example_client ${PROJECT_SOURCE_DIR}/examples/run_client.cpp ${PROJECT_SOURCE_DIR}/src/BaseClient.cpp)
target_link_libraries(example_client PRIVATE ${_PROTOBUF_LIBPROTOBUF} ${_REFLECTION} ${_GRPC_GRPCPP} proto-objects)
target_include_directories(example_client PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROTO_BINARY_DIR})

# Etcd client
add_executable(etcd_client ${PROJECT_SOURCE_DIR}/examples/etcd_client.cpp)
target_link_libraries(etcd_client PRIVATE etcd-cpp-api)
target_include_directories(etcd_client PRIVATE ${PROJECT_SOURCE_DIR}/external/etcd-cpp-apiv3)