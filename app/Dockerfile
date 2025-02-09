FROM gcc:11

WORKDIR /app

RUN apt-get update && \
    apt-get install -y cmake \
    libssl-dev \ 
    libcpprest-dev \
    libc++-dev
RUN apt-get install -y libboost-dev

COPY build.sh CMakeLists.txt /app
COPY ./examples/ /app/examples/
COPY ./proto/ /app/proto
COPY ./external/ /app/external
COPY ./include/ /app/include
COPY ./src/ /app/src

RUN cd /app/external/grpc/ && \
    mkdir -p cmake/build && \
    cd cmake/build && \
    cmake ../.. -DCMAKE_CXX_STANDARD=17 -DgRPC_INSTALL=ON -DCMAKE_INSTALL_PREFIX=/ && \
    make -j8 && \
    make -j8 install
RUN ./build.sh

CMD ./build/vecodex-app coordinator
