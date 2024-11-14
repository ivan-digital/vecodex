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
# fetch latest argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)


# Main executable
file(GLOB SRC_LIST src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} argparse)
