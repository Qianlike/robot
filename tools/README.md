# tools —— Python 打包

把 hightorque_fdcan C++ SDK 打包为 pip 可安装的 Python 库 `hightorque-robot`。
本文档说明 wheel 的构建方法；包的使用方法（快速开始、API 一览、参数文件
查找顺序）见 `hightorque_robot` 模块的 docstring（`tools/python/hightorque_robot/__init__.py`）。

## 目录结构

```
tools/
├── python/                  # pip 构建工程（scikit-build-core + pybind11）
│   ├── pyproject.toml       # 构建配置（Python 3.8+ / Linux）
│   ├── CMakeLists.txt       # 绑定模块构建（core/ 双路回退）
│   ├── src/python_bindings.cpp   # pybind11 绑定源码（模块 _core）
│   ├── hightorque_robot/    # Python 包（仅 __init__.py，不含数据副本）
│   ├── tests/               # 无硬件 pytest 测试
│   └── core/                # 构建产物：rsync 的核心源码副本（勿提交）
├── build_wheels.sh          # 构建 x86_64/aarch64 wheel（Docker + cibuildwheel）
├── build_arm_wheels.sh      # 交叉编译 aarch64 wheel（zig，无需 Docker/ARM 机器）
├── ci-wheel.yml             # GitHub Actions 模板（复制到 .github/workflows/）
└── dist/                    # 构建产物：wheel 文件（勿提交）
```

## 快速上手

```bash
# 本地安装（Ubuntu 20.04 等需先升级 pip 到 >=20.3 以支持 PEP 517）
python3 -m pip install -U pip
pip install -e tools/python

# 运行测试（全部无硬件）
pip install pytest
pytest tools/python/tests -v
```

## 构建 wheel

```bash
# x86_64：有 Docker 时用 cibuildwheel 打 manylinux 标签（跨发行版兼容）；
#         无 Docker 时自动降级为 uv 本地构建（tag: linux_x86_64，本机及同 glibc 环境可装）
tools/build_wheels.sh
# 产物: tools/dist/hightorque_robot-6.0.0-cp3{8..14}-cp3{8..14}-manylinux_2_28_x86_64.whl
#     （降级模式: ...-cp3{8..14}-linux_x86_64.whl）

# ARM64：zig 交叉编译，无需 Docker（推荐）
tools/build_arm_wheels.sh all
# 产物: tools/dist/hightorque_robot-6.0.0-cp3{8..14}-cp3{8..14}-linux_aarch64.whl

# ARM64（备选，需要 Docker+QEMU 或 ARM 机器）
tools/build_wheels.sh aarch64
```

**Python 版本策略（分版本 wheel）**：为 **Python 3.8 ~ 3.14** 分别构建
wheel（共 7 个，pip 安装时自动选择匹配版本，对用户透明）。
不做 abi3 单包的原因：pybind11 从未真正支持 `Py_LIMITED_API` 编译
（pybind/pybind11#1755，stable ABI 支持仍在其路线图讨论中），
abi3 方案在 pybind11 下不可行。构建/CI 配置已按此固定。

**平台**：仅 Linux（vendored serial 库是 POSIX 实现，硬编码编译 unix 版）。
代码经检查**无任何 x86 特有代码**（无 SIMD/内联汇编/x86 头文件），
协议结构本就是为 ARM 电机固件设计，ARM 天然兼容。

**ARM (aarch64) 支持**（已配置完成 + 实测验证）：
- **本地交叉编译**（推荐）：`tools/build_arm_wheels.sh` —— 用 zig 交叉编译器
  （`-target aarch64-linux-gnu`）+ python-build-standalone 的 aarch64 Python
  头文件，直接在本机产出 ARM wheel，**无需 Docker / ARM 机器**：
  ```bash
  ./tools/build_arm_wheels.sh           # Python 3.12
  ./tools/build_arm_wheels.sh all       # 3.8~3.14 全部
  # 产物: tools/dist/hightorque_robot-<ver>-cp<xy>-cp<xy>-linux_aarch64.whl
  ```
  依赖：zig（`~/.local/bin/zig`，来源见下）+ 网络（南大 github-release 镜像
  下载 aarch64 Python，首次 ~85MB/版本，缓存于 `~/.cache/ht-arm-py/`）
- 其他路径：
  1. CI：`tools/ci-wheel.yml` 的 `build-aarch64` job（注意：仓库托管在私有
     Git 服务器 git.clicki.cn，GitHub Actions 模板需适配 Gitea Actions /
     runner 标签后再用）
  2. `tools/build_wheels.sh aarch64`（Docker + QEMU）
  3. 直接在一台 ARM Linux 机器（如 Jetson/RK3588）上 `pip install` 源码构建，
     天然产出 aarch64 wheel
- **zig 安装**（无 sudo）：`pip download ziglang -i https://pypi.tuna.tsinghua.edu.cn/simple`
  解压 wheel 后整个目录复制到 `~/.local/zig-0.16/` 并 `chmod +x`（zig 需配套
  lib/ 目录，不能只拷二进制），软链 `~/.local/bin/zig`

## 对 C++ 源码的改动（纯加法，向后兼容）

打包需要给核心库加了 4 处**纯加法**改动，不改变既有行为：

1. `include/parse_robot_params.h` + `src/parse_robot_params.cpp`：
   新增 `parse_robot_params(config_path)` 重载；无参版行为不变。
   **行为变更**（仅新路径参数生效）：`param_file` 相对路径改为相对
   配置文件所在目录解析（默认路径下与原 CWD 语义结果一致）。
2. `include/robot.h` + `src/robot.cpp`：新增 `Robot(config_path)` 构造；
   原 `Robot()` 行为不变。
3. `include/motor.h`：新增内联 `get_id()`。
4. `include/robot.h`：显式 `delete` 拷贝构造/赋值（原本由 unique_ptr 成员
   隐式删除，行为不变；显式声明供 pybind11 正确识别不可拷贝类型）。

## 已知限制

- 串口不足时 Python 层抛 `RuntimeError`；但端口存在而打开失败/握手超时等
  场景 C++ 端仍会 `exit()` 直接终止进程（SDK 现状，后续 v2 计划改为抛异常）
- sdist 不支持（核心源码在打包工程之外），只发 wheel
- **yaml 单一数据源**：wheel 内置参数数据不在此维护副本——构建时由 CMake
  从仓库根 `robot_param/` 复制进 wheel（原始 `robot_config.yaml` 的
  `param_file` 相对路径在 wheel 内归一化后仍指向同一目录，无需修改版）
