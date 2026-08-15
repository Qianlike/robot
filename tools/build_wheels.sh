#!/usr/bin/env bash
# Build hightorque-robot Linux wheels with cibuildwheel.
#
# Usage:
#   ./tools/build_wheels.sh          # x86_64
#   ./tools/build_wheels.sh aarch64  # ARM64, requires Docker/QEMU or native ARM

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}/python"
ARCH="${1:-x86_64}"
DIST_DIR="${SCRIPT_DIR}/dist"
VENV_DIR="${SCRIPT_DIR}/.cibw-venv"

echo "==> staging core sources -> ${PROJECT_DIR}/core/"
rsync -a --delete \
    --exclude .git --exclude build --exclude tools --exclude '*.pyc' \
    "${SCRIPT_DIR}/../" "${PROJECT_DIR}/core/"

export PATH="${HOME}/.local/bin:${PATH}"
if [ ! -d "${VENV_DIR}" ]; then
    if command -v uv >/dev/null 2>&1; then
        echo "==> creating build environment with uv (Python 3.12)"
        uv venv --python 3.12 "${VENV_DIR}" --quiet
    else
        PYTHON="${PYTHON:-python3}"
        command -v "${PYTHON}" >/dev/null 2>&1 || {
            echo "error: cannot find ${PYTHON}" >&2
            exit 1
        }
        "${PYTHON}" -m venv "${VENV_DIR}"
    fi
fi

if [ ! -f "${VENV_DIR}/bin/activate" ]; then
    echo "error: build environment was not created: ${VENV_DIR}" >&2
    echo "Install uv or ensure python3-venv is available on the host." >&2
    exit 1
fi

# shellcheck disable=SC1091
source "${VENV_DIR}/bin/activate"

CIBW_VERSION_SPEC="${CIBW_VERSION_SPEC:-cibuildwheel==3.4.1}"

if command -v uv >/dev/null 2>&1; then
    uv pip install -U pip --quiet
    uv pip install -U "${CIBW_VERSION_SPEC}"
else
    python -m pip install -U pip
    pip install -U "${CIBW_VERSION_SPEC}"
fi

if ! command -v docker >/dev/null 2>&1; then
    if [ "${ARCH}" != "x86_64" ]; then
        echo "error: building ${ARCH} requires Docker; use ./tools/build_arm_wheels.sh for zig-based ARM builds." >&2
        exit 1
    fi

    if ! command -v uv >/dev/null 2>&1; then
        echo "error: Docker is unavailable and uv is required for local Linux wheel fallback." >&2
        exit 1
    fi

    echo "==> Docker not found; falling back to local x86_64 wheels (linux_x86_64, not manylinux)"
    rm -rf "${PROJECT_DIR}/dist"
    for version in 3.8 3.9 3.10 3.11 3.12 3.13 3.14; do
        echo "  - Python ${version}"
        uv build "${PROJECT_DIR}" --python "${version}" --wheel 2>&1 | tail -1
    done
    mkdir -p "${DIST_DIR}"
    mv -f "${PROJECT_DIR}"/dist/*.whl "${DIST_DIR}/"
    echo "==> done:"
    ls -lh "${DIST_DIR}"/hightorque_robot-*.whl
    exit 0
fi

echo "==> building wheels (arch=${ARCH})"
cibuildwheel "${PROJECT_DIR}" --platform linux --archs "${ARCH}" --output-dir "${DIST_DIR}"

echo "==> done:"
ls -lh "${DIST_DIR}"/hightorque_robot-*.whl
