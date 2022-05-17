# compose for build

这里的 compose 用于在不同目标平台下进行构建。

用法:

执行所有的构建:

```bash
DOCKER_BUILDKIT=1 docker-compose up
```

构建 CentOS 7:

```bash
DOCKER_BUILDKIT=1 docker-compose run centos7
```
