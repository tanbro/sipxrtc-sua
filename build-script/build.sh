#!/usr/bin/env bash

set -e

(
    cd /workspace/submodules/bcg729
    cmake . && make make install
)

(
    cd /workspace/submodules/pjproject
    ./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-libwebrtc --disable-sound
    make dep && make clean && make
)

mkdir -p /workspace/build
(
    cd /workspace/build
    cmake ..
    cmake --build .
)
