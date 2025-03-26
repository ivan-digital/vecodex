mkdir -p app/cmake/build
cd app/cmake/build
cmake -DBUILD_SHARED_LIBS=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DgRPC_PROTOBUF_PROVIDER=module \
      -DgRPC_ABSL_PROVIDER=module \
      -DgRPC_CARES_PROVIDER=module \
      -DgRPC_RE2_PROVIDER=module \
      -DgRPC_SSL_PROVIDER=package \
      -DgRPC_ZLIB_PROVIDER=module \
      -DCMAKE_INSTALL_PREFIX=/app \
      -DRE2_BUILD_TESTING=OFF \
      ../..
ldconfig
make -j8 vecodex-app
# make -j8 example_client
make -j8 etcd_client
