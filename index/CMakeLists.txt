cmake_minimum_required(VERSION 3.12)
project(vecodex-index LANGUAGES C CXX)

# ─── C++ standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
enable_testing()

# ─── Platform-specific configurations ────────────────────────────────────────────
if(APPLE)
    message(STATUS "Configuring for macOS")

    # Detect Homebrew prefixes for LLVM/OpenMP/OpenBLAS
    execute_process(COMMAND brew --prefix llvm
                    OUTPUT_VARIABLE _LLVM_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND brew --prefix libomp
                    OUTPUT_VARIABLE _LIBOMP_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND brew --prefix openblas
                    OUTPUT_VARIABLE _OPENBLAS_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    message(STATUS "  LLVM @ ${_LLVM_PREFIX}")
    message(STATUS "  libomp @ ${_LIBOMP_PREFIX}")
    message(STATUS "  OpenBLAS @ ${_OPENBLAS_PREFIX}")

    # Force FindOpenMP to pick up Homebrew libomp
    set(OpenMP_C_FLAGS       "-Xpreprocessor -fopenmp" CACHE STRING "" FORCE)
    set(OpenMP_CXX_FLAGS     "-Xpreprocessor -fopenmp" CACHE STRING "" FORCE)
    set(OpenMP_C_LIB_NAMES   "omp" CACHE STRING "" FORCE)
    set(OpenMP_CXX_LIB_NAMES "omp" CACHE STRING "" FORCE)
    set(OpenMP_omp_LIBRARY   "${_LIBOMP_PREFIX}/lib/libomp.dylib" CACHE FILEPATH "" FORCE)

    # Tell CMake where to look
    set(CMAKE_PREFIX_PATH
        "${_LLVM_PREFIX}"
        "${_LIBOMP_PREFIX}"
        "${_OPENBLAS_PREFIX}"
        ${CMAKE_PREFIX_PATH}
        CACHE STRING "Search prefixes" FORCE)

    include_directories(
        "${_LIBOMP_PREFIX}/include"
        "${_OPENBLAS_PREFIX}/include"
        "${PROJECT_SOURCE_DIR}/include"
    )
    link_directories(
        "${_LIBOMP_PREFIX}/lib"
        "${_OPENBLAS_PREFIX}/lib"
    )

elseif(UNIX)
    message(STATUS "Configuring for Linux/Ubuntu")
    include_directories(
        "/usr/include"
        "${PROJECT_SOURCE_DIR}/include"
        "/usr/include/openblas"
    )
    find_library(OpenBLAS_LIBRARIES NAMES openblas PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
    if(NOT OpenBLAS_LIBRARIES)
        message(FATAL_ERROR "OpenBLAS not found")
    endif()
    message(STATUS "Found OpenBLAS: ${OpenBLAS_LIBRARIES}")
endif()

# ─── OpenMP Support ─────────────────────────────────────────────────────────────
find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
    message(STATUS "Found OpenMP")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

# ─── Threading & Boost ───────────────────────────────────────────────────────────
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread timer chrono)

# ─── FAISS Submodule Configuration ──────────────────────────────────────────────
set(FAISS_ENABLE_PYTHON OFF CACHE BOOL "" FORCE)
set(FAISS_ENABLE_GPU    OFF CACHE BOOL "" FORCE)
set(FAISS_ENABLE_OPENMP ON  CACHE BOOL "" FORCE)
set(BUILD_TESTING       OFF CACHE BOOL "" FORCE)
set(FAISS_ENABLE_TESTS      OFF CACHE BOOL "" FORCE)
set(FAISS_ENABLE_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(FAISS_ENABLE_PERF_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/faiss)

# ─── Google Benchmark & gtest ───────────────────────────────────────────────────
include(FetchContent)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.7.1
)
FetchContent_MakeAvailable(googlebenchmark)

if(NOT TARGET gtest)
    add_subdirectory(external/googletest)
endif()

# ─── JSON ───────────────────────────────────────────────────────────────────────
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# ─── vecodex-index Library (INTERFACE) ──────────────────────────────────────────
add_library(vecodex-index INTERFACE)
target_link_libraries(vecodex-index INTERFACE faiss)
target_include_directories(vecodex-index INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/baselines>
    $<INSTALL_INTERFACE:include>
    external/faiss
)

# ─── Unit Tests ─────────────────────────────────────────────────────────────────
add_executable(VecodexIndexTest tests/VecodexIndexTest.cpp)
add_test(NAME test_index_basic COMMAND VecodexIndexTest)
target_include_directories(VecodexIndexTest PRIVATE ${PROJECT_SOURCE_DIR}/baselines)
target_link_libraries(VecodexIndexTest
    PRIVATE Boost::thread Boost::timer Boost::chrono
            nlohmann_json::nlohmann_json faiss gtest gtest_main
            OpenMP::OpenMP_CXX Threads::Threads ${OpenBLAS_LIBRARIES}
            vecodex-index
)
if(APPLE)
    target_link_libraries(VecodexIndexTest PRIVATE "-framework Accelerate")
    target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra)
    set_target_properties(VecodexIndexTest PROPERTIES INSTALL_RPATH "@loader_path/lib")
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(VecodexIndexTest PRIVATE pthread)
    target_compile_options(VecodexIndexTest PRIVATE -Wall -Wextra -fPIC)
    set_target_properties(VecodexIndexTest PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
endif()

# ─── Benchmark Executable ───────────────────────────────────────────────────────
add_executable(Benchmark tests/Benchmark.cpp)
target_include_directories(Benchmark PRIVATE ${PROJECT_SOURCE_DIR}/baselines)
target_link_libraries(Benchmark
    PRIVATE benchmark::benchmark benchmark::benchmark_main
            Boost::thread Boost::timer Boost::chrono
            nlohmann_json::nlohmann_json faiss gtest
            OpenMP::OpenMP_CXX Threads::Threads
            ${OpenBLAS_LIBRARIES} vecodex-index
)