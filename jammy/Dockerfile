#
# This file is generated from Dockerfile.in. Do not edit!
#

FROM ubuntu:jammy

LABEL org.opencontainers.image.authors="kees-jan@dijkzeul.eu"

RUN \
  export DEBIAN_FRONTEND=noninteractive && \
  apt-get update -y && \
  apt-get install -y \
    clang \
    cmake \
    gcc g++ \
    git \
    gpg \
    libboost-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libcairo2-dev \
    libglib2.0-dev \
    libgtest-dev \
    libgtk-3-dev \
    libtiff5-dev \
    ninja-build \
    python3 \
    python3-pip \
    software-properties-common \
    time \
    wget \
    xvfb

RUN \
  update-alternatives --install /usr/bin/python python /usr/bin/python3 30 \
    --slave /usr/bin/pip pip /usr/bin/pip3

CMD ["/bin/bash"]
