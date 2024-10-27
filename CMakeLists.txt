# rm -rf CMakeCache.txt build CMakeFiles
# cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Debug .  

cmake_minimum_required(VERSION 3.12)

project(vecodex-index)

set(CMAKE_CXX_STANDARD 17)

set(HAVE_POSIX_REGEX 1)

# Ensure we're using AppleClang
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

# Get Homebrew prefixes
execute_process(COMMAND brew --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

# OpenMP Support for AppleClang
if(APPLE)
    # Ensure libomp is installed:
    # brew install libomp

    # Set OpenMP include and lib directories
    set(OPENMP_INCLUDE_DIR "${LIBOMP_PREFIX}/include")
    set(OPENMP_LIB_DIR "${LIBOMP_PREFIX}/lib")

    # Set OpenMP flags
    set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_C_LIB_NAMES "omp")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY "${OPENMP_LIB_DIR}/libomp.dylib")
    set(OpenMP_C_LIBRARIES "-L${OPENMP_LIB_DIR} -lomp")
    set(OpenMP_CXX_LIBRARIES "-L${OPENMP_LIB_DIR} -lomp")

    # Include OpenMP include directory globally
    include_directories(BEFORE "${OPENMP_INCLUDE_DIR}")

    # Create imported target for OpenMP::OpenMP_CXX
    add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
    set_target_properties(OpenMP::OpenMP_CXX PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${OpenMP_CXX_FLAGS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENMP_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${OpenMP_CXX_LIBRARIES}"
    )

    # Manually set OpenMP variables for FAISS
    set(OpenMP_C_FLAGS "${OpenMP_C_FLAGS}" CACHE STRING "" FORCE)
    set(OpenMP_C_LIB_NAMES "${OpenMP_C_LIB_NAMES}" CACHE STRING "" FORCE)
    set(OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS}" CACHE STRING "" FORCE)
    set(OpenMP_CXX_LIB_NAMES "${OpenMP_CXX_LIB_NAMES}" CACHE STRING "" FORCE)
    set(OpenMP_omp_LIBRARY "${OpenMP_omp_LIBRARY}" CACHE FILEPATH "" FORCE)
    set(OpenMP_C_INCLUDE_DIRS "${OPENMP_INCLUDE_DIR}" CACHE PATH "" FORCE)
    set(OpenMP_CXX_INCLUDE_DIRS "${OPENMP_INCLUDE_DIR}" CACHE PATH "" FORCE)
    set(OpenMP_C_LIB_DIRS "${OPENMP_LIB_DIR}" CACHE PATH "" FORCE)
    set(OpenMP_CXX_LIB_DIRS "${OPENMP_LIB_DIR}" CACHE PATH "" FORCE)
    set(OpenMP_C_LIBRARIES "${OpenMP_C_LIBRARIES}" CACHE STRING "" FORCE)
    set(OpenMP_CXX_LIBRARIES "${OpenMP_CXX_LIBRARIES}" CACHE STRING "" FORCE)
    set(OpenMP_FOUND TRUE CACHE BOOL "" FORCE)
else()
    find_package(OpenMP REQUIRED)
endif()

# Set the compiler and linker flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_C_LIBRARIES}")

# Cache the compiler flags so that subdirectories (like FAISS) can access them
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "" FORCE)

# Manually configure threading on macOS
if(APPLE)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    # Set Threads_FOUND to TRUE to avoid issues
    set(Threads_FOUND TRUE CACHE BOOL "" FORCE)
else()
    find_package(Threads REQUIRED)
endif()

# Get OpenBLAS prefix using Homebrew
execute_process(COMMAND brew --prefix openblas OUTPUT_VARIABLE OPENBLAS_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

# Find the OpenBLAS library file
file(GLOB OPENBLAS_LIBRARIES "${OPENBLAS_PREFIX}/lib/libopenblas*.dylib")

# Check if any library was found
if(NOT OPENBLAS_LIBRARIES)
    message(FATAL_ERROR "Could not find OpenBLAS library in ${OPENBLAS_PREFIX}/lib")
endif()

# Use the first found OpenBLAS library
list(GET OPENBLAS_LIBRARIES 0 BLAS_LIBRARIES)

# Optionally, display the library being used
message(STATUS "Using OpenBLAS library: ${BLAS_LIBRARIES}")

# Set BLAS and LAPACK variables in the cache for FAISS
set(BLAS_FOUND TRUE CACHE BOOL "BLAS found" FORCE)
set(LAPACK_FOUND TRUE CACHE BOOL "LAPACK found" FORCE)
set(BLAS_LIBRARIES "${BLAS_LIBRARIES}" CACHE STRING "BLAS libraries" FORCE)
set(LAPACK_LIBRARIES "${BLAS_LIBRARIES}" CACHE STRING "LAPACK libraries" FORCE)

# Disable Python support and GPU in FAISS
set(FAISS_ENABLE_PYTHON OFF CACHE BOOL "Disable Python support in FAISS")
set(FAISS_ENABLE_GPU OFF CACHE BOOL "Disable GPU support in FAISS")
set(BUILD_TESTING OFF CACHE BOOL "Disable testing in FAISS")
set(FAISS_ENABLE_OPENMP ON CACHE BOOL "Enable OpenMP support in FAISS" FORCE)

# Ensure FAISS uses the same compiler
set(FAISS_C_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "" FORCE)
set(FAISS_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "" FORCE)

# Add FAISS as a submodule
add_subdirectory(external/faiss)

# Add Google Test as a submodule only if not already defined
if (NOT TARGET gtest)
    add_subdirectory(external/googletest)
endif()

# Include directories for your project and Google Test
include_directories(external/googletest/googletest/include)

# Add main library target
add_library(vecodex-index
    src/VecodexIndex.cpp
    src/VecodexSegment.cpp
    src/DocumentMetadata.cpp
    include/VecodexIndex.h
    include/VecodexSegment.h
    include/DocumentMetadata.h
)

# Add header files explicitly
include_directories(${PROJECT_SOURCE_DIR}/include)
target_include_directories(vecodex-index PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link the FAISS library, OpenMP, threading libraries, and BLAS libraries
target_link_libraries(vecodex-index PRIVATE faiss OpenMP::OpenMP_CXX "${CMAKE_THREAD_LIBS_INIT}" "${BLAS_LIBRARIES}")

# Add Google Test executable
add_executable(VecodexIndexTest tests/VecodexIndexTest.cpp)

# Link Google Test libraries, FAISS, OpenMP, threading libraries, and BLAS libraries to the test executable
target_link_libraries(VecodexIndexTest PRIVATE faiss gtest gtest_main OpenMP::OpenMP_CXX "${CMAKE_THREAD_LIBS_INIT}" "${BLAS_LIBRARIES}")
