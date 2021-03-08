#! /bin/bash

mkdir -p /service/build && \
    cd /service/build && \
    conan install -g virtualenv .. && \
    . ./activate.sh && \
    cmake .. && \
    cmake --build .