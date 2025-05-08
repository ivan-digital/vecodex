cmake_minimum_required(VERSION 3.12)

project(vecodex-index LANGUAGES C CXX)

# ───────────────────────────── Build options ──────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# ───────────────────────────── Platform setup ─────────────────────────────
if(APPLE)
  message(STATUS "Configuring for macOS / Apple Silicon")
  set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "" FORCE)
endif()

# ─────────────────────────── Threading & OpenMP ──────────────────────────
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)

# ───────────────────────────── Boost (header-only + libs) ─────────────────
find_package(Boost REQUIRED COMPONENTS thread chrono timer)

# ────────────────────── Choose BLAS/LAPACK implementation ─────────────────
add_library(math INTERFACE)

if(APPLE)
  # Apple’s Accelerate framework has BLAS/LAPACK and is arm64-native
  target_link_libraries(math INTERFACE "-framework Accelerate")
else() # generic UNIX – use OpenBLAS
  find_library(OpenBLAS_LIBRARIES openblas
               PATHS /usr/lib /usr/lib/x86_64-linux-gnu
                     /opt/homebrew/lib
               NO_DEFAULT_PATH)
  if(NOT OpenBLAS_LIBRARIES)
    message(FATAL_ERROR "OpenBLAS not found – please install libopenblas-dev")
  endif()
  target_link_libraries(math INTERFACE ${OpenBLAS_LIBRARIES})
endif()

# ────────────────────────────── External deps ─────────────────────────────
add_subdirectory(external/faiss)

# Google Benchmark – fetch only if nobody else already did
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

# Google Test – use vendored copy
if(NOT TARGET gtest)
  add_subdirectory(external/googletest)
endif()

# nlohmann/json – header-only
include(FetchContent)
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# ───────────────────── vecodex-index interface library ────────────────────
add_library(vecodex-index INTERFACE)

target_link_libraries(vecodex-index INTERFACE
  faiss
  math
  OpenMP::OpenMP_CXX)

target_include_directories(vecodex-index INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/baselines>
  $<INSTALL_INTERFACE:include>
  external/faiss)

# ────────────────────────────── Test executable ───────────────────────────
add_executable(VecodexIndexTest tests/VecodexIndexTest.cpp)

target_include_directories(VecodexIndexTest PRIVATE
  ${PROJECT_SOURCE_DIR}/baselines)

target_link_libraries(VecodexIndexTest PRIVATE
  Boost::boost Boost::thread Boost::chrono Boost::timer
  nlohmann_json::nlohmann_json
  gtest gtest_main
  Threads::Threads
  vecodex-index)

add_test(NAME test_index_basic COMMAND VecodexIndexTest)

# ─────────────────────────── Benchmark executable ─────────────────────────
add_executable(Benchmark tests/Benchmark.cpp)

target_include_directories(Benchmark PRIVATE
  ${PROJECT_SOURCE_DIR}/baselines)

target_link_libraries(Benchmark PRIVATE
  benchmark::benchmark_main 
  Boost::boost Boost::thread Boost::chrono Boost::timer
  nlohmann_json::nlohmann_json
  Threads::Threads
  vecodex-index)

# ─────────────────────────── Compiler warnings etc. ───────────────────────
target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra)
target_compile_options(Benchmark        PRIVATE -Wall -Wextra)

if(UNIX AND NOT APPLE)
  # Position-independent code is often needed on Linux
  target_compile_options(VecodexIndexTest PRIVATE -fPIC)
endif()

# Install rules or packaging can be added here if needed.
