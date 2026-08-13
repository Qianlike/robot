"""pytest 公共配置。

不向 sys.path 插入源码树：测试必须在安装后的环境运行
（本地 `pip install -e tools/python`，或 cibuildwheel 容器内的 wheel 安装）。
"""
import pathlib

import pytest

import hightorque_robot


@pytest.fixture(scope="session")
def bundled_config() -> pathlib.Path:
    """wheel 内置的 robot_config.yaml 绝对路径。"""
    return (
        pathlib.Path(hightorque_robot.__file__).resolve().parent
        / "robot_param"
        / "robot_config.yaml"
    )


@pytest.fixture(scope="session")
def bundled_1dof_params() -> pathlib.Path:
    """wheel 内置的 1dof_params.yaml 绝对路径。"""
    return (
        pathlib.Path(hightorque_robot.__file__).resolve().parent
        / "robot_param"
        / "1dof_params.yaml"
    )


@pytest.fixture(scope="session")
def bundled_70dof_params() -> pathlib.Path:
    """wheel 内置的 70dof_params.yaml 绝对路径。"""
    return (
        pathlib.Path(hightorque_robot.__file__).resolve().parent
        / "robot_param"
        / "70dof_params.yaml"
    )
