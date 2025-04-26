cmake_minimum_required(VERSION 3.12)

project(vecodex-index)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
enable_testing()
# Platform-specific configurations
if(APPLE)
    message(STATUS "Configuring for macOS")
    # Use Apple Clang
    # set(CMAKE_C_COMPILER "/usr/bin/clang")
    # set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

    message(STATUS "Using Apple Clang: ${CMAKE_C_COMPILER} and ${CMAKE_CXX_COMPILER}")

    # Get Homebrew prefixes
    execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND brew --prefix openblas OUTPUT_VARIABLE OPENBLAS_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Set CMAKE_PREFIX_PATH to prioritize Homebrew
    set(CMAKE_PREFIX_PATH
        "${LIBOMP_PREFIX}"
        "${OPENBLAS_PREFIX}"
        "${CMAKE_PREFIX_PATH}"
        CACHE STRING "CMake Prefix Path")

    # Include directories and library paths
    include_directories(
        "${LIBOMP_PREFIX}/include"
        "${OPENBLAS_PREFIX}/include"
        "${PROJECT_SOURCE_DIR}/include"
    )

    link_directories(
        "${LIBOMP_PREFIX}/lib"
        "${OPENBLAS_PREFIX}/lib"
    )

elseif(UNIX)
    message(STATUS "Configuring for Linux/Ubuntu")

    # Include directories
    include_directories(
        "/usr/include"
        "${PROJECT_SOURCE_DIR}/include"
    )

    # Manually specify OpenBLAS paths for Linux/Ubuntu
    include_directories("/usr/include/openblas")
    find_library(OpenBLAS_LIBRARIES NAMES openblas PATHS /usr/lib /usr/lib/x86_64-linux-gnu)

    if(OpenBLAS_LIBRARIES)
        message(STATUS "Found OpenBLAS: ${OpenBLAS_LIBRARIES}")
    else()
        message(FATAL_ERROR "OpenBLAS not found")
    endif()
endif()

# Common configurations

# OpenMP Support
find_package(OpenMP REQUIRED)

if(OpenMP_FOUND)
    message(STATUS "Found OpenMP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

# Threading
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread timer chrono)

# FAISS Configuration
# Prevent CMake from searching system paths for FAISS by not setting FAISS_ROOT or similar variables
# Ensure FAISS is built from the submodule
set(FAISS_ENABLE_PYTHON OFF CACHE BOOL "Disable Python support in FAISS")
set(FAISS_ENABLE_GPU OFF CACHE BOOL "Disable GPU support in FAISS")
set(FAISS_ENABLE_OPENMP ON CACHE BOOL "Enable OpenMP support in FAISS")
set(FAISS_BUILD_TESTS OFF CACHE BOOL "Disable FAISS tests")
#set(FAISS_CMAKE_C_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "")
#set(FAISS_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "")

# Add FAISS as a subdirectory before any find_package calls that might locate system FAISS
add_subdirectory(external/faiss)
# Google Test Integration
if (NOT TARGET gtest)
    add_subdirectory(external/googletest)
endif()

# Json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# Main Library Target
add_library(vecodex-index INTERFACE)
target_link_libraries(vecodex-index INTERFACE faiss)
target_include_directories(vecodex-index INTERFACE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/baselines
        external/faiss)

# Test Executable
add_executable(VecodexIndexTest tests/VecodexIndexTest.cpp)
add_test(NAME test_index_basic COMMAND VecodexIndexTest)

target_include_directories(VecodexIndexTest PRIVATE ${PROJECT_SOURCE_DIR}/baselines)
# Link Libraries to Main Library Target

# Link Libraries to Test Executable
target_link_libraries(VecodexIndexTest PRIVATE Boost::boost Boost::thread nlohmann_json::nlohmann_json faiss gtest gtest_main OpenMP::OpenMP_CXX Threads::Threads ${OpenBLAS_LIBRARIES} vecodex-index)

add_executable(Benchmark tests/Benchmark.cpp)
target_include_directories(Benchmark PRIVATE ${PROJECT_SOURCE_DIR}/baselines)

target_link_libraries(Benchmark PRIVATE Boost::boost Boost::thread nlohmann_json::nlohmann_json faiss gtest gtest_main OpenMP::OpenMP_CXX Threads::Threads ${OpenBLAS_LIBRARIES} vecodex-index)

# Additional macOS-specific settings
if(APPLE)
    # Example: Link frameworks if needed
    target_link_libraries(VecodexIndexTest PRIVATE "-framework Accelerate")

    # Set macOS specific compile options
    target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra)

    # Set rpath for dynamic libraries if necessary
    set_target_properties(VecodexIndexTest PROPERTIES INSTALL_RPATH "@loader_path/lib")
endif()

# Additional Linux-specific settings
if(UNIX AND NOT APPLE)
    # Example: Link pthread explicitly if needed
    target_link_libraries(VecodexIndexTest PRIVATE pthread)

    # Set Linux specific compile options
    target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra -fPIC)

    # Set rpath for dynamic libraries if necessary
    set_target_properties(VecodexIndexTest PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
endif()
