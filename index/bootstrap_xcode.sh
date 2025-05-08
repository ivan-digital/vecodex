#!/usr/bin/env bash
#
# bootstrap_xcode.sh ─ generate an Xcode project for vecodex-index
#
# Usage
#   ./bootstrap_xcode.sh            # release
#   ./bootstrap_xcode.sh --debug    # debug
#   ./bootstrap_xcode.sh --source path/to/src
set -euo pipefail

### ───────────────────── options ─────────────────────
BUILD_TYPE="Release"
SOURCE_DIR=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --debug)   BUILD_TYPE="Debug"  ;;
    --release) BUILD_TYPE="Release";;
    --source)  SOURCE_DIR="$2"; shift;;
    *) echo "Unknown option: $1" >&2; exit 1;;
  esac
  shift
done

### ───────────────────── sanity ──────────────────────
command -v brew >/dev/null      || { echo "Homebrew missing"; exit 1; }
xcode-select -p >/dev/null 2>&1 || { echo "Xcode CLT missing"; exit 1; }

### ───────────────────── source dir ──────────────────
if [[ -z $SOURCE_DIR ]]; then
  [[ -f CMakeLists.txt       ]] && SOURCE_DIR="."
  [[ -f index/CMakeLists.txt ]] && SOURCE_DIR="index"
fi
[[ -z $SOURCE_DIR ]] && { echo "Cannot detect source dir"; exit 1; }

SOURCE_DIR=$(cd "$SOURCE_DIR" && pwd)
ROOT_DIR=$(pwd)

### ───────────────────── deps ────────────────────────
echo "[1/5] brew deps"
brew update
brew install llvm libomp openblas boost cmake git faiss || true   # gtest/benchmark fetched via CMake

### ───────────────────── sub-modules ─────────────────
echo "[2/5] git submodule"
git -C "$ROOT_DIR" submodule update --init --recursive

### ───────────────────── env ─────────────────────────
LLVM_PREFIX=$(brew --prefix llvm)
OPENBLAS_PREFIX=$(brew --prefix openblas)

export CC="${LLVM_PREFIX}/bin/clang"
export CXX="${LLVM_PREFIX}/bin/clang++"
export CMAKE_PREFIX_PATH="${LLVM_PREFIX};${OPENBLAS_PREFIX}"

### ───────────────────── configure ───────────────────
echo "[3/5] configure"
BUILD_DIR="$ROOT_DIR/build-xcode"
rm -rf "$BUILD_DIR"

cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Xcode \
      -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DCMAKE_C_COMPILER="$CC" \
      -DCMAKE_CXX_COMPILER="$CXX"

### ───────────────────── build  ──────────────────────
echo "[4/5] build"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -- -quiet

echo -e "\n✅  open \"$BUILD_DIR/$(basename "$SOURCE_DIR").xcodeproj\""