# SIPUAC

一个实验性质的 SIPUAC，基于 PJPROJECT。目标是:

- 将/从自定义的内存块文件/流或者其它什么东西作为声音的IO

## 实现原理

参照 <https://trac.pjsip.org/repos/wiki/media-flow> 的说明：

![media-flow](http://www.pjsip.org/images/media-flow.jpg)

我们如果实习一个自定义虚拟设备，就可以达到上述目的。

进而按照 <https://trac.pjsip.org/repos/wiki/External_Sound_Device> 的说明，我们应在构建 `PJPROJECT` 的时候，指定 `--enable-ext-sound` 参数，然后编译和连接自定义设备的实现。

<https://trac.pjsip.org/repos/wiki/Audio_Dev_API> 简单说明了自定义设备的接口。

## PJPROJECT 的构建

由于我们需要使用自定义设备，且只需要音频部分，所以 PJPROJECT 的构建步骤与默认情况稍有不同，我们应:

1. configure:

   ```bash
   ./configure --disable-video --enable-ext-sound
   ```
