#!/usr/bin/env bash
# 构建 hightorque-robot 的 Linux wheel（cibuildwheel，需要 Docker）。
#
# 用法:
#   ./tools/build_wheels.sh            # x86_64
#   ./tools/build_wheels.sh aarch64    # ARM64（需要 QEMU 或原生 ARM 环境）
#
# 产物: tools/dist/hightorque_robot-6.0.0-cp3{8..14}-cp3{8..14}-manylinux_2_28_<arch>.whl
# （每个 Python 版本一个 wheel，pip 自动选择匹配版本）
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}/python"
ARCH="${1:-x86_64}"
DIST_DIR="${SCRIPT_DIR}/dist"

# 1) stage 核心源码到项目目录
#    cibuildwheel 只把项目目录（tools/python）拷进容器，必须把仓库根的
#    src/include/lib 一并复制进去；tools/python/CMakeLists.txt 会优先使用 core/。
echo "==> staging core sources -> ${PROJECT_DIR}/core/"
rsync -a --delete \
    --exclude .git --exclude build --exclude tools --exclude '*.pyc' \
    "${SCRIPT_DIR}/../" "${PROJECT_DIR}/core/"

# 2) 构建环境（cibuildwheel 3.x 要求 host Python >= 3.11）
#    优先用 uv（standalone Python 3.12，不依赖系统 venv/pip 支持）；
#    无 uv 时回退系统 python3 -m venv（需 python3-venv 包）
export PATH="${HOME}/.local/bin:${PATH}"
VENV_DIR="${SCRIPT_DIR}/.cibw-venv"
if [ ! -d "${VENV_DIR}" ]; then
    if command -v uv >/dev/null 2>&1; then
        echo "==> 用 uv 创建构建环境（Python 3.12）"
        uv venv --python 3.12 "${VENV_DIR}" --quiet
    else
        PYTHON="${PYTHON:-python3}"
        command -v "${PYTHON}" >/dev/null 2>&1 || { echo "error: 找不到 ${PYTHON}" >&2; exit 1; }
        "${PYTHON}" -m venv "${VENV_DIR}"
    fi
fi
if [ ! -f "${VENV_DIR}/bin/activate" ]; then
    echo "error: 构建环境创建失败: ${VENV_DIR}" >&2
    echo "  （Ubuntu 20.04 系统 python3 可能缺 venv 支持，建议先安装 uv：curl -LsSf https://astral.sh/uv/install.sh | sh）" >&2
    exit 1
fi
# shellcheck disable=SC1091
source "${VENV_DIR}/bin/activate"
# pin <4：cibuildwheel 4.x 移除了 cp38 构建目标，3.x 是支持 Python 3.8 的最后一个系列
if command -v uv >/dev/null 2>&1; then
    uv pip install -U pip --quiet
    uv pip install "cibuildwheel<4"
else
    python -m pip install -U pip
    pip install "cibuildwheel<4"
fi

# 3) 构建
if ! command -v docker >/dev/null 2>&1; then
    # 无 Docker 时：
    #   x86_64 -> 降级为 uv 本地构建（tag 为 linux_x86_64，非 manylinux，本机及
    #              同 glibc 环境可装；跨发行版兼容需 manylinux，留给 CI/有 Docker 的环境）
    #   aarch64 -> 提示用 zig 交叉编译（无需 Docker）
    if [ "${ARCH}" != "x86_64" ]; then
        echo "error: 构建 ${ARCH} 需要 Docker；无 Docker 时 ARM 请用 ./tools/build_arm_wheels.sh（zig 交叉编译）" >&2
        exit 1
    fi
    echo "==> 未检测到 Docker，降级为 uv 本地构建 x86_64（wheel tag: linux_x86_64，非 manylinux）"
    rm -rf "${PROJECT_DIR}/dist"
    for v in 3.8 3.9 3.10 3.11 3.12 3.13 3.14; do
        echo "  - Python ${v}"
        uv build "${PROJECT_DIR}" --python "$v" --wheel 2>&1 | tail -1
    done
    mkdir -p "${DIST_DIR}"
    mv -f "${PROJECT_DIR}"/dist/*.whl "${DIST_DIR}/"
    echo "==> done（降级模式，未打 manylinux 标签）:"
    ls -lh "${DIST_DIR}"/hightorque_robot-*.whl
    exit 0
fi
echo "==> building wheels (arch=${ARCH})"
cibuildwheel "${PROJECT_DIR}" --platform linux --arch "${ARCH}" --output-dir "${DIST_DIR}"

# 4) 结果
echo "==> done:"
ls -lh "${DIST_DIR}"/hightorque_robot-*.whl
