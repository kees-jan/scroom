FROM ubuntu:vivid

MAINTAINER kees-jan.dijkzeul@iae.nl

RUN \
  export DEBIAN_FRONTEND=noninteractive && \
  apt-get update -y && \
  apt-get install -y \
    libboost-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-test-dev \
    libboost-thread-dev \
    libcairo2-dev \
    libglade2-dev \
    libglib2.0-dev \
    libgtk2.0-dev \
    libtiff-dev

CMD ["/bin/bash"]