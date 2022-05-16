# BuildKit required

FROM centos:7

# 修改 YUM 镜像，以下载的 CentOS-Base.repo 为准
RUN --mount=type=bind,target=/root/project \
    # yum cache
    --mount=type=cache,target=/var/cache/yum \
    set -e; \
    if [ $(ls -l /root/project/download/CentOS-*.repo 2>/dev/null | wc -l) -gt 0 ]; then \
    yes | cp -fv /root/project/download/CentOS-*.repo /etc/yum.repos.d/ ;\
    yum_opts="--disableplugin=fastestmirror" ;\
    fi ;\
    # 安装开发工具套件
    yum groupinstall -y ${yum_opts} "Development Tools" ;\
    # 安装其它几个必要的工具
    # 再就是几个系统可以提供的, PJ 的可选软件包
    yum install -y ${yum_opts} wget openssl-devel libuuid-devel libsamplerate-devel ;

# 需要版本更高的 CMAKE
ARG CMAKE_VERSION=3.23.1
RUN --mount=type=bind,target=/root/project \
    # 自定义的下载 cache
    --mount=type=cache,target=/root/.cache/download \
    set -e ;\
    cmake_bin_dist="cmake-${CMAKE_VERSION}-$(echo $(uname -s) | tr '[:upper:]' '[:lower:]')-$(echo $(uname -m) | tr '[:upper:]' '[:lower:]').sh" ;\
    cmake_checksum_file="cmake-${CMAKE_VERSION}-SHA-256.txt" ;\
    ( \
    cd /root/.cache/download ;\
    # WARNING: checksum 伪造风险
    # 有缓存的 cmake checksum 么？ cache 没有, 看看 HOST 映射的 /root/project/download 有没有
    [ ! -f ${cmake_checksum_file} ] \
    && [ ! f /root/project/download/${cmake_checksum_file} ] \
    && cp -fv /root/project/download/${cmake_checksum_file} . ;\
    # 还是没有就得重新下载 checksum 文件！
    [ ! -f ${cmake_checksum_file} ] && wget -O ${cmake_checksum_file} https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${cmake_checksum_file} ;\
    # 读取 dist 的 checksum
    checksum=$(cat ${cmake_checksum_file} | grep ${cmake_bin_dist}) ;\
    echo "SHA-256: ${checksum}" ;\
    # 有缓存的 cmake dist 么？ cache 没有, 看看 HOST 映射的 /root/project/download 有没有
    ( \
    set +e ;\
    echo "${checksum}" | sha256sum -c ;\
    [ $? -ne 0 ] && [ -f /root/project/download/${cmake_bin_dist} ] cp -fv /root/project/download/${cmake_bin_dist} . ;\
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

# 构建和安装 `bcg729`
RUN --mount=type=bind,target=/root/project \
    --mount=type=cache,target=/root/.cache/download \
    set -e ;\
    _url=https://github.com/BelledonneCommunications/bcg729/archive/refs/tags/1.1.1.tar.gz ;\
    _file=bcg729-1.1.1.tar.gz ;\
    _dir=bcg729-1.1.1 ;\
    ( \
    cd /root/.cache/download ;\
    [ ! -f  ${_file} ] \
    && [ -f /root/project/download/${_file} ] \
    && cp -av /root/project/download/${_file} . ;\
    [ ! -f  ${_file} ] && wget -O ${_file} ${_url} ;\
    mkdir -p /root/deps ;\
    tar -xf ${_file} -C /root/deps ;\
    ) ;\
    ( \
    cd /root/deps/${_dir} ;\
    cmake . ;\
    make ;\
    make install ;\
    ldconfig ;\
    ) ;\
    rm -rf /root/deps/${_dir} ;\
    pkg-config --modversion libbcg729 ;

# 构建 PJSIP
# 特殊: 要使用静态库，不要删除！
RUN --mount=type=bind,target=/root/project \
    --mount=type=cache,target=/root/.cache/download \
    set -e ;\
    _url=https://github.com/pjsip/pjproject/archive/refs/tags/2.12.1.tar.gz ;\
    _file=pjproject-2.12.1.tar.gz ;\
    _dir=pjproject-2.12.1 ;\
    ( \
    cd /root/.cache/download ;\
    [ ! -f  ${_file} ] \
    && [ -f /root/project/download/${_file} ] \
    && cp -av /root/project/download/${_file} . ;\
    [ ! -f  ${_file} ] && wget -O ${_file} ${_url} ;\
    mkdir -p /root/deps ;\
    tar -xf ${_file} -C /root/deps ;\
    ) ;\
    ( \
    cd /root/deps/${_dir} ;\
    ./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-libwebrtc --disable-sound ;\
    make dep && make clean && make ;\
    ) ;

# GFlags
RUN --mount=type=bind,target=/root/project \
    --mount=type=cache,target=/root/.cache/download \
    set -e ;\
    _url=https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.tar.gz ;\
    _file=gflags-2.2.2.tar.gz ;\
    _dir=gflags-2.2.2 ;\
    ( \
    cd /root/.cache/download ;\
    [ ! -f  ${_file} ] \
    && [ -f /root/project/download/${_file} ] \
    && cp -av /root/project/download/${_file} . ;\
    [ ! -f  ${_file} ] && wget -O ${_file} ${_url} ;\
    mkdir -p /root/deps ;\
    tar -xf ${_file} -C /root/deps ;\
    ) ;\
    ( \
    cd /root/deps/${_dir} ;\
    mkdir build && cd build ;\
    cmake -DBUILD_SHARED_LIBS=ON BUILD_gflags_LIBS=ON .. ;\
    make && make install ;\
    ) ;\
    rm -rf cd /root/deps/${_dir} ;\
    pkg-config --modversion gflags;

# GLog
RUN --mount=type=bind,target=/root/project \
    --mount=type=cache,target=/root/.cache/download \
    set -e ;\
    _url=https://github.com/google/glog/archive/refs/tags/v0.6.0.tar.gz ;\
    _file=glog-0.6.0.tar.gz ;\
    _dir=glog-0.6.0 ;\
    ( \
    cd /root/.cache/download ;\
    [ ! -f  ${_file} ] \
    && [ -f /root/project/download/${_file} ] \
    && cp -av /root/project/download/${_file} . ;\
    [ ! -f  ${_file} ] && wget -O ${_file} ${_url} ;\
    mkdir -p /root/deps ;\
    tar -xf ${_file} -C /root/deps ;\
    ) ;\
    ( \
    cd /root/deps/${_dir} ;\
    cmake -S . -B build ;\
    cmake --build build ;\
    cmake --build build --target install ;\
    ) ;\
    rm -rf cd /root/deps/${_dir} ;\
    pkg-config --modversion libglog;

# 本项目
RUN --mount=type=bind,target=/root/project \
    set -e ;\
    export PJDIR=/root/deps/pjproject-2.12.1 ;\
    mkdir -p /root/build && cd /root/build ;\
    cmake -DCMAKE_BUILD_TYPE=Release -DWITH_BCG729_STATIC=OFF /root/project ;\
    cmake --build ;
