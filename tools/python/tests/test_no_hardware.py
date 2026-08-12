"""无硬件环境的测试（CI/容器）。

开发机连接了通信板时这些测试自动跳过（构造 Robot 需要真实硬件）。
"""
import pathlib

import pytest

import hightorque_robot


def _has_hardware() -> bool:
    return len(hightorque_robot.detect_com_ports()) > 0


requires_no_hardware = pytest.mark.skipif(
    _has_hardware(), reason="开发机已连接通信板硬件，跳过无硬件用例"
)


@requires_no_hardware
def test_robot_raises_without_hardware():
    """无通信板时 Robot() 应抛 RuntimeError 而非让 C++ exit() 杀进程。"""
    with pytest.raises(RuntimeError):
        hightorque_robot.Robot()


def test_bundled_config_data_exists():
    """wheel 内置 robot_param 数据文件齐全。"""
    pkg_dir = pathlib.Path(hightorque_robot.__file__).resolve().parent
    for name in ("robot_config.yaml", "1dof_params.yaml", "70dof_params.yaml"):
        assert (pkg_dir / "robot_param" / name).is_file(), name


def test_parse_params_without_hardware():
    """参数解析不触碰硬件，无硬件环境可用。"""
    params = hightorque_robot.parse_robot_params()
    assert params.robot_name == "Single_Motor"


def test_resolve_config_falls_back_to_bundled(monkeypatch, tmp_path):
    """在没有任何 robot_param 目录的 CWD 下，默认解析 fallback 到 wheel 内置配置。"""
    # 仓库根 CWD 存在 robot_param/（会优先命中），切到空目录验证 fallback
    monkeypatch.chdir(tmp_path)
    monkeypatch.delenv("HIGHTORQUE_ROBOT_CONFIG", raising=False)

    path = hightorque_robot.resolve_config_path()
    bundled = (
        pathlib.Path(hightorque_robot.__file__).resolve().parent
        / "robot_param"
        / "robot_config.yaml"
    )
    assert path == bundled
