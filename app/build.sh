rm -rf generated
mkdir -p build
cd build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=/home/evgeniy-pon/course-work \
      ..
make -j8 vecodex-app
make -j8 example_client