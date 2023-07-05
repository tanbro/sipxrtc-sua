# ubuntu 2004 LTS (focal)
FROM ubuntu:20.04

ARG APT_SOURCES

RUN --mount=type=cache,target=/var/cache/apt \
    if [ -n "${APT_SOURCES}" ]; then printenv APT_SOURCES | tee /etc/apt/sources.list; fi && \
    DEBIAN_FRONTEND=noninteractive apt-get -o Acquire::https::Verify-Peer=false --allow-unauthenticated update

RUN --mount=type=cache,target=/var/cache/apt \
    DEBIAN_FRONTEND=noninteractive apt-get --allow-unauthenticated install -y git build-essential cmake

RUN --mount=type=cache,target=/var/cache/apt \
    DEBIAN_FRONTEND=noninteractive apt-get --allow-unauthenticated install -y libssl-dev uuid-dev libsamplerate0-dev libgoogle-glog-dev
