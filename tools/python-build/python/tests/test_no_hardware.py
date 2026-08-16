"""无硬件环境的测试（CI/容器）。"""

import pathlib

import pytest

import hightorque_robot


def test_robot_requires_config_path():
    """Python wheel 不提供 Robot(void)，必须显式传入配置文件路径。"""
    with pytest.raises(TypeError, match="config_path"):
        hightorque_robot.Robot()


def test_config_is_not_bundled(monkeypatch, tmp_path):
    """wheel 不包含 robot_param，且没有外部配置时不能解析默认路径。"""
    pkg_dir = pathlib.Path(hightorque_robot.__file__).resolve().parent
    assert not (pkg_dir / "robot_param").exists()

    monkeypatch.chdir(tmp_path)
    monkeypatch.delenv("HIGHTORQUE_ROBOT_CONFIG", raising=False)
    with pytest.raises(FileNotFoundError):
        hightorque_robot.resolve_config_path()
