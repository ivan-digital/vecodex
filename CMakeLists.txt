cmake_minimum_required(VERSION 3.12)

project(vecodex-index)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific configurations
if(APPLE)
    message(STATUS "Configuring for macOS")

    # Use Apple Clang
    set(CMAKE_C_COMPILER "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

    message(STATUS "Using Apple Clang: ${CMAKE_C_COMPILER} and ${CMAKE_CXX_COMPILER}")

    # Get Homebrew prefixes
    execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND brew --prefix openblas OUTPUT_VARIABLE OPENBLAS_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND brew --prefix faiss OUTPUT_VARIABLE FAISS_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND brew --prefix googletest OUTPUT_VARIABLE GTEST_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Set CMAKE_PREFIX_PATH to prioritize Homebrew
    set(CMAKE_PREFIX_PATH
        "${LIBOMP_PREFIX}"
        "${OPENBLAS_PREFIX}"
        "${FAISS_PREFIX}"
        "${GTEST_PREFIX}"
        "${CMAKE_PREFIX_PATH}"
        CACHE STRING "CMake Prefix Path" FORCE)

    # Include directories and library paths
    include_directories(
        "${LIBOMP_PREFIX}/include"
        "${OPENBLAS_PREFIX}/include"
        "${FAISS_PREFIX}/include"
        "${GTEST_PREFIX}/include"
        "${PROJECT_SOURCE_DIR}/include"
    )

    link_directories(
        "${LIBOMP_PREFIX}/lib"
        "${OPENBLAS_PREFIX}/lib"
        "${FAISS_PREFIX}/lib"
        "${GTEST_PREFIX}/lib"
    )

elseif(UNIX)
    message(STATUS "Configuring for Linux/Ubuntu")

    # Include directories
    include_directories(
        "/usr/include"
        "${PROJECT_SOURCE_DIR}/include"
    )

    # Manually specify OpenBLAS paths for Linux/Ubuntu
    include_directories("/usr/include/openblas")  # Include OpenBLAS directory manually
    find_library(OpenBLAS_LIBRARIES NAMES openblas PATHS /usr/lib /usr/lib/x86_64-linux-gnu)

    if(OpenBLAS_LIBRARIES)
        message(STATUS "Found OpenBLAS: ${OpenBLAS_LIBRARIES}")
    else()
        message(FATAL_ERROR "OpenBLAS not found")
    endif()

    # Link pthread explicitly if needed
    target_link_libraries(vecodex-index PRIVATE pthread)
    target_link_libraries(VecodexIndexTest PRIVATE pthread)

    # Set Linux specific compile options
    target_compile_options(vecodex-index PRIVATE -Wall -Wextra -fPIC)
    target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra -fPIC)

    # Set rpath for dynamic libraries if necessary
    set_target_properties(vecodex-index PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
    set_target_properties(VecodexIndexTest PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
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

# FAISS Configuration
# Prevent CMake from searching system paths for FAISS by not setting FAISS_ROOT or similar variables
# Ensure FAISS is built from the submodule
set(FAISS_ENABLE_PYTHON OFF CACHE BOOL "Disable Python support in FAISS" FORCE)
set(FAISS_ENABLE_GPU OFF CACHE BOOL "Disable GPU support in FAISS" FORCE)
set(FAISS_ENABLE_OPENMP ON CACHE BOOL "Enable OpenMP support in FAISS" FORCE)
set(FAISS_BUILD_TESTS OFF CACHE BOOL "Disable FAISS tests" FORCE)
set(FAISS_CMAKE_C_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "" FORCE)
set(FAISS_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "" FORCE)

# Add FAISS as a subdirectory before any find_package calls that might locate system FAISS
add_subdirectory(external/faiss)

# Google Test Integration
if (NOT TARGET gtest)
    add_subdirectory(external/googletest)
endif()

include_directories(external/googletest/googletest/include)

# Main Library Target
add_library(vecodex-index
    src/VecodexIndex.cpp
    src/VecodexSegment.cpp
    src/DocumentMetadata.cpp
    include/VecodexIndex.h
    include/VecodexSegment.h
    include/DocumentMetadata.h
)

target_include_directories(vecodex-index PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link Libraries
target_link_libraries(vecodex-index PRIVATE faiss OpenMP::OpenMP_CXX Threads::Threads ${OpenBLAS_LIBRARIES})

# Test Executable
add_executable(VecodexIndexTest tests/VecodexIndexTest.cpp)

target_link_libraries(VecodexIndexTest PRIVATE faiss gtest gtest_main OpenMP::OpenMP_CXX Threads::Threads ${OpenBLAS_LIBRARIES})

# Additional macOS-specific settings
if(APPLE)
    # Example: Link frameworks if needed
    target_link_libraries(vecodex-index PRIVATE "-framework Accelerate")
    target_link_libraries(VecodexIndexTest PRIVATE "-framework Accelerate")
    
    # Set macOS specific compile options
    target_compile_options(vecodex-index PRIVATE -Wall -Wextra)
    target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra)
    
    # Set rpath for dynamic libraries if necessary
    set_target_properties(vecodex-index PROPERTIES INSTALL_RPATH "@loader_path/lib")
    set_target_properties(VecodexIndexTest PROPERTIES INSTALL_RPATH "@loader_path/lib")
endif()

# Additional Linux-specific settings
if(UNIX AND NOT APPLE)
    # Example: Link pthread explicitly if needed
    target_link_libraries(vecodex-index PRIVATE pthread)
    target_link_libraries(VecodexIndexTest PRIVATE pthread)
    
    # Set Linux specific compile options
    target_compile_options(vecodex-index PRIVATE -Wall -Wextra -fPIC)
    target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra -fPIC)
    
    # Set rpath for dynamic libraries if necessary
    set_target_properties(vecodex-index PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
    set_target_properties(VecodexIndexTest PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
endif()
