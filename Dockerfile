FROM ubuntu:20.04
MAINTAINER Andreas Bihlmaier <andreas.bihlmaier@gmx.de>

# Upgrade and install dependencies
RUN export DEBIAN_FRONTEND=noninteractive; \
    apt-get update \
    && apt-get -y upgrade \
    && apt-get -y install \
       build-essential \
       clang-tidy \
       cmake \
       cppcheck \
    && rm -rf /var/lib/apt/lists/*

COPY . /cpp_rt_pubsub

RUN cd /cpp_rt_pubsub \
    && rm -rf build \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make -j`nproc`