# compose for build

这里的 compose 用于在不同目标平台下进行构建。

用法:

执行所有的构建:

```bash
docker-compose up
```

如果是第一次执行，需要构建 docker 镜像，要加上 `build` 参数:

```bash
DOCKER_BUILDKIT=1 docker-compose up --build
```

又如，仅执行 CentOS 7 下的构建:

```bash
docker-compose run centos7
```
