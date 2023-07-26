# ubuntu 2004 LTS (focal)
FROM ubuntu:focal

ARG APT_SOURCES

RUN --mount=type=cache,target=/var/cache/apt \
    if [ -n "${APT_SOURCES}" ]; then printenv APT_SOURCES | tee /etc/apt/sources.list; fi && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get -o Acquire::https::Verify-Peer=false --allow-unauthenticated update && \
    apt-get --allow-unauthenticated install -y \
        git build-essential cmake libssl-dev uuid-dev libsamplerate0-dev libgoogle-glog-dev
