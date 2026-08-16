# GitHub Actions Python Wheel 构建指南

本文档说明本仓库如何使用 GitHub Actions 自动构建 Python wheel，以及 .github 目录下工作流文件的写法和维护方法。

当前构建目标：

- Python 3.8 到 Python 3.14
- Linux x86_64
- Linux aarch64
- Windows AMD64
- Linux wheel 使用 manylinux2014 兼容目标，覆盖 Ubuntu 20.04 及更新版本

主要文件：

~~~text
.github/workflows/python-wheels.yml  # GitHub Actions 工作流
tools/python-build/python/pyproject.toml # Python 包和 cibuildwheel 配置
tools/python-build/build_wheels.sh       # Linux 构建脚本
tools/python-build/build_wheels.ps1       # Windows 构建脚本
~~~

## 1. 快速使用

### 1.1 自动构建

当前工作流只监听正式版本标签：

~~~yaml
on:
  push:
    tags:
      - "v[0-9]+.[0-9]+.[0-9]+"
~~~

向 GitHub 推送正式版本标签后，GitHub Actions 会自动开始构建：

~~~powershell
git tag -a v6.0.5 -m "hightorque-robot v6.0.5"
git push github v6.0.5
~~~

本仓库的远端名称约定如下：

~~~text
github  -> https://github.com/Qianlike/robot.git
origin  -> 公司 Git 服务器
~~~

只有符合 `v主版本.次版本.修订版本` 的标签会触发正式构建，例如 `v6.0.5`。
普通分支 push、`v6` 分支 push、`v6.0` 和 `v6.0.5-test1` 都不会触发正式构建。

### 1.2 构建方式

当前工作流没有配置 `workflow_dispatch`，正式构建通过推送版本标签启动。
需要重新构建时，应重新推送一个新的版本标签；如果需要网页手动启动，需先在工作流中增加 `workflow_dispatch` 触发器。

### 1.3 查看运行状态

进入 Actions -> Python wheels 后，可以看到每次运行记录。当前一次运行包含 3 个构建任务：

~~~text
Linux x86_64 wheels
Linux aarch64 wheels
Windows AMD64 wheels
~~~

点击具体任务后，可以展开查看：

~~~text
Check out repository
Set up Python
Build wheels
Report build failure
Upload wheels
Upload build log
~~~

## 2. 构建流程

一次完整构建的流程如下：

~~~text
向 GitHub push 正式版本标签
        |
        v
GitHub 读取 .github/workflows/python-wheels.yml
        |
        +--> Linux x86_64 job
        +--> Linux aarch64 job
        +--> Windows AMD64 job
        |
        v
checkout 源代码并准备 Python 3.11 构建环境
        |
        v
调用 cibuildwheel，分别生成 cp38 到 cp314 wheel
        |
        v
检查每个 Python 版本的 wheel 是否存在
        |
        v
上传 wheel Artifacts 和构建日志
~~~

这里的 Python 3.11 是运行构建工具的环境，不代表最终只支持 Python 3.11。最终支持的 Python 版本由 CIBW_BUILD 和 pyproject.toml 决定。

## 3. .github 和工作流文件

GitHub 会自动读取仓库根目录下的 .github/workflows/：

~~~text
.github/
└── workflows/
    └── python-wheels.yml
~~~

每个 .yml 或 .yaml 文件都可以定义一个独立工作流。例如以后可以增加：

~~~text
.github/workflows/python-tests.yml
.github/workflows/release.yml
~~~

当前工作流的基本结构：

~~~yaml
name: Python wheels

on:
  push:
    tags:
      - "v[0-9]+.[0-9]+.[0-9]+"

permissions:
  contents: read

jobs:
  linux:
    ...

  windows-amd64:
    ...
~~~

常见配置含义：

| 配置 | 作用 |
| --- | --- |
| name | GitHub Actions 页面中显示的工作流名称 |
| on | 定义什么时候触发工作流 |
| workflow_dispatch | 允许在网页上手动启动 |
| push.branches | 指定哪些分支 push 后自动启动 |
| permissions | 限制工作流访问仓库的权限 |
| jobs | 定义具体要执行的任务 |
| steps | 定义每个任务中的执行步骤 |

当前只需要读取代码和上传构建结果，因此权限设置为：

~~~yaml
permissions:
  contents: read
~~~

## 4. Linux 构建矩阵

当前 Linux job 使用矩阵构建两个架构：

~~~yaml
strategy:
  fail-fast: false
  matrix:
    include:
      - arch: x86_64
        runner: ubuntu-22.04
      - arch: aarch64
        runner: ubuntu-22.04-arm
~~~

矩阵会展开成两个独立任务：

| 目标架构 | GitHub Runner |
| --- | --- |
| Linux x86_64 | ubuntu-22.04 |
| Linux aarch64 | ubuntu-22.04-arm |

runs-on 使用 matrix.runner，构建脚本使用 matrix.arch。矩阵的好处是只需写一套 job 配置，就可以生成多个架构任务。

### 4.1 为什么 Runner 是 Ubuntu 22.04

runner: ubuntu-22.04 表示 GitHub 用 Ubuntu 22.04 机器执行命令。它不等于 wheel 只能在 Ubuntu 22.04 上运行。

Linux wheel 的兼容目标由 manylinux 镜像决定：

~~~yaml
CIBW_MANYLINUX_X86_64_IMAGE: manylinux2014
CIBW_MANYLINUX_AARCH64_IMAGE: manylinux2014
~~~

因此构建出来的 wheel 目标是 manylinux2014，通常可以用于 Ubuntu 20.04、Ubuntu 22.04、Ubuntu 24.04 以及其他满足对应 glibc 兼容条件的 Linux 系统。

### 4.2 fail-fast: false

~~~yaml
fail-fast: false
~~~

表示某个架构失败时，不取消其他架构的任务。例如 x86_64 失败时，aarch64 仍然继续构建。

## 5. Python 版本和 wheel 标签

tools/python-build/python/pyproject.toml 中定义了最低 Python 版本：

~~~toml
requires-python = ">=3.8"
~~~

并指定 cibuildwheel 构建以下标签：

~~~toml
[tool.cibuildwheel]
build = "cp38-* cp39-* cp310-* cp311-* cp312-* cp313-* cp314-*"
~~~

标签与 Python 版本的对应关系：

| 标签 | Python 版本 |
| --- | --- |
| cp38 | Python 3.8 |
| cp39 | Python 3.9 |
| cp310 | Python 3.10 |
| cp311 | Python 3.11 |
| cp312 | Python 3.12 |
| cp313 | Python 3.13 |
| cp314 | Python 3.14 |

当前不是一个 wheel 支持所有 Python 版本，而是每个 Python 版本生成一个 wheel。用户使用 pip 安装时，pip 会自动选择与当前 Python、系统和架构匹配的文件。

## 6. cibuildwheel 版本和 Artifact 分组

当前选择一个同时覆盖目标 Python wheel 标签的 cibuildwheel 版本：

~~~yaml
CIBW_BUILD="cp38-* cp39-* cp310-* cp311-* cp312-* cp313-* cp314-*"
CIBW_VERSION_SPEC="cibuildwheel==3.4.1"
~~~

因此 cp38 到 cp314 可以在一次 cibuildwheel 调用中完成，不需要因为 Python 3.14 再拆成第二次构建。这里的 Python 版本标签是构建目标，和 cibuildwheel 自身运行所需的 Python 版本不是同一个概念；当前工作流使用 Python 3.11 运行 cibuildwheel。

Artifact 的分组方式由 Upload wheels 步骤的 name 和 path 决定，与 cibuildwheel 是否调用一次没有直接关系。每个系统和架构 job 都把所有 wheel 写入 tools/python-build/dist/，然后一次性上传 tools/python-build/dist/*.whl，因此得到按系统和架构划分的 Artifact。若想按 Python 版本拆分 Artifact，需要增加多个上传步骤，分别指定 cp38、cp39 等文件路径。

## 7. 构建脚本和步骤

### 7.1 下载代码和准备 Python

工作流使用 actions/checkout 下载仓库代码，使用 actions/setup-python 准备 Python 3.11。Python 3.11 只是构建工具环境。

### 7.2 Linux 构建脚本

实际构建由以下脚本完成：

~~~text
tools/python-build/build_wheels.sh
~~~

脚本会创建构建虚拟环境、安装 cibuildwheel、准备核心 C++ 源码，然后把生成的 wheel 放入 tools/python-build/dist/。

### 7.3 Windows 构建脚本

Windows job 使用 PowerShell，并调用：

~~~powershell
powershell -ExecutionPolicy Bypass -File tools/python-build/build_wheels.ps1
~~~

Windows 当前只构建 win_amd64，也就是 Windows AMD64，不构建 Windows ARM64 或 32 位 Windows。

## 8. 构建检查和日志

构建完成后，工作流会检查以下标签是否都生成：

~~~text
cp38 cp39 cp310 cp311 cp312 cp313 cp314
~~~

Linux 的检查逻辑类似：

~~~bash
for tag in cp38 cp39 cp310 cp311 cp312 cp313 cp314; do
  matches=(tools/python-build/dist/*-$tag-*.whl)
  if [ ${#matches[@]} -eq 0 ]; then
    echo "error: missing $tag wheel"
    exit 1
  fi
done
~~~

实际工作流会使用数组长度检查匹配文件数量，文档中的代码只用于理解流程。

构建步骤使用 continue-on-error: true，让构建失败时仍能上传日志。随后 Report build failure 步骤会重新返回失败状态。日志上传使用 if: always:，因此成功和失败都会尝试上传日志。

## 9. Artifacts 的名称和内容

成功后会产生 3 个主要 wheel Artifacts：

~~~text
hightorque-robot-linux-x86_64-cp38-cp314
hightorque-robot-linux-aarch64-cp38-cp314
hightorque-robot-windows-amd64-cp38-cp314
~~~

每个 Artifact 中包含对应系统和架构下的多个 wheel，例如：

~~~text
hightorque_robot-6.0.0-cp38-...whl
hightorque_robot-6.0.0-cp39-...whl
hightorque_robot-6.0.0-cp310-...whl
hightorque_robot-6.0.0-cp311-...whl
hightorque_robot-6.0.0-cp312-...whl
hightorque_robot-6.0.0-cp313-...whl
hightorque_robot-6.0.0-cp314-...whl
~~~

同时会上传 3 个构建日志 Artifact：

~~~text
hightorque-robot-linux-x86_64-cp38-cp314-log
hightorque-robot-linux-aarch64-cp38-cp314-log
hightorque-robot-windows-amd64-cp38-cp314-log
~~~

Artifact 只是 GitHub Actions 的构建产物存储，不等同于 PyPI。当前工作流还没有自动发布到 PyPI、公司 PyPI 或 GitLab Package Registry。

## 10. 下载 Artifact 后如何安装

下载并解压某个 Artifact 后，在 wheel 所在目录执行：

~~~powershell
pip install --find-links . hightorque-robot==6.0.0
~~~

pip 会自动选择匹配当前环境的 wheel。例如：

~~~text
Python 3.11 + Linux x86_64  -> cp311 / x86_64 wheel
Python 3.12 + Linux aarch64 -> cp312 / aarch64 wheel
Python 3.14 + Windows AMD64 -> cp314 / win_amd64 wheel
~~~

如果直接执行 pip install hightorque-robot==6.0.0，则要求这个包已经发布到了 PyPI 或其他已配置的 Python 包索引。GitHub Actions Artifact 本身不会被 pip 自动搜索。

## 11. 如何修改包版本

修改 tools/python-build/python/pyproject.toml：

~~~toml
[project]
version = "6.0.0"
~~~

例如升级到 6.0.1：

~~~toml
version = "6.0.1"
~~~

然后提交并推送对应的版本标签：

~~~powershell
git add tools/python-build/python/pyproject.toml
git commit -m "发布 Python 包 6.0.1"
git tag -a v6.0.1 -m "hightorque-robot v6.0.1"
git push github v6.0.1
~~~

推送成功后，GitHub Actions 会自动生成带有新版本号的 wheel。

## 12. 如何修改 Python 支持范围

如果要增加 Python 3.15，需要同时修改多个位置：

1. 修改 pyproject.toml 的 build 列表。
2. 修改工作流中 CIBW_BUILD 的列表。
3. 修改 Linux 完整性检查列表。
4. 修改 Windows PowerShell 完整性检查列表。
5. 如果新版本需要新的 cibuildwheel，再增加单独的构建段。
6. 修改 Artifact 名称中的版本范围。

只修改其中一处可能导致新版本没有构建、被检查步骤判定为缺失，或者 Linux 和 Windows 支持范围不一致。

## 13. 如何修改触发条件

当前配置：

~~~yaml
on:
  push:
    tags:
      - "v[0-9]+.[0-9]+.[0-9]+"
~~~

正式版本标签不区分分支，标签指向哪个提交，工作流就构建哪个提交。
如果以后希望普通分支 push 也触发：

~~~yaml
on:
  push:
    branches:
      - v6
      - main
~~~

如果只想监听 main：

~~~yaml
on:
  push:
    branches:
      - main
~~~

旧的 codex/github-python-build 分支不再是当前工作流的触发条件。即使以后删除旧分支，也不会影响正式版本标签触发的自动构建。

## 14. 如何增加 Pull Request 检查

如果希望 Pull Request 创建或更新时也构建，可以增加：

~~~yaml
on:
  push:
    tags:
      - "v[0-9]+.[0-9]+.[0-9]+"
  pull_request:
    branches:
      - v6
~~~

实际项目中通常会把任务分成两类：

~~~text
python-tests.yml   # Pull Request 时运行快速测试
python-wheels.yml  # push 到指定分支时构建完整 wheel
~~~

## 15. Artifact、Release 和 PyPI 的区别

### GitHub Actions Artifact

适合测试构建结果、内部下载、查看构建日志和临时保存某次运行的 wheel。不适合直接作为正式 Python 包索引。

### GitHub Release

适合按版本发布正式构建产物、保存版本说明和校验值，用户可以从版本页面下载。

### PyPI 或公司 Package Registry

适合用户直接执行：

~~~powershell
pip install hightorque-robot==6.0.0
~~~

用户不需要自己下载和筛选 wheel，pip 会自动从包索引中选择正确文件。

当前仓库只配置了 Artifact 上传。如果以后需要正式发布，需要额外增加发布 job、版本触发规则和仓库密钥，例如 PyPI Token 或 GitLab Package Registry Token。

## 16. 常见问题排查

### 没有触发构建

检查：

1. 提交是否真的推送到了 GitHub 的 v6。
2. 是否误用了 git push origin v6。
3. 工作流文件是否已经存在于 GitHub 的 v6。
4. Actions 页面是否被禁用。
5. 是否在正确的仓库中查看 Actions。

可以检查远端分支：

~~~powershell
git ls-remote github refs/heads/v6
~~~

### 某个 Python 版本缺少 wheel

重点查看：

- CIBW_BUILD 是否包含该版本；
- pyproject.toml 的 build 是否包含该版本；
- 验证列表是否包含该版本；
- 对应版本的 cibuildwheel 是否支持该 Python 版本。

### Linux wheel 无法安装

检查 CPU 架构、Python 版本、Linux glibc 兼容性，以及是否把 x86_64 wheel 下载到了 aarch64 机器。

### Windows wheel 无法安装

当前只提供 win_amd64，因此必须使用 64 位 Windows 和 64 位 Python。32 位 Python 或 Windows ARM64 当前没有对应 wheel。

### 构建失败但没有看到原因

先打开对应 job 的 Build wheels 步骤。如果日志不完整，再下载对应的 *-log Artifact。工作流会在构建失败时保留日志。

## 17. 当前配置总结

| 项目 | 当前配置 |
| --- | --- |
| GitHub 工作流 | .github/workflows/python-wheels.yml |
| 自动触发分支 | v6 |
| 手动触发 | 支持 |
| Linux x86_64 | 支持 |
| Linux aarch64 | 支持 |
| Windows AMD64 | 支持 |
| Windows ARM64 | 暂不支持 |
| Python 版本 | 3.8 到 3.14 |
| Linux 兼容目标 | manylinux2014 |
| Ubuntu 20.04 | 支持兼容目标 |
| wheel 存储位置 | GitHub Actions Artifacts |
| 自动发布到 PyPI | 暂未配置 |
| 自动发布到 GitLab Registry | 暂未配置 |
| 旧构建分支 | 不再作为触发条件，未删除 |
