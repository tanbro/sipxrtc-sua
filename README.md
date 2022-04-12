# SIPUAC

一个实验性质的 SIPUAC，基于 PJPROJECT。目标是:

- 将/从自定义的内存块文件/流或者其它什么东西作为声音的IO

## 实现原理

参照 <https://trac.pjsip.org/repos/wiki/media-flow> 的说明：

![media-flow](http://www.pjsip.org/images/media-flow.jpg)

让 PJ 使用“空声音设备”，然后利用录音、放音的 AudioPort 组合达到上述目的。

我们可以仿造 `pjmedia/include/wav_port.h`, `pjmedia/src/wav_player.h`, `pjmedia/src/wav_writer.h` 制作一个针对第三方的路放音 AudioPort；仿造 `pjsip/src/pjsua2/media.cpp` 的 `AudioMediaRecorder` 与 `AudioMediaPlayer` 调用上述 Port。

## PJPROJECT 的构建

configure 的时候，指定 `--disable-sound` 参数让 PJ 使用“空声音设备”。另外，由于只需要音频部分，所以 PJ 的构建步骤与默认情况稍有不同，我们应这样构建:

```bash
cd submodules/pjproject
./configure --disable-video --disable-libyuv --disable-sdl --disable-ffmpeg --disable-v4l2 --disable-openh264 --disable-vpx --disable-ipp --disable-libwebrtc --enable-ext-sound
make dep && make
```
