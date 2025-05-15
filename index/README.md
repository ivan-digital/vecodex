# Vecodex - Vector Search Index Library

The Vecodex Index Library is a C++ library providing efficient vector indexing and search capabilities. It leverages [FAISS](https://github.com/facebookresearch/faiss) for its core indexing structures and is designed for easy integration into C++ projects or as a component of the wider Vecodex distributed vector search cluster.

This library is an **INTERFACE library** in CMake terms. This means it doesn't compile into a single `.a` or `.so` file itself. Instead, when you link against `vecodex-index`, CMake ensures that your project correctly includes all necessary headers and links against its dependencies (like FAISS).

## Features

*   **High-Performance Indexing:** Utilizes FAISS for optimized speed and memory efficiency (supports HNSWFlat, IndexFlat, etc.).
*   **Scalable Search:** Capable of handling large-scale vector datasets.
*   **Flexible Integration:** Designed to be used as a CMake subdirectory.
*   **Interface-Driven Design:** Core functionalities are exposed through interfaces like `IIndex.h`.
*   **JSON Configuration:** Index instances can be configured and created via JSON objects using `IndexFactory.h`.
*   **Extensible:** Supports custom segment types and update/serialization callbacks.

## Prerequisites

*   **CMake:** Version 3.12 or higher.
*   **C++ Compiler:** Supporting C++17 (e.g., GCC, Clang).
*   **Git:** For cloning and managing submodules.
*   **OpenMP:** For parallel processing.
*   **Boost:** Specifically `thread`, `timer`, and `chrono` components.
*   **OpenBLAS:** For optimized linear algebra operations.

**macOS Specifics:**
*   The `CMakeLists.txt` is configured to find OpenMP (libomp) and OpenBLAS via Homebrew. Ensure you have them installed:
    ```zsh
    brew install libomp openblas llvm
    ```
    The build system will attempt to use the LLVM Clang compiler provided by Homebrew if it's found, for better OpenMP support.

## Dependencies

The library manages the following dependencies primarily through Git submodules and CMake's `FetchContent`:

*   **FAISS:** Core vector indexing. (Added as a CMake subdirectory from `external/faiss`)
*   **Googletest:** For unit testing. (Added as a CMake subdirectory from `external/googletest`)
*   **Google Benchmark:** For performance benchmarking. (Fetched via `FetchContent`)
*   **nlohmann/json:** For JSON parsing (used in `IndexFactory` and tests). (Fetched via `FetchContent`)

## Getting Started

### 1. Clone the Repository (if using standalone)

If you are using this library as part of the main Vecodex project, this step is likely already done. If standalone:

```zsh
git clone git@github.com:ivan-digital/vecodex.git
cd vecodex
```

### 2. Initialize and Update Git Submodules

This is crucial to fetch dependencies like FAISS and Googletest.
```zsh
# Navigate to the root of the index/ directory (or the standalone repo)
git submodule init
git submodule update --recursive
```

### 3. Building the Library and Tests

The library itself is an INTERFACE library, so building primarily means building its dependencies (like FAISS) and any associated executables (tests, benchmarks).

**General Build (Linux/macOS):**
```zsh
# From the index/ directory
mkdir build
cd build
cmake ..
make -j$(nproc) # For Linux
# make -j$(sysctl -n hw.ncpu) # For macOS
```
This will:
*   Configure the project and download `FetchContent` dependencies.
*   Build FAISS.
*   Build the `VecodexIndexTest` unit test executable.
*   Build the `Benchmark` executable.

**Building with Xcode (macOS):**
A helper script `bootstrap_xcode.sh` is provided to generate an Xcode project:
```zsh
# Navigate to the index/ directory
./bootstrap_xcode.sh
```
This will create an Xcode project in the `build-xcode/` directory. You can then open `vecodex-index.xcodeproj` in Xcode and build targets like `VecodexIndexTest` and `Benchmark` from there.

## Usage

### Integrating as a CMake Subdirectory (Recommended)

1.  **Add as Submodule (if not already part of your project):**
    ```zsh
    git submodule add git@github.com:ivan-digital/vecodex.git external/vecodex-index
    git submodule update --init --recursive
    ```

2.  **Add to your `CMakeLists.txt`:**

    See the example CMake configuration in [`index/example/CMakeLists.txt`](example/CMakeLists.txt) for how to add and link the Vecodex Index library in your project.

### API Overview & Example

The primary way to create and use an index is through the `IndexFactory.h` and the `vecodex::Index` class, which is templated based on the underlying FAISS index type.

**Key Headers:**
*   `index/include/IIndex.h`: Defines the main interface for index operations.
*   `index/include/Index.h`: The concrete `vecodex::Index` template class.
*   `index/include/IndexFactory.h`: Provides `vecodex::CreateIndex` to instantiate indexes from JSON configurations.
*   `index/include/Segment.h`: Represents a segment of the index.

**Conceptual Example (using `IndexFactory`):**

See the minimal working example in [`index/example/main.cpp`](example/main.cpp) and its build setup in [`index/example/CMakeLists.txt`](example/CMakeLists.txt) for a complete usage demonstration.

## Running Tests

### Unit Tests (GoogleTest)

The unit tests are defined in `tests/VecodexIndexTest.cpp` and built into an executable named `VecodexIndexTest`.

1.  **Build the tests:** Ensure you've run `cmake` and `make` in your build directory as described in "Building the Library". This will build the `VecodexIndexTest` target.

2.  **Run the tests:**
    *   **Using CTest (recommended):** Navigate to your build directory and run:
        ```zsh
        cd build # Or your chosen build directory
        ctest
        ```
        CTest will execute tests registered with `add_test()` in `CMakeLists.txt` (like `test_index_basic`).
    *   **Directly executing the test binary:**
        ```zsh
        cd build # Or your chosen build directory
        ./VecodexIndexTest # Or path_to_build_dir/VecodexIndexTest
        ```

### Benchmarks (Google Benchmark)

The benchmark tests are in `tests/Benchmark.cpp` and compiled into an executable named `Benchmark`.

1.  **Build the benchmark executable:**
    Ensure you've run `cmake` and `make` in your build directory. If you only want to build the benchmark:
    ```zsh
    cd build
    make Benchmark
    ```

2.  **Run the benchmarks:**
    The benchmark executable takes command-line arguments:
    ```
    ./Benchmark <N> <Q> <k> <dim> [enable_merge]
    ```
    Where:
    *   `<N>`: Total number of vectors to add during the benchmark.
    *   `<Q>`: Total number of queries to perform.
    *   `<k>`: Number of nearest neighbors to retrieve for each query.
    *   `<dim>`: Dimensionality of the vectors.
    *   `[enable_merge]`: Optional. Set to `1` to enable segment merging during additions, `0` (or omit) to disable.

    **Example:**
    ```zsh
    cd build # Or your chosen build directory
    ./Benchmark 10000 1000 10 128 1
    ```
    This will run the benchmark with 10,000 vectors, 1,000 queries, k=10, 128 dimensions, and segment merging enabled.
    The benchmark program outputs statistics to CSV files (e.g., `flat_stat_1.csv`, `hnsw_stat_1.csv`, `flat_srch_1.csv`, etc.) in the directory where it's run.

## Contributing

Please refer to the main `CONTRIBUTING.md` and `DCO.md` in the root of the Vecodex project.

## License

Vecodex is licensed under the [LICENSE](../../LICENSE) file in the root directory of this source tree.

# Vecodex Index Library

## Quick Start

**Requirements:**
- CMake >= 3.12
- A C++17 compiler
- [Boost](https://www.boost.org/) (including headers such as `asio.hpp`)
- OpenMP, OpenBLAS, and other dependencies as listed in the CMake configuration

**Clone and build:**

```sh
git clone --recursive git@github.com:ivan-digital/vecodex.git
cd vecodex/index
cmake .
make -j$(sysctl -n hw.ncpu)
```

> **Note:** You must build from the top-level `index` directory. All dependencies (including nlohmann_json and Boost) are set up by the top-level CMakeLists.txt.

## Testing and Benchmarking

To run unit tests and benchmarks:

```sh
make VecodexIndexTest
./VecodexIndexTest
make Benchmark
./Benchmark
```

## Troubleshooting

- If you see errors about missing Boost headers (e.g., `boost/asio.hpp`), ensure Boost is installed and discoverable by CMake. On macOS, you can install it with `brew install boost`.
- If you see errors about missing `nlohmann_json`, ensure you are building from the top-level `index` directory so CMake fetches all dependencies.

## Submodules

If you did not use `--recursive` when cloning, initialize submodules with:

```sh
git submodule update --init --recursive
```

## License

See [LICENSE](../LICENSE).
