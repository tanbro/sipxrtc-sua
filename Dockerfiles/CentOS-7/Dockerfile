# BuildKit required

FROM centos:7

# 修改 YUM 镜像，以下载的 CentOS-Base.repo 为准
RUN --mount=type=bind,source=Dockerfiles/CentOS-7,target=/root/build \
    yes | cp -fv /root/build/download/CentOS-*.repo /etc/yum.repos.d/

RUN --mount=type=bind,source=Dockerfiles/CentOS-7,target=/root/build \
    # yum cache
    --mount=type=cache,target=/var/cache/yum \
    set -e; \
    [ $(ls -l /root/build/CentOS-*.repo 2>/dev/null | wc -l) -gt 0 ] && yum_opts="--disableplugin=fastestmirror"; \
    # 安装开发工具套件
    yum groupinstall -y ${yum_opts} "Development Tools"; \
    # 安装其它几个必要的工具
    yum install -y ${yum_opts} wget;

# 需要版本更高的 CMAKE
# ARG CMAKE_BIN_DIST_URL=https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1-linux-x86_64.sh
# ARG CMAKE_BIN_DIST_FILE=cmake-3.23.1-linux-x86_64.sh
# ARG CMAKE_SUMMARY_FILE_URL=https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1-SHA-256.txt
# ARG CMAKE_SUMMARY_FILE=cmake-3.23.1-SHA-256.txt
ARG CMAKE_VERSION=3.23.1
RUN --mount=type=bind,source=Dockerfiles/CentOS-7,target=/root/build \
    # 自定义的下载 cache
    --mount=type=cache,target=/root/.cache/download \
    set -e; \
    ( \
    cd /root/.cache/download; \
    # 有缓存的 cmake dist 么？没有就下载
    cmake_bin_dist = $(echo "cmake-${CMAKE_VERSION}-$(echo $(uname -s) | tr '[:upper:]' '[:lower:]')-$(echo $(uname -m) | tr '[:upper:]' '[:lower:]').sh"); \
    [ ! -f ${cmake_bin_dist} ] && \
    wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${cmake_bin_dist}; \
    # 还没有 check sum 文件就下载
    [ ! -f cmake-${CMAKE_VERSION}-SHA-256.txt ] && \
    wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-SHA-256.txt; \
    # 校验
    sha256sum -c --ignore-missing cmake-${CMAKE_VERSION}-SHA-256.txt; \
    # 安装
    /bin/sh ${cmake_bin_dist}; \
    ); \
    # 打印 cmake 版本
    cmake --version;
