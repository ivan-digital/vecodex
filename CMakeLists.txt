cmake_minimum_required(VERSION 3.12)

project(vecodex-index)

set(CMAKE_CXX_STANDARD 17)

set(HAVE_POSIX_REGEX 1)

# Compiler settings for macOS
if(APPLE)
    set(CMAKE_C_COMPILER "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

    # Get Homebrew prefixes
    execute_process(COMMAND brew --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

    # OpenMP Support for AppleClang
    set(OPENMP_INCLUDE_DIR "${LIBOMP_PREFIX}/include")
    set(OPENMP_LIB_DIR "${LIBOMP_PREFIX}/lib")
    set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_C_LIB_NAMES "omp")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY "${OPENMP_LIB_DIR}/libomp.dylib")
    set(OpenMP_C_LIBRARIES "-L${OPENMP_LIB_DIR} -lomp")
    set(OpenMP_CXX_LIBRARIES "-L${OPENMP_LIB_DIR} -lomp")

    include_directories(BEFORE "${OPENMP_INCLUDE_DIR}")
    add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
    set_target_properties(OpenMP::OpenMP_CXX PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${OpenMP_CXX_FLAGS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENMP_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${OpenMP_CXX_LIBRARIES}"
    )

    set(Threads_FOUND TRUE CACHE BOOL "" FORCE)

else()
    # For non-Apple platforms, use standard GCC or Clang settings
    find_package(Threads REQUIRED)
    find_package(OpenMP REQUIRED)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_C_LIBRARIES}")
endif()

# Link to threading and OpenMP libraries
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# OpenBLAS settings
execute_process(COMMAND brew --prefix openblas OUTPUT_VARIABLE OPENBLAS_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
file(GLOB OPENBLAS_LIBRARIES "${OPENBLAS_PREFIX}/lib/libopenblas*.dylib")
if(NOT OPENBLAS_LIBRARIES)
    message(FATAL_ERROR "Could not find OpenBLAS library in ${OPENBLAS_PREFIX}/lib")
endif()

list(GET OPENBLAS_LIBRARIES 0 BLAS_LIBRARIES)
set(BLAS_FOUND TRUE CACHE BOOL "BLAS found" FORCE)
set(LAPACK_FOUND TRUE CACHE BOOL "LAPACK found" FORCE)
set(BLAS_LIBRARIES "${BLAS_LIBRARIES}" CACHE STRING "BLAS libraries" FORCE)
set(LAPACK_LIBRARIES "${BLAS_LIBRARIES}" CACHE STRING "LAPACK libraries" FORCE)

# Disable Python and GPU support in FAISS
set(FAISS_ENABLE_PYTHON OFF CACHE BOOL "Disable Python support in FAISS")
set(FAISS_ENABLE_GPU OFF CACHE BOOL "Disable GPU support in FAISS")
set(FAISS_ENABLE_OPENMP ON CACHE BOOL "Enable OpenMP support in FAISS" FORCE)

# Submodules for FAISS and Google Test
add_subdirectory(external/faiss)
if (NOT TARGET gtest)
    add_subdirectory(external/googletest)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
add_library(vecodex-index
    src/VecodexIndex.cpp
    src/VecodexSegment.cpp
    src/DocumentMetadata.cpp
    include/VecodexIndex.h
    include/VecodexSegment.h
    include/DocumentMetadata.h
)

target_include_directories(vecodex-index PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(vecodex-index PRIVATE faiss OpenMP::OpenMP_CXX Threads::Threads "${BLAS_LIBRARIES}")

# Test executable
add_executable(VecodexIndexTest tests/VecodexIndexTest.cpp)
target_link_libraries(VecodexIndexTest PRIVATE faiss gtest gtest_main OpenMP::OpenMP_CXX Threads::Threads "${BLAS_LIBRARIES}")
