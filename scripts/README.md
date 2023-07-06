# README

利用 docker-compose 构建便于在容器种使用的产出物

```bash
docker compose run --rm build
```

将会构建 ubuntu:focal 兼容的二进制产出，输出到 `out/${os}-${arch}-${compiler}/{buildType}/bin/` 目录，如 `out/Linux-x86_64-GNU/Release/bin/sipxsua`

后续可以基于上述容器制作镜像。
