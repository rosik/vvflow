FROM ubuntu:xenial
# RUN echo "deb http://deb.debian.org/debian jessie contrib" >> /etc/apt/sources.list
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    cmake-curses-gui \
    git \
    libfl-dev \
    libgflags-dev \
    libhdf5-dev \
    liblapack-dev \
    liblua5.2-dev \
    libmatheval-dev \
    make \
    pkg-config \
    python-pip \
    ruby-ronn \
    && pip install grip \
    && apt-get clean

WORKDIR /root
CMD /bin/bash
