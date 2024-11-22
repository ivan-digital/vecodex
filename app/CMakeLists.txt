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
# TBD

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


# gRPC
add_subdirectory(external/grpc)


# generate pb.cc pb.h
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_library(proto-objects OBJECT "${CMAKE_CURRENT_LIST_DIR}/proto/service.proto")

target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf gRPC::grpc++)

set(PROTO_BINARY_DIR "${PROJECT_SOURCE_DIR}/generated")
set(PROTO_IMPORT_DIRS "${PROJECT_SOURCE_DIR}/proto")
file(MAKE_DIRECTORY ${PROTO_BINARY_DIR})

target_include_directories(proto-objects PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

protobuf_generate(
    TARGET proto-objects
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")

protobuf_generate(
    TARGET proto-objects
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")


# Main executable
file(GLOB SRC_LIST src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} argparse protobuf::libprotobuf gRPC::grpc++ proto-objects)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/generated)


# Example client
add_executable(example_client ${PROJECT_SOURCE_DIR}/examples/run_client.cpp ${PROJECT_SOURCE_DIR}/src/coordinator.cpp ${PROJECT_SOURCE_DIR}/src/base.cpp)
target_link_libraries(example_client argparse protobuf::libprotobuf gRPC::grpc++ proto-objects)
target_include_directories(example_client PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/generated)
