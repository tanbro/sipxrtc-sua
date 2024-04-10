# Ref: https://github.com/microsoft/vscode-dev-containers/blob/main/containers/cpp/README.md

# latest Debian GA
# Ref: https://mirrors.tuna.tsinghua.edu.cn/help/debian/
# Debian Buster 以上版本默认支持 HTTPS 源。如果遇到无法拉取 HTTPS 源的情况，请先使用 HTTP 源并安装：
# apt install apt-transport-https ca-certificates

FROM mcr.microsoft.com/vscode/devcontainers/cpp

ARG APT_SOURCES

RUN --mount=type=cache,target=/var/cache/apt \
    if [ -n "${APT_SOURCES}" ]; then printenv APT_SOURCES | tee /etc/apt/sources.list; fi && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt -o Acquire::https::Verify-Peer=false --allow-unauthenticated update && \
    apt --allow-unauthenticated install -y apt-transport-https ca-certificates && \
    apt --allow-unauthenticated install -y uuid-dev libsamplerate0-dev libgoogle-glog-dev
