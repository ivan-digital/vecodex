FROM gcc:latest

WORKDIR /app

RUN apt-get update && \
    apt-get install -y cmake

COPY build.sh CMakeLists.txt /app
COPY ./examples/ /app/examples/
COPY ./proto/ /app/proto
COPY ./external/ /app/external
COPY ./include/ /app/include
COPY ./src/ /app/src

RUN ls -la

RUN ./build.sh

CMD ./build/vecodex-app coordinator
