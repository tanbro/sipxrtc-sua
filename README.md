# SIPUAC

一个实验性质的 SIPUAC，基于 PJPROJECT。目标是:

- 将/从自定义的内存块文件/流或者其它什么东西作为声音的IO

## 音频流的获取与推送

`pjproject` 的声音管道：

参照 <https://trac.pjsip.org/repos/wiki/media-flow> 的说明：

![media-flow](http://www.pjsip.org/images/media-flow.jpg)

### 第一个思路 - 空设备

让 `pjproject` 使用“空声音设备”，然后利用录音、放音的 AudioPort 组合达到上述目的。

> 参考:
>
> - `pjmedia/include/wav_port.h`, `pjmedia/src/wav_player.h`, `pjmedia/src/wav_writer.h` 实现了文件读写 AudioPort
> - `submodules/pjproject/pjsip/src/pjsua-lib/pjsua_aud.c` 的 `pjsua_recorder_create` 等，对上述 AudioPort 的调用
> - `pjsip/src/pjsua2/media.cpp` 的 `AudioMediaRecorder` 与 `AudioMediaPlayer` 调用 `pjsua` 提供的接口。

`pjmedia/include/pjmedia/mem_port.h` 似乎可以做这件事

configure 的时候，指定 `--disable-sound` 参数让 `pjproject` 使用“空声音设备”。

### 第二个思路 - 自定义设备

参照 <https://trac.pjsip.org/repos/wiki/External_Sound_Device> 与 <https://trac.pjsip.org/repos/wiki/Audio_Dev_API#PortedDevices>

在构建时指定 `--enable-ext-sound`，然后模仿 `pjmedia/src/pjmedia-audiodev/alsa_dev.c`, `pjmedia/src/pjmedia-audiodev/null_dev.c` 等实现我们所需要的自定义设备，而后修改 `/pjmedia/src/pjmedia-audiodev/audiodev.c` ，让 `pjproject` 加载这个设备。

## 搭建开发环境

### Develop on Ubuntu 20.04 LTS (focal)

1. 安装开发工具

   ```bash
   sudo apt install build-essential
   ```

2. 安装项目构建工具

   ```bash
   sudo apt install cmake
   ```

3. 如果尚未从远端获取 submodule 代码, 应进行一次初始化更新:

   ```bash
   git submodules foreach update --init
   ```

4. 构建和安装 `bcg729`

   由于 G.729 在 CTI 领域十分重要，我们认为这是一个**必选项**。

   `pjproject` 可以使用 bcg729 (<https://github.com/BelledonneCommunications/bcg729>)，它已经被加入到了 submodules.

   我们首先将这个 submodule 检出到最近的发行版（目前是 1.1.1）:

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

5. 构建和安装 `pjproject`

   1. 安装 `pjproject` 的开发依赖包:

      1. 可使用系统包管理器安装的:

         ```bash
         sudo apt install libssl-dev uuid-dev libopus-dev
         ```

         > 说明:
         >
         > - 如需使用 srtp, SIP over TLS 等网络安全特性，则安装 `libssl-dev`；否则不用。
         > - 如需使用 `opus` 音频编码，则安装 `libopus-dev`；否则不用。

      2. 需要通过源代码安装的:

         - `bcg729`: 在之前的步骤中已经安装

   2. 编译 `pjproject`

      检出到最近的发布版(目前是 2.12)

      ```bash
      cd submodules/pjproject
      git checkout -b release-2.12 2.12
      ```

      然后进行 `confgiure` 和 `make`。调用 `./configure` 的时候，注意需要指定 `--disable-sound` 参数让 `pjproject` 使用“空声音设备”。另外，由于只需要音频部分，还可关闭许多视频相关部分的配置:

      ```bash
      ./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-libwebrtc --disable-sound
      make dep && make
      ```

      这个应用默认使用静态链接的方式调用 `pjproject`，所以如无必要，不需安装到系统。

6. 构建该项目(`sipxsua` 执行文件)

   1. 安装其依赖开发包:

      1. 全部可使用系统包管理器安装:

         ```bash
         sudo apt install libsamplerate0-dev libgoogle-glog-dev
         ```

   2. 使用 CMake 构建

      应单独建立一个名为 `build` 的目录，专用于构建:

      ```bash
      mkdir build
      cmake -S build -B build
      cmake --build build
      ```

      构建得到的可执行/库文件等输出在 `out` 目录。

## 发布到目标系统

### Deploy on Ubuntu 20.04 LTS (focal)

可使用系统包管理器进行安装的:

```bash
sudo apt install libssl1.1 libuuid1 libopus0 libsamplerate0 libgoogle-glog0v5
```
