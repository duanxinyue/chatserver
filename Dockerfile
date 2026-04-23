FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libmysqlclient-dev \
    libhiredis-dev \
    libspdlog-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libgtest-dev \
    libgmock-dev \
    openssl \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN cd /usr/src/googletest && \
    mkdir -p build && cd build && \
    cmake .. && make -j$(nproc) && \
    cp lib/libgtest*.a lib/libgmock*.a /usr/lib

RUN git clone https://github.com/chenshuo/muduo.git /tmp/muduo && \
    cd /tmp/muduo && \
    mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/muduo

WORKDIR /app

COPY . .

RUN mkdir -p build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && make -j$(nproc)

EXPOSE 6000

CMD ["./bin/ChatServer", "0.0.0.0", "6000"]
