# SIPXSUA

## 概述

[sipx][] 的 [SIP][] 软电话。

这个程序用于 [sipx][]，它的功能是：

- 启动后向指定的 SIP URI 发起音频呼叫。
- 接通后，通过 IPC 将从对端收到的 PCM 音频流发送到指定的位置。
- 接通后，通过 IPC 从指定位置读取 PCM 音频流，并发送给对端。

它基于 [PJSIP][]

## 关于开源

> 该项目、代码不可用于商业目的；如需进行商业应用，请联系作者。该项目允许个人学习，修改和使用。基于该项目开发的项目、代码需要开放源代码，且在项目的根目录提供该文件并注明变更。不得利用该项目申请专利、著作权或任何其它资质、权利；不得将该项目用于商标或商业文件，也不得暗示使用。该项目的作者与拥有者不承担使用其后带来的义务，也不承诺提供后期维护。广州市和声信息技术有限公司拥有该项目的全部权利以及该协议的最终解释权。

[sipx][] 的开发手册: <https://sipx.cn/docs/develop-guide/>

通过这个项目，学习者可以了解:

1. 关于 [SIP][] 协议的一些常识
1. 使用 CMake 对 C++ 项目进行构建和依赖管理
1. 使用 Git Submodules 引入较大的第三方库的源码
1. 一些 Linux 常见功能的用法，比如 Unix domain socket(aka UDS), Linux poll 等
1. 一些常见的通用 C++ libraries 的用法，例如 gFlags, gLog
1. 使用 Docker/Compose 进行跨平台构建

## 音频流控制的实现原理

[PJSIP][] 的声音管道：

参照 <https://trac.pjsip.org/repos/wiki/media-flow> 的说明：

![media-flow](http://www.pjsip.org/images/media-flow.jpg)

1. 第一个思路 - 空设备

   让 [PJSIP][] 使用“空声音设备”，然后利用录音、放音的 AudioPort 组合达到上述目的。

   > 参考:
   >
   > - `pjmedia/include/wav_port.h`, `pjmedia/src/wav_player.h`, `pjmedia/src/wav_writer.h` 实现了文件读写 `AudioPort`
   > - `submodules/pjproject/pjsip/src/pjsua-lib/pjsua_aud.c` 的 `pjsua_recorder_create` 等，对上述 `AudioPort` 的调用
   > - `pjsip/src/pjsua2/media.cpp` 的 `AudioMediaRecorder` 与 `AudioMediaPlayer` 调用 `pjsua` 提供的接口。

   `pjmedia/include/pjmedia/mem_port.h` 似乎可以做这件事

   `configure` 的时候，指定 `--disable-sound` 参数让 [PJSIP][] 使用“空声音设备”。

1. 第二个思路 - 自定义设备

   参照 <https://trac.pjsip.org/repos/wiki/External_Sound_Device> 与 <https://trac.pjsip.org/repos/wiki/Audio_Dev_API#PortedDevices>

   在构建时指定 `--enable-ext-sound`，然后模仿 `pjmedia/src/pjmedia-audiodev/alsa_dev.c`, `pjmedia/src/pjmedia-audiodev/null_dev.c` 等实现我们所需要的自定义设备，而后修改 `/pjmedia/src/pjmedia-audiodev/audiodev.c` ，让 [PJSIP][] 加载这个设备。

最终，我们采用了**第一种方法**。

## 搭建开发环境

### Develop on Ubuntu 20.04 LTS (focal)

1. 安装开发工具

   ```bash
   sudo apt install build-essential
   ```

1. 安装项目构建工具

   ```bash
   sudo apt install cmake
   ```

1. 如果尚未从远端获取 submodule 代码, 应进行一次初始化更新:

   ```bash
   git submodule update --init --recursive
   ```

1. 构建和安装 `bcg729`

   由于 [G.729][] 在 CTI 领域十分重要，我们认为这是一个**必选项**。

   [PJSIP][] 可以使用 [bcg729](https://github.com/BelledonneCommunications/bcg729)，它已经被加入到了 submodules.

   我们首先将这个 `submodule` 检出到支持的最新发布版（目前是 1.1.1）:

   ```bash
   cd submodules/bcg729
   git checkout -b release-1.1.1 1.1.1
   ```

   然后按照其 [README](submodules/bcg729/README.md) 的说明进行构建和安装:

   ```bash
   cmake .
   make
   sudo make install
   ```

1. 构建和安装 [PJSIP][]

   1. 安装 [PJSIP][] 的开发依赖包:

      1. 可使用系统包管理器安装的:

         ```bash
         sudo apt install libssl-dev uuid-dev
         ```

         > 说明:
         >
         > - 如需使用 `srtp`, `SIP over TLS` 等网络安全特性(**强烈推荐**)，则安装 `libssl-dev`；否则不用。
         >
         > - 其它编码，如需使用 `opus` 音频编码，则安装 `libopus-dev`；否则不用。
         >
         >   `opus` 是默认支持多 Channel Audio 的，而我们的 Mix 与 Resample 还没有为 multiple channels 做好准备，所以不建议使用！

      1. 需要通过源代码安装的:

         - `bcg729`: 在之前的步骤中已经安装

   1. 编译 [PJSIP][]

      检出到支持的最新发布版(目前是 2.12)

      ```bash
      cd submodules/pjproject
      git checkout -b release-2.12 2.12
      ```

      按照其 [README](submodules/pjproject/README.txt) 说明进行 `configure` 和 `make`。

      > 注意:
      >
      > - 使用 `--disable-sound` 参数，让 [PJSIP][] 使用“空声音设备”。这是这个项目所**必须**的。
      > - 由于只需要音频部分，可关闭许多视频相关部分的配置

      完整的构建命令是:

      ```bash
      ./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-libwebrtc --disable-sound
      make dep && make clean && make
      ```

      本项目默认使用 `submodules/pjproject` 子目录的的相对路径静态链接 [PJSIP][]，故不必安装到系统。

1. 构建该项目(`sipxsua` 执行文件)

   1. 安装其依赖开发包:

      1. 全部可使用系统包管理器安装:

         ```bash
         sudo apt install libsamplerate0-dev libgoogle-glog-dev
         ```

   1. 使用 CMake 构建

      应单独建立一个名为 `build` 的目录，专用于构建:

      ```bash
      mkdir build && cd build
      cmake ..
      make build
      ```

      构建得到的可执行/库文件等输出在 `out` 目录。

### Develop on CentOS 7

1. 安装开发工具

   ```bash
   yum groupinstall "Development Tools"
   ```

1. 安装项目构建工具，我们版本高于系统所提供的 CMake

   按照 CMake 官方给出的安装说明，安装 `CMake v3.23.1`:

   ```bash
   export CMAKE_VERSION=3.23.1
   cmake_bin_dist="cmake-${CMAKE_VERSION}-$(echo $(uname -s) | tr '[:upper:]' '[:lower:]')-$(echo $(uname -m) | tr '[:upper:]' '[:lower:]').sh"
   cmake_checksum_file="cmake-${CMAKE_VERSION}-SHA-256.txt"
   wget ${cmake_checksum_file} https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${cmake_checksum_file}
   wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${cmake_bin_dist}
   checksum=$(cat ${cmake_checksum_file} | grep ${cmake_bin_dist})
   echo "${checksum}" | sha256sum -c
   sh ${cmake_bin_dist} --skip-license --prefix=/usr/local
   ```

   安装完毕后，检查其版本:

   ```bash
   $ cmake --version
   cmake version 3.23.1

   CMake suite maintained and supported by Kitware (kitware.com/cmake).
   ```

1. 如果尚未从远端获取 submodule 代码, 应进行一次初始化更新:

   ```bash
   git submodule update --init --recursive
   ```

1. 构建和安装 `bcg729`

   由于 [G.729][] 在 CTI 领域十分重要，我们认为这是一个**必选项**。

   [PJSIP][] 可以使用 [bcg729](https://github.com/BelledonneCommunications/bcg729)，它已经被加入到了 submodules.

   我们首先将这个 `submodule` 检出到支持的最新发布版（目前是 1.1.1）:

   ```bash
   cd submodules/bcg729
   git checkout -b release-1.1.1 1.1.1
   ```

   然后按照其 [README](submodules/bcg729/README.md) 的说明进行构建和安装:

   ```bash
   cmake .
   make
   sudo make install
   ```

1. 构建和安装 [PJSIP][]

   1. 安装 [PJSIP][] 的开发依赖包:

      1. 可使用系统包管理器安装的:

         ```bash
         yum install openssl-devel libuuid-devel
         ```

      1. 需要通过源代码安装的:

         - `bcg729`: 在之前的步骤中已经安装

   1. 编译 [PJSIP][]

      检出到支持的最新发布版(目前是 2.12)

      ```bash
      cd submodules/pjproject
      git checkout -b release-2.12 2.12
      ```

      按照其 [README](submodules/pjproject/README.txt) 说明进行 `configure` 和 `make`。

      > 注意:
      >
      > - 使用 `--disable-sound` 参数，让 [PJSIP][] 使用“空声音设备”。这是这个项目所**必须**的。
      > - 由于只需要音频部分，可关闭许多视频相关部分的配置

      完整的构建命令是:

      ```bash
      ./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-libwebrtc --disable-sound
      make dep && make clean && make
      ```

      本项目默认使用 `submodules/pjproject` 子目录的的相对路径静态链接 [PJSIP][]，故不必安装到系统。

1. 构建该项目(`sipxsua` 执行文件)

   1. 安装其依赖开发包:

      1. 可使用系统包管理器安装的:

         ```bash
         sudo apt install libsamplerate-devel
         ```

      1. 需要从源代码构建的:

         1. google-gflags:

            ```bash
            wget https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.tar.gz
            tar -xf gflags-2.2.2.tar.gz
            cd gflags-2.2.2
            mkdir build && cd build
            cmake -DBUILD_SHARED_LIBS=ON ..
            make && make install
            ```

   1. 使用 CMake 构建

      应单独建立一个名为 `build` 的目录，专用于构建:

      ```bash
      mkdir build && cd build
      cmake ..
      make
      ```

      构建得到的可执行/库文件等输出在 `out` 目录。

## 发布到目标系统

### Deploy on Ubuntu 20.04 LTS (focal)

可使用系统包管理器进行安装的:

```bash
sudo apt install libssl1.1 libuuid1 libsamplerate0 libgoogle-glog0v5
```

### Deploy on Centos 7

可使用系统包管理器进行安装的:

```bash
sudo apt install openssl-libs libuuid libsamplerate
```

需要从源代码构建的:

- gflags
- glog

见上文

## 使用 Docker 构建

项目的 `build-script` 目录下有一系列基于 `docker-compose` 的构建脚本，可以直接构建出 `ubuntu:focal` 兼容的 binary 目标。

```bash
cd build-script
docker compose up
```

> **Note:**
>
> 使用 `docker` 构建前仍需要同步 submodule 代码, 并检出到正确的版本。

----

其它？没写完。

这个程序本身怎么发布？还没有想好，以后再说吧。

[SIP]: https://www.ietf.org/rfc/rfc3261.txt "SIP: Session Initiation Protocol"
[PJSIP]: https://www.pjsip.org/ "PJSIP is a free and open source multimedia communication library written in C language implementing standard based protocols such as SIP, SDP, RTP, STUN, TURN, and ICE. It combines signaling protocol (SIP) with rich multimedia framework and NAT traversal functionality into high level API that is portable and suitable for almost any type of systems ranging from desktops, embedded systems, to mobile handsets."
[G.729]: https://www.itu.int/rec/T-REC-G.729 "G.729 : Coding of speech at 8 kbit/s using conjugate-structure algebraic-code-excited linear prediction (CS-ACELP)"
[sipx]: https://sipx.cn/ "实现互联网音视频和SIP话路的互联互通"
