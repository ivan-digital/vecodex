cmake_minimum_required(VERSION 3.12)

project(vecodex-index LANGUAGES C CXX)

# ─────────── Compiler & build settings ───────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

# ─────────── Platform specifics ───────────
if(APPLE)
  message(STATUS "Configuring for macOS / Apple Silicon")
  set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "" FORCE)
endif()

# ─────────── Threads & OpenMP ───────────
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)

# ─────────── Boost ───────────
find_package(Boost REQUIRED COMPONENTS thread chrono timer)

# ─────────── Math / BLAS interface target ───────────
add_library(math INTERFACE)

if(APPLE)
  target_link_libraries(math INTERFACE "-framework Accelerate")
else()
  find_library(OpenBLAS_LIB openblas
               PATHS /usr/lib /usr/lib/x86_64-linux-gnu /opt/homebrew/lib)
  if(NOT OpenBLAS_LIB)
    message(FATAL_ERROR "OpenBLAS not found (install libopenblas-dev)")
  endif()
  target_link_libraries(math INTERFACE ${OpenBLAS_LIB})
endif()

# ─────────── Faiss (CPU-only) ───────────
set(FAISS_ENABLE_GPU  OFF CACHE BOOL "Disable Faiss CUDA support" FORCE)
set(FAISS_ENABLE_ROCM OFF CACHE BOOL "" FORCE)
set(FAISS_ENABLE_CUVS OFF CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit ON CACHE BOOL "" FORCE)
add_subdirectory(external/faiss)

# ─────────── Google Benchmark ───────────
if(NOT TARGET benchmark::benchmark)
  include(FetchContent)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_INSTALL        OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.8.3
  )
  FetchContent_MakeAvailable(googlebenchmark)
endif()

# ─────────── Google Test ───────────
if(NOT TARGET gtest)
  add_subdirectory(external/googletest)
endif()

# ─────────── nlohmann/json ───────────
include(FetchContent)
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# ─────────── vecodex-index interface library ───────────
add_library(vecodex-index INTERFACE)

target_link_libraries(vecodex-index INTERFACE
  faiss
  math              # propagates BLAS / Accelerate
  OpenMP::OpenMP_CXX)

target_include_directories(vecodex-index INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/baselines>
  $<INSTALL_INTERFACE:include>
  external/faiss)

# ─────────── Test executable ───────────
add_executable(VecodexIndexTest tests/VecodexIndexTest.cpp)

target_include_directories(VecodexIndexTest PRIVATE ${PROJECT_SOURCE_DIR}/baselines)

target_link_libraries(VecodexIndexTest PRIVATE
  Boost::boost Boost::thread Boost::chrono Boost::timer
  nlohmann_json::nlohmann_json
  gtest gtest_main             # single main() from gtest
  Threads::Threads
  vecodex-index)

add_test(NAME test_index_basic COMMAND VecodexIndexTest)

# ─────────── Benchmark executable ───────────
add_executable(Benchmark tests/Benchmark.cpp)

target_include_directories(Benchmark PRIVATE ${PROJECT_SOURCE_DIR}/baselines)

target_link_libraries(Benchmark PRIVATE
  benchmark::benchmark_main    # single main() from benchmark
  Boost::boost Boost::thread Boost::chrono Boost::timer
  nlohmann_json::nlohmann_json
  Threads::Threads
  vecodex-index)

# ─────────── Warnings & PIC ───────────
target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra)
target_compile_options(Benchmark        PRIVATE -Wall -Wextra)
if(UNIX AND NOT APPLE)
  target_compile_options(VecodexIndexTest PRIVATE -fPIC)
endif()
