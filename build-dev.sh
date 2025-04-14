#!/bin/bash

cd /app/external/grpc/ && \
mkdir -p cmake/build && \
cd cmake/build && \
cmake ../.. -DCMAKE_CXX_STANDARD=17 \
            -DgRPC_INSTALL=ON \
            -DCMAKE_INSTALL_PREFIX=/usr/local \
            -DgRPC_SSL_PROVIDER=package \
            -DgRPC_PROTOBUF_PROVIDER=module \
            -DgRPC_ABSL_PROVIDER=module \
            -DgRPC_CARES_PROVIDER=module \
            -DgRPC_RE2_PROVIDER=module \
            -DgRPC_ZLIB_PROVIDER=package && \
make -j8 && \
make -j8 install

cd /app/external/aws-cpp-sdk/ && \
mkdir -p cmake/build && \
cd cmake/build && \
cmake ../.. -DCPP_STANDARD=17 \
            -DCMAKE_INSTALL_PREFIX=/usr/local\
            -DENABLE_TESTING=OFF \
            -DBUILD_ONLY="s3" && \
make -j8 && \
make -j8 install

mkdir /build
cd /build
cmake -DBUILD_SHARED_LIBS=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=/app \
      -DRE2_BUILD_TESTING=OFF \
      -DBUILD_VECODEX_TESTS=ON \
      /app
ldconfig
make -j8 vecodex-app
make -j8 vecodex_app_ut

echo "$1"
if [ "$1" = "coordinator" ]; then
      echo "Coordinator instance is starting"
      /build/vecodex-app --config-path /app/configs/coordinator_config.json
elif [ "$1" = "searcher" ]; then
      echo "Searcher instance is starting"
      /build/vecodex-app --config-path /app/configs/searcher_config.json
elif [ "$1" = "writer" ]; then
      echo "Writer instance is starting"
      /build/vecodex-app --config-path /app/configs/writer_config.json
else
      echo "Unknown argument. Start bash"
      /bin/bash
fi