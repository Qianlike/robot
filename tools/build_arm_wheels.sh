#!/usr/bin/env bash
# hightorque-robot aarch64 (ARM64) wheel 交叉编译构建
#
# 无需 Docker / ARM 机器：
#   - zig 交叉编译器（zig c++ -target aarch64-linux-gnu）
#   - python-build-standalone 的 aarch64 Python（仅用头文件；扩展的 Python 符号
#     由运行时宿主解析，Linux 链接共享库时未解析符号默认允许）
#
# 用法:
#   ./tools/build_arm_wheels.sh                # 构建 Python 3.12
#   ./tools/build_arm_wheels.sh 3.8 3.10 3.12  # 指定版本（3.8~3.15）
#   ./tools/build_arm_wheels.sh all            # 全部 3.8~3.14
#
# 产物: tools/dist/hightorque_robot-<ver>-cp<xy>-cp<xy>-linux_aarch64.whl
# 环境变量: NJU_MIRROR=1 使用南大 github-release 镜像（默认；GitHub 直连失败时自动回退）
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "${SCRIPT_DIR}")"
DIST_DIR="${SCRIPT_DIR}/dist"
CACHE_DIR="${HOME}/.cache/ht-arm-py"
WORK_DIR="/tmp/ht-arm-build-$$"
PYBIND11_VERSION="2.13.6"

# zig 常见安装位置（~/.local/bin 等）
export PATH="${HOME}/.local/bin:${HOME}/.local/zig-0.16:${PATH}"

# Python 版本 → aarch64 python-build-standalone release tag
# 3.8/3.9 已停止维护，用最后的 20240713（3.8.19/3.9.19）；其余用最新 20260807
declare -A PBS_TAG=(
    ["3.8"]="20240713" ["3.9"]="20240713"
    ["3.10"]="20260807" ["3.11"]="20260807" ["3.12"]="20260807"
    ["3.13"]="20260807" ["3.14"]="20260807" ["3.15"]="20260807"
)

log()  { printf '\033[1;32m==>\033[0m %s\n' "$*"; }
die()  { printf '\033[1;31merror:\033[0m %s\n' "$*" >&2; exit 1; }

[ -x "$(command -v zig)" ] || die "找不到 zig（交叉编译器），请先安装：pip download ziglang（清华镜像）或 https://ziglang.org"

# ==================== 参数解析 ====================
if [ $# -eq 0 ]; then
    PYVERS=("3.12")
elif [ "$1" = "all" ]; then
    PYVERS=(3.8 3.9 3.10 3.11 3.12 3.13 3.14)
else
    PYVERS=("$@")
fi

mkdir -p "${DIST_DIR}" "${CACHE_DIR}"

# ==================== pybind11 include（缓存） ====================
PYB_DIR="${CACHE_DIR}/pybind11-${PYBIND11_VERSION}"
if [ ! -d "${PYB_DIR}/pybind11/include/pybind11" ]; then
    log "获取 pybind11 ${PYBIND11_VERSION} include ..."
    PYB_WHEEL=$(find "${CACHE_DIR}" -maxdepth 1 -name "pybind11-${PYBIND11_VERSION}*.whl" | head -1)
    if [ -z "${PYB_WHEEL}" ]; then
        python3 -m pip download "pybind11==${PYBIND11_VERSION}" -d "${CACHE_DIR}" \
            -i https://pypi.tuna.tsinghua.edu.cn/simple --no-deps --quiet
        PYB_WHEEL=$(find "${CACHE_DIR}" -maxdepth 1 -name "pybind11-${PYBIND11_VERSION}*.whl" | head -1)
    fi
    mkdir -p "${PYB_DIR}" && python3 -c \
        "import zipfile; zipfile.ZipFile('${PYB_WHEEL}').extractall('${PYB_DIR}')"
fi

# ==================== 查找 aarch64 python-build-standalone ====================
# 返回: <tag> <asset> <已解压目录>；找不到则报错
get_arm_python() {
    local ver="$1" tag asset url
    if [ -n "${PBS_TAG[${ver}]:-}" ]; then
        tag="${PBS_TAG[${ver}]}"
        asset=$(python3 - "$ver" "$tag" <<'PY'
import json, sys, urllib.request
ver, tag = sys.argv[1], sys.argv[2]
url = f"https://api.github.com/repos/astral-sh/python-build-standalone/releases/tags/{tag}"
with urllib.request.urlopen(url, timeout=20) as r:
    assets = json.load(r)["assets"]
for a in assets:
    n = a["name"]
    if n.startswith(f"cpython-{ver}.") and "aarch64-unknown-linux-gnu-install_only.tar.gz" in n \
       and "pgo" not in n and "lto" not in n and "freethreaded" not in n:
        print(n); break
PY
)
    else
        # 动态查找：从最新 release 从新到旧找含该版本的 aarch64 资产
        tag=$(python3 - "$ver" <<'PY'
import json, sys, urllib.request
ver = sys.argv[1]
for page in range(1, 6):
    url = f"https://api.github.com/repos/astral-sh/python-build-standalone/releases?per_page=30&page={page}"
    with urllib.request.urlopen(url, timeout=20) as r:
        rels = json.load(r)
    for rel in rels:
        t = rel["tag_name"]
        for a in rel.get("assets", []):
            n = a["name"]
            if n.startswith(f"cpython-{ver}.") and "aarch64-unknown-linux-gnu-install_only.tar.gz" in n \
               and "pgo" not in n and "lto" not in n and "freethreaded" not in n:
                print(t); raise SystemExit
PY
)
        asset=$(python3 - "$ver" "$tag" <<'PY'
import json, sys, urllib.request
ver, tag = sys.argv[1], sys.argv[2]
url = f"https://api.github.com/repos/astral-sh/python-build-standalone/releases/tags/{tag}"
with urllib.request.urlopen(url, timeout=20) as r:
    assets = json.load(r)["assets"]
for a in assets:
    n = a["name"]
    if n.startswith(f"cpython-{ver}.") and "aarch64-unknown-linux-gnu-install_only.tar.gz" in n \
       and "pgo" not in n and "lto" not in n and "freethreaded" not in n:
        print(n); break
PY
)
    fi
    [ -n "$asset" ] || die "找不到 Python ${ver} 的 aarch64 python-build-standalone"

    local cached="${CACHE_DIR}/py-${ver}-aarch64"
    if [ ! -f "${cached}/include/python${ver}/Python.h" ]; then
        local tarball="${CACHE_DIR}/${asset}"
        if [ ! -f "$tarball" ]; then
            log "下载 aarch64 Python ${ver}（${asset}）..." >&2
            if ! curl -fsSL --connect-timeout 15 -o "$tarball" \
                "https://mirror.nju.edu.cn/github-release/astral-sh/python-build-standalone/${tag}/${asset}"; then
                log "南大镜像失败，尝试 GitHub 直连..." >&2
                curl -fsSL --connect-timeout 20 -o "$tarball" \
                    "https://github.com/astral-sh/python-build-standalone/releases/download/${tag}/${asset}"
            fi
        fi
        mkdir -p "$cached" && tar -xzf "$tarball" -C "$cached" --strip-components=1 python
    fi
    echo "${cached}"
}

# ==================== 逐版本构建 ====================
for ver in "${PYVERS[@]}"; do
    [[ "$ver" =~ ^3\.(8|9|10|11|12|13|14|15)$ ]] || die "不支持的 Python 版本: ${ver}"
    xy="3${ver#3.}"
    log "构建 Python ${ver} (cp${xy}) aarch64 wheel"

    ARMPY=$(get_arm_python "$ver")
    [ -f "${ARMPY}/include/python${ver}/Python.h" ] || die "Python ${ver} 头文件缺失"

    rm -rf "$WORK_DIR" && mkdir -p "$WORK_DIR"
    cd "$REPO_ROOT"

    # 1) 编译全部源文件 → aarch64 .o（-fPIC：共享库需要）
    log "  编译核心库 + 绑定 (aarch64)..."
    # Python 3.8/3.9 头文件依赖 <crypt.h>，zig 的 glibc 头文件集不含：
    # 复制系统声明头（架构无关），缺失时生成最小 stub
    STUB_DIR="${WORK_DIR}/stub"
    mkdir -p "${STUB_DIR}"
    if [ -f /usr/include/crypt.h ]; then
        cp /usr/include/crypt.h "${STUB_DIR}/crypt.h"
    else
        cat > "${STUB_DIR}/crypt.h" <<'EOF'
/* 最小 crypt.h stub（编译 Python 头文件所需，仅声明） */
#ifndef _CRYPT_H
#define _CRYPT_H
struct crypt_data { char *dummy; };
char *crypt(const char *, const char *);
char *crypt_r(const char *, const char *, struct crypt_data *);
#endif
EOF
    fi
    SRCS=(src/robot.cpp src/canport.cpp src/motor.cpp src/crc.cpp src/convert.cpp src/parse_robot_params.cpp
          lib/serial/src/serial.cc lib/serial/src/impl/unix.cc lib/serial/src/impl/list_ports/list_ports_linux.cc
          lib/yaml-cpp/src/*.cpp
          tools/python/src/python_bindings.cpp)
    # 注意：输出名用序号而非 basename —— src/convert.cpp 与
    # lib/yaml-cpp/src/convert.cpp 同名，basename 会互相覆盖导致核心符号丢失
    INCS=(-I "${STUB_DIR}" -I include -I lib/serial/include -I lib/yaml-cpp/include
          -I "${ARMPY}/include/python${ver}"
          -I "${PYB_DIR}/pybind11/include")
    # -Wno-macro-redefined：静音 pyconfig.h 与 zig glibc 头文件的
    # _POSIX_C_SOURCE/_XOPEN_SOURCE 重定义警告（无害，仅编译期宏值）
    INCS_STR="${INCS[*]}"
    NPROC="$(nproc)"
    printf '%s\n' "${SRCS[@]}" | nl -w1 -s'|' | xargs -P"${NPROC}" -I{} bash -c "
        n=\"\${1%%|*}\"; f=\"\${1#*|}\"
        zig c++ -target aarch64-linux-gnu -std=c++17 -O2 -fPIC -Wno-macro-redefined \
            ${INCS_STR} -c \"\$f\" -o \"${WORK_DIR}/o\$n.o\"
    " _ {}
    echo "    ${#SRCS[@]} 个文件编译完成（并行 ${NPROC}）"

    # 2) 链接扩展模块（Python 符号由运行时宿主解析）
    log "  链接 _core.cpython-${xy}-aarch64-linux-gnu.so ..."
    zig c++ -target aarch64-linux-gnu -shared -O2 \
        "${WORK_DIR}"/o*.o -o "${WORK_DIR}/_core.cpython-${xy}-aarch64-linux-gnu.so" \
        -pthread -lrt

    # 3) 打包 wheel
    log "  打包 wheel ..."
    python3 - "$ver" "${xy}" "${WORK_DIR}" "$DIST_DIR" "$REPO_ROOT" "$SCRIPT_DIR" <<'PY'
import csv, hashlib, io, os, pathlib, re, sys, zipfile

ver, xy, work, dist, repo, tools = sys.argv[1:]
work, dist, repo, tools = pathlib.Path(work), pathlib.Path(dist), pathlib.Path(repo), pathlib.Path(tools)
version = re.search(r'version = "([^"]+)"', (tools / "python" / "pyproject.toml").read_text()).group(1)

name, import_name = "hightorque-robot", "hightorque_robot"
files = []

def add(arcname, data):
    files.append((arcname, data))

# 包内容
src_init = tools / "python" / import_name / "__init__.py"
add(f"{import_name}/__init__.py", src_init.read_bytes())
so = work / f"_core.cpython-{xy}-aarch64-linux-gnu.so"
add(f"{import_name}/{so.name}", so.read_bytes())
for yaml_path in sorted((repo / "robot_param").glob("*.yaml")):
    add(f"{import_name}/robot_param/{yaml_path.name}", yaml_path.read_bytes())

# dist-info
metadata = f"""Metadata-Version: 2.1
Name: {name}
Version: {version}
Summary: Python bindings for the hightorque_fdcan high-torque motor SDK (Linux, FDCAN over serial)
Requires-Python: >=3.8
"""
add(f"{import_name}-{version}.dist-info/METADATA", metadata.encode())

wheel_file = f"""Wheel-Version: 1.0
Generator: hightorque-cross-build (zig)
Root-Is-Purelib: false
Tag: cp{xy}-cp{xy}-linux_aarch64
"""
add(f"{import_name}-{version}.dist-info/WHEEL", wheel_file.encode())

# RECORD（按 wheel 规范：csv + sha256 + 最后一行是自身占位）
record_rows = []
for arcname, data in files:
    digest = hashlib.sha256(data).hexdigest()
    record_rows.append((arcname, f"sha256={digest}", str(len(data))))
record_name = f"{import_name}-{version}.dist-info/RECORD"
record_rows.append((record_name, "", ""))

out = dist / f"{import_name}-{version}-cp{xy}-cp{xy}-linux_aarch64.whl"
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as zf:
    for arcname, data in files:
        zf.writestr(arcname, data)
    buf = io.StringIO()
    csv.writer(buf, lineterminator="\n").writerows(record_rows)
    zf.writestr(record_name, buf.getvalue().encode())

print(f"    -> {out.name} ({len(files) + 1} 个文件)")
PY
done

rm -rf "$WORK_DIR"
log "完成：$(ls -lh "${DIST_DIR}"/hightorque_robot-*-linux_aarch64.whl 2>/dev/null | awk '{print $9, $5}' | tr '\n' ' ')"
