# SIPUAC

一个实验性质的 SIPUAC，基于 PJPROJECT。目标是:

- 将/从自定义的内存块文件/流或者其它什么东西作为声音的IO

## PJ 的常见 Audio 编码

### G.729

由于 G.729 在 CTI 领域十分重要，我们认为这是一个**必选项**。

PJ 可以使用 bcg729 (<https://github.com/BelledonneCommunications/bcg729>)，它已经被加入到了 submodules.
我们首先将这个 submodule 检出到最近的发行版（目前是 1.1.1）:

```bash
cd submodules/bcg729
git checkout -b release-1.1.1 1.1.1
```

然后按照其 README 说明进行构建和安装:

```bash
cmake .
make
sudo make install
```

### opus

由于 opus 在 CTI 领域并不常见，我们认为这是一个*可选项*

在 Ubuntu 2004 下，只需:

```bash
sudo install libopus-dev
```

## 音频流的获取与推送

PJ 的声音管道：

参照 <https://trac.pjsip.org/repos/wiki/media-flow> 的说明：

![media-flow](http://www.pjsip.org/images/media-flow.jpg)

### 第一个思路 - 空设备

让 PJ 使用“空声音设备”，然后利用录音、放音的 AudioPort 组合达到上述目的。

> 参考:
>
> - `pjmedia/include/wav_port.h`, `pjmedia/src/wav_player.h`, `pjmedia/src/wav_writer.h` 实现了文件读写 AudioPort
> - `submodules/pjproject/pjsip/src/pjsua-lib/pjsua_aud.c` 的 `pjsua_recorder_create` 等，对上述 AudioPort 的调用
> - `pjsip/src/pjsua2/media.cpp` 的 `AudioMediaRecorder` 与 `AudioMediaPlayer` 调用 `pjsua` 提供的接口。

`pjmedia/include/pjmedia/mem_port.h` 似乎可以做这件事

configure 的时候，指定 `--disable-sound` 参数让 PJ 使用“空声音设备”。
另外，由于只需要音频部分，还可关闭许多视频相关部分的配置。
所以，我们应这样构建:

```bash
cd submodules/pjproject
./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-libwebrtc --disable-sound
make dep && make
```

### 第二个思路 - 自定义设备

参照 <https://trac.pjsip.org/repos/wiki/External_Sound_Device> 与 <https://trac.pjsip.org/repos/wiki/Audio_Dev_API#PortedDevices>

在构建时指定 `--enable-ext-sound`，然后模仿 `pjmedia/src/pjmedia-audiodev/alsa_dev.c`, `pjmedia/src/pjmedia-audiodev/null_dev.c` 等实现我们所需要的自定义设备，而后修改 `/pjmedia/src/pjmedia-audiodev/audiodev.c` ，让 PJ 加载这个设备。

```bash
cd submodules/pjproject
./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-ipp --disable-libwebrtc --enable-ext-sound
make dep && make
```

## 开发环境

### Develop on Ubuntu 2004 LTS

1. 开发工具

   ```bash
   sudo apt install build-essential
   ```

1. 依赖软件开发包

   ```bash
   sudo apt install libsamplerate0-dev libssl-dev uuid-dev
   ```
