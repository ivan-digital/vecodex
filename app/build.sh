rm -rf generated
mkdir -p build
cd build
cmake -DgRPC_INSTALL=ON \
      -DBUILD_SHARED_LIBS=yes \
      -DgRPC_BUILD_TESTS=OFF \
      -DgRPC_PROTOBUF_PROVIDER=module \
      -DgRPC_ABSL_PROVIDER=module \
      -DgRPC_CARES_PROVIDER=module \
      -DgRPC_RE2_PROVIDER=module \
      -DgRPC_SSL_PROVIDER=module \
      -DgRPC_ZLIB_PROVIDER=module \
      -DCMAKE_INSTALL_PREFIX=/app \
      -DRE2_BUILD_TESTING=OFF \
      ..
make -j8 vecodex-app
# make -j8 example_client
# make -j8 etcd_client