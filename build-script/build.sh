#!/usr/bin/env bash

set -e

(
    cd submodules/bcg729
    cmake .
    make
    make install
)

(
    cd submodules/pjproject
    ./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-libwebrtc --disable-sound
    make dep && make
)

mkdir -p build
(
    cd build
    cmake ..
    cmake --build .
)
