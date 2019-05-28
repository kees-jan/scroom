FROM ubuntu:disco

MAINTAINER kees-jan.dijkzeul@iae.nl

RUN \
  export DEBIAN_FRONTEND=noninteractive && \
  apt-get update -y && \
  apt-get install -y \
    autoconf automake libtool \
    libboost-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-test-dev \
    libboost-thread-dev \
    libgtest-dev \
    libgmock-dev \
    google-mock \
    libcairo2-dev \
    libglade2-dev \
    libglib2.0-dev \
    libgtk2.0-dev \
    libtiff-dev \
    time

CMD ["/bin/bash"]
