FROM gcc:latest

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

RUN ./build.sh

CMD ./build/vecodex-app coordinator
