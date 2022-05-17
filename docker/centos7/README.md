# SIPxSUA CentOS-7 Build environment

SIPxSUA CentOS-7 Build environment 的 Docker 定义文件

它为 SIPxSUA 在 CentOS-7 平台上进行了以下操作:

- `yum` 安装编译构建工具集
- 从源代码下载安装系统未提供的构建工具
- `yum` 安装第三方依赖软件
- 从源代码下载安装系统未提供的三方依赖软件

> 注意:
>
> - 构建需要 BUILDKIT
> - 需要以项目根目录作为 Context directory

构建命令例子:

```bash
cd <project-dir>
DOCKER_BUILDKIT=1 docker build -f docker/centos7/Dockerfile .
```

上级目录的 `docker-compose.yml` 定义了使用该进行编译 `sipxsua` 的 compose 脚本，可直接在上级目录运行 `docker compose up`
