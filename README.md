# SIPUAC

一个实验性质的 SIPUAC，基于 PJPROJECT。目标是:

- 将/从自定义的内存块文件/流或者其它什么东西作为声音的IO

PJ 的声音管道：

参照 <https://trac.pjsip.org/repos/wiki/media-flow> 的说明：

![media-flow](http://www.pjsip.org/images/media-flow.jpg)

## 第一个思路 - 空设备

让 PJ 使用“空声音设备”，然后利用录音、放音的 AudioPort 组合达到上述目的。

> 参考:
>
> - `pjmedia/include/wav_port.h`, `pjmedia/src/wav_player.h`, `pjmedia/src/wav_writer.h` 实现了文件读写 AudioPort
> - `submodules/pjproject/pjsip/src/pjsua-lib/pjsua_aud.c` 的 `pjsua_recorder_create` 等，对上述 AudioPort 的调用
> - `pjsip/src/pjsua2/media.cpp` 的 `AudioMediaRecorder` 与 `AudioMediaPlayer` 调用 `pjsua` 提供的接口。

`pjmedia/include/pjmedia/mem_port.h` 似乎可以做这件事

configure 的时候，指定 `--disable-sound` 参数让 PJ 使用“空声音设备”。另外，由于只需要音频部分，所以 PJ 的构建步骤与默认情况稍有不同，我们应这样构建:

```bash
cd submodules/pjproject
./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-ipp --disable-libwebrtc --disable-sound
make dep && make
```

## 第二个思路 - 自定义设备

参照 <https://trac.pjsip.org/repos/wiki/External_Sound_Device> 与 <https://trac.pjsip.org/repos/wiki/Audio_Dev_API#PortedDevices>

在构建时指定 `--enable-ext-sound`，然后模仿 `pjmedia/src/pjmedia-audiodev/alsa_dev.c`, `pjmedia/src/pjmedia-audiodev/null_dev.c` 等实现我们所需要的自定义设备，而后修改 `/pjmedia/src/pjmedia-audiodev/audiodev.c` ，让 PJ 加载这个设备。

```bash
cd submodules/pjproject
./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-ipp --disable-libwebrtc --enable-ext-sound
make dep && make
```
