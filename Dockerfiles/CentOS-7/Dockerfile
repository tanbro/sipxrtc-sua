# BuildKit required

FROM centos:7

# 修改 YUM 镜像，以下载的 CentOS-Base.repo 为准
RUN --mount=type=bind,source=Dockerfiles/CentOS-7,target=/root/build \
    # yum cache
    --mount=type=cache,target=/var/cache/yum \
    set -e; \
    if [ $(ls -l /root/build/download/CentOS-*.repo 2>/dev/null | wc -l) -gt 0 ]; then \
    yes | cp -fv /root/build/download/CentOS-*.repo /etc/yum.repos.d/ ;\
    yum_opts="--disableplugin=fastestmirror" ;\
    fi ;\
    # 安装开发工具套件
    yum groupinstall -y ${yum_opts} "Development Tools" ;\
    # 安装其它几个必要的工具
    yum install -y ${yum_opts} wget ;

# 需要版本更高的 CMAKE
ARG CMAKE_VERSION=3.23.1
RUN --mount=type=bind,source=Dockerfiles/CentOS-7,target=/root/build \
    # 自定义的下载 cache
    --mount=type=cache,target=/root/.cache/download \
    set -e ;\
    cmake_bin_dist="cmake-${CMAKE_VERSION}-$(echo $(uname -s) | tr '[:upper:]' '[:lower:]')-$(echo $(uname -m) | tr '[:upper:]' '[:lower:]').sh" ;\
    cmake_checksum_file="cmake-${CMAKE_VERSION}-SHA-256.txt" ;\
    ( \
    cd /root/.cache/download ;\
    # WARNING: checksum 伪造风险
    # 有缓存的 cmake checksum 么？ cache 没有, 看看 HOST 映射的 /root/build/download 有没有
    [ ! -f ${cmake_checksum_file} ] && cp -fv /root/build/download/${cmake_checksum_file} . ;\
    # 还是没有就得重新下载 checksum 文件！
    [ ! -f ${cmake_checksum_file} ] && wget -O ${cmake_checksum_file} https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${cmake_checksum_file} ;\
    # 读取 dist 的 checksum
    checksum=$(cat ${cmake_checksum_file} | grep ${cmake_bin_dist}) ;\
    echo "SHA-256: ${checksum}" ;\
    # 有缓存的 cmake dist 么？ cache 没有, 看看 HOST 映射的 /root/build/download 有没有
    ( \
    set +e ;\
    echo "${checksum}" | sha256sum -c ;\
    [ $? -ne 0 ] && cp -fv /root/build/download/${cmake_bin_dist} . ;\
    : ;\
    ) ;\
    # 还是没有，就下载
    ( \
    set +e ;\
    echo "${checksum}" | sha256sum -c ;\
    [ $? -ne 0 ] && wget -O ${cmake_bin_dist} https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${cmake_bin_dist} ;\
    : ;\
    ); \
    # 最后一次校验，过了就安装
    echo "${checksum}" | sha256sum -c ;\
    sh ${cmake_bin_dist} --skip-license --prefix=/usr/local ;\
    ) ;\
    # 打印 cmake 版本
    cmake --version;
