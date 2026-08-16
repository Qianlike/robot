# tools —— Python 打包

把 hightorque_fdcan C++ SDK 打包为 pip 可安装的 Python 库 `hightorque-robot`。
本文档说明 wheel 的构建方法；包的使用方法（快速开始、API 一览、参数文件
查找顺序）见 `hightorque_robot` 模块的 docstring（`tools/python-build/python/hightorque_robot/__init__.py`）。

## 目录结构

```
tools/
├── python-build/            # Python wheel 构建工程
│   ├── python/              # pip 工程（scikit-build-core + pybind11）
│   │   ├── pyproject.toml   # 构建配置（Python 3.8+ / Linux / Windows）
│   │   ├── CMakeLists.txt   # 绑定模块构建（core/ 双路回退）
│   │   ├── src/             # pybind11 绑定源码
│   │   ├── hightorque_robot/ # Python 包（不含配置文件）
│   │   └── tests/            # pytest 测试
│   ├── build_wheels.sh      # Linux x86_64/aarch64 构建
│   ├── build_wheels.ps1     # Windows AMD64 构建
│   ├── build_arm_wheels.sh  # zig 交叉编译 aarch64 wheel
│   └── dist/                # 构建产物（勿提交）
└── README.md               # 本说明文件
```

## 快速上手

```console
# 本地安装（Linux/Windows；Ubuntu 20.04 等需先升级 pip 到 >=20.3）
python -m pip install -U pip
python -m pip install -e tools/python-build/python

# 运行测试（全部无硬件）
python -m pip install pytest
python -m pytest tools/python-build/python/tests -v
```

## 构建 wheel

```bash
# x86_64：有 Docker 时用 cibuildwheel 打 manylinux 标签（跨发行版兼容）；
#         无 Docker 时自动降级为 uv 本地构建（tag: linux_x86_64，本机及同 glibc 环境可装）
tools/python-build/build_wheels.sh
# 产物: tools/python-build/dist/hightorque_robot-6.0.0-cp3{8..14}-cp3{8..14}-manylinux_2_28_x86_64.whl
#     （降级模式: ...-cp3{8..14}-linux_x86_64.whl）

# ARM64：zig 交叉编译，无需 Docker（推荐）
tools/python-build/build_arm_wheels.sh all
# 产物: tools/python-build/dist/hightorque_robot-6.0.0-cp3{8..14}-cp3{8..14}-linux_aarch64.whl

# ARM64（备选，需要 Docker+QEMU 或 ARM 机器）
tools/python-build/build_wheels.sh aarch64
```

Windows 10（64 位）在 PowerShell 中构建：

```powershell
# 需要 Python 3.11+，以及 Visual Studio 2022 C++ Build Tools 或 MinGW-w64
powershell -ExecutionPolicy Bypass -File tools/python-build/build_wheels.ps1

# 仅构建当前需要的 Python 3.11 wheel，并明确使用 MinGW
powershell -ExecutionPolicy Bypass -File tools/python-build/build_wheels.ps1 `
  -Toolchain MinGW -Build "cp311-*"

# 产物：tools/python-build/dist/hightorque_robot-6.0.0-cp3*-cp3*-win_amd64.whl
```

脚本会把核心源码暂存到 `tools/python-build/python/core/`，在独立虚拟环境中安装
`cibuildwheel==3.4.1`，自动选择 MSVC 或 MinGW，并为 Python 3.8~3.14 构建
`win_amd64` wheel。可用 `-Build "cp311-*"` 限定 Python 版本；若构建依赖
已经安装，可传 `-SkipBootstrap` 跳过联网安装步骤。MinGW 构建会静态链接
GCC、C++ 和 winpthread 运行库，目标机器无需额外安装 MinGW DLL。

**Python 版本策略（分版本 wheel）**：为 **Python 3.8 ~ 3.14** 分别构建
wheel（共 7 个，pip 安装时自动选择匹配版本，对用户透明）。
不做 abi3 单包的原因：pybind11 从未真正支持 `Py_LIMITED_API` 编译
（pybind/pybind11#1755，stable ABI 支持仍在其路线图讨论中），
abi3 方案在 pybind11 下不可行。构建/CI 配置已按此固定。

**平台**：支持 Linux x86_64/aarch64 和 64 位 Windows 10。串口子库由 CMake
按平台选择 POSIX 或 Windows 实现；Windows wheel 使用 SetupAPI 枚举串口，按
`VID_CAF1/PID_FFFF` 筛选通信板，并依据完整 `hardware_id` 中的 `MI_xx`
稳定映射 CAN0~CAN6。代码经检查**无任何 x86 特有代码**（无 SIMD、内联汇编
或 x86 头文件），协议结构本就是为 ARM 电机固件设计，ARM 天然兼容。

**ARM (aarch64) 支持**（已配置完成 + 实测验证）：
- **本地交叉编译**（推荐）：`tools/python-build/build_arm_wheels.sh` —— 用 zig 交叉编译器
  （`-target aarch64-linux-gnu`）+ python-build-standalone 的 aarch64 Python
  头文件，直接在本机产出 ARM wheel，**无需 Docker / ARM 机器**：
  ```bash
  ./tools/python-build/build_arm_wheels.sh           # Python 3.12
  ./tools/python-build/build_arm_wheels.sh all       # 3.8~3.14 全部
  # 产物: tools/python-build/dist/hightorque_robot-<ver>-cp<xy>-cp<xy>-linux_aarch64.whl
  ```
  依赖：zig（`~/.local/bin/zig`，来源见下）+ 网络（南大 github-release 镜像
  下载 aarch64 Python，首次 ~85MB/版本，缓存于 `tools/python-build/cache/arm-python/`）
- 其他路径：
  1. `tools/python-build/build_wheels.sh aarch64`（Docker + QEMU）
  2. 直接在一台 ARM Linux 机器（如 Jetson/RK3588）上 `pip install` 源码构建，
     天然产出 aarch64 wheel
- **zig 安装**（无 sudo）：`pip download ziglang -i https://pypi.tuna.tsinghua.edu.cn/simple`
  解压 wheel 后整个目录复制到 `~/.local/zig-0.16/` 并 `chmod +x`（zig 需配套
  lib/ 目录，不能只拷二进制），软链 `~/.local/bin/zig`

## GitHub Actions 构建与 Release 发布

GitHub Actions 工作流位于 `.github/workflows/python-wheels.yml`。工作流只在推送正式版本标签时运行，标签格式必须是 `v主版本.次版本.修订版本`，例如：

```bash
git tag -a v6.0.5 -m "hightorque-robot v6.0.5"
git push github v6.0.5
```

普通分支 push、`v6` 分支 push、`v6.0` 和 `v6.0.5-test1` 都不会触发正式构建。

每个版本会构建以下 Python wheel：

- Python 3.8、3.9、3.10、3.11、3.12、3.13、3.14；
- Linux x86_64；
- Linux aarch64；
- Windows AMD64。

Linux 使用 manylinux2014 构建，目标是兼容 Ubuntu 20.04 及之后的常见 Ubuntu 系统；工作流跳过 musllinux。Windows 只构建 64 位 AMD64 wheel。

### Release 资产

为了避免 Release 页面出现大量单独的 wheel 文件，Python wheel 会按系统和架构分别打包成 ZIP：

```text
hightorque-robot-linux-x86_64-python-wheels.zip
hightorque-robot-linux-aarch64-python-wheels.zip
hightorque-robot-windows-amd64-python-wheels.zip
hightorque-robot-v<版本号>-cpp-source.tar.gz
SHA256SUMS
```

每个 Python ZIP 包含对应系统或架构的 7 个 wheel。下载时只选择目标系统和架构的 ZIP，解压后再根据 Python 版本选择一个 wheel，不要把 ZIP 内的 7 个 wheel 一起安装。

Python wheel 不包含 `robot_param` 目录、`robot_config.yaml` 或其他 YAML 配置文件。使用 Python API 时，`Robot` 必须显式传入外部配置文件路径：

```python
from hightorque_robot import Robot

robot = Robot("/path/to/robot_config.yaml")
```

### 安装 Python wheel

以 Linux x86_64 和 Python 3.11 为例：

```bash
unzip hightorque-robot-linux-x86_64-python-wheels.zip
python3.11 -m pip install \
  ./hightorque_robot-6.0.5-cp311-cp311-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
```

wheel 文件名中的 Python 标签对应关系如下：

```text
Python 3.8  -> cp38
Python 3.9  -> cp39
Python 3.10 -> cp310
Python 3.11 -> cp311
Python 3.12 -> cp312
Python 3.13 -> cp313
Python 3.14 -> cp314
```

Windows AMD64 使用 PowerShell：

```powershell
Expand-Archive .\hightorque-robot-windows-amd64-python-wheels.zip -DestinationPath .\hightorque-wheels
py -3.11 -m pip install .\hightorque-wheels\hightorque_robot-6.0.5-cp311-cp311-win_amd64.whl
```

安装后可以检查版本和模块是否正常：

```bash
python -c "import hightorque_robot; print(hightorque_robot.__version__)"
```
