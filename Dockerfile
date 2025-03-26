FROM gcc:12

WORKDIR /

RUN apt-get update && \
    apt-get install -y cmake \
    libssl-dev \
    libcpprest-dev \
    libc++-dev
RUN apt-get install -y libboost-dev
RUN apt-get install -y libomp-dev \
    libopenblas-dev \
    libgtest-dev \
    libgflags-dev

COPY ./build.sh /
COPY ./app/CMakeLists.txt /app/
COPY ./app/examples/ /app/examples/
COPY ./app/proto/ /app/proto/
COPY ./app/external/ /app/external/
COPY ./app/include/ /app/include/
COPY ./app/src/ /app/src/
COPY ./app/configs/ /app/configs/

COPY ./index/CMakeLists.txt /index/
COPY ./index/baselines/ /index/baselines/
COPY ./index/external/ /index/external/
COPY ./index/include/ /index/include/
COPY ./index/tests/ /index/tests/

RUN cd /app/external/grpc/ && \
    mkdir -p cmake/build && \
    cd cmake/build && \
    cmake ../.. -DCMAKE_CXX_STANDARD=17 -DgRPC_INSTALL=ON -DCMAKE_INSTALL_PREFIX=/usr/local -DgRPC_SSL_PROVIDER=package && \
    make -j8 && \
    make -j8 install

RUN cd /app/external/aws-cpp-sdk/ && \
    mkdir -p cmake/build && \
    cd cmake/build && \
    cmake ../.. -DCPP_STANDARD=17 -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_TESTING=OFF -DBUILD_ONLY="s3" && \
    make -j8 && \
    make -j8 install
RUN ./build.sh

CMD ./app/cmake/build/vecodex-app coordinator
