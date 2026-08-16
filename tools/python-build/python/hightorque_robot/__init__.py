"""hightorque_robot —— hightorque_fdcan 高扭电机 SDK 的 Python 绑定。

用法::

    from hightorque_robot import Robot, parse_robot_params

    robot = Robot("/path/to/robot_config.yaml")   # 必须显式指定
    params = parse_robot_params("/path/to/robot_config.yaml")

参数文件查找顺序（resolve_config_path）:
    1. 显式传入的 config_path
    2. 环境变量 ``HIGHTORQUE_ROBOT_CONFIG``
    3. 当前目录 ``robot_param/robot_config.yaml``
    4. 当前目录 ``../robot_param/robot_config.yaml``（保持 C++ 原始行为）
    wheel 不包含配置文件，调用方必须提供外部配置文件。

注意:
    - 支持 Linux 和 Windows 10；Windows 按 USB ``MI_xx`` 接口号映射 CAN 通道。
    - 构造 ``Robot``/``CanPort`` 需要通信板硬件（VID:PID=CAF1:FFFF）；
      串口数量不足时会抛 ``RuntimeError``。端口存在但打开失败/握手超时等场景
      C++ 端会 ``exit()`` 直接终止进程（SDK 现状，暂未改造为异常）
"""
from __future__ import annotations

import os
from pathlib import Path
from typing import List, Optional, Union

from . import _core
from ._core import (
    CanPort as CanPort,
    CanPortParams as CanPortParams,
    FdcanFault as FdcanFault,
    FdcanState as FdcanState,
    Motor as Motor,
    MotorParams as MotorParams,
    MotorState as MotorState,
    RobotParams as RobotParams,
    VersionInfo as VersionInfo,
    __version__ as _core_version,
    detect_com_ports as detect_com_ports,
    motor_state_age as motor_state_age,
    parse_robot_params as _parse_robot_params,
)

__version__ = _core_version

def resolve_config_path(config_path: Optional[Union[str, Path]] = None) -> Path:
    """解析 robot_config.yaml 的路径，返回存在的文件路径。

    查找顺序见模块文档。全部不存在时抛 ``FileNotFoundError``。
    """
    candidates: List[Path] = []

    if config_path is not None:
        explicit = Path(config_path)
        # 显式指定的路径必须存在，不存在直接报错（避免静默回退掩盖配置错误）
        if not explicit.is_file():
            raise FileNotFoundError("指定的配置文件不存在: %s" % explicit)
        candidates.append(explicit)

    env_path = os.environ.get("HIGHTORQUE_ROBOT_CONFIG")
    if env_path:
        candidates.append(Path(env_path))

    cwd = Path.cwd()
    candidates.append(cwd / "robot_param" / "robot_config.yaml")
    candidates.append(cwd / ".." / "robot_param" / "robot_config.yaml")
    for candidate in candidates:
        if candidate.is_file():
            return candidate

    raise FileNotFoundError(
        "找不到 robot_config.yaml，已尝试：\n  " + "\n  ".join(str(c) for c in candidates)
    )


def parse_robot_params(config_path: Optional[Union[str, Path]] = None) -> RobotParams:
    """解析机器人参数文件，返回 RobotParams（robot_name/can_port_num/can_ports）。

    仅读取文件、不触碰硬件，无硬件环境也可调用。
    """
    # C++ 的 get_dirname() 以 '/' 分隔目录；Windows Path 默认输出反斜杠，
    # 因此传入绑定前统一转成绝对 POSIX 风格路径。
    resolved_path = resolve_config_path(config_path).resolve()
    return _parse_robot_params(resolved_path.as_posix())


class Robot(_core.Robot):
    """机器人控制接口（对 pybind11 绑定类的 Python 封装）。

    构造前先解析参数并预检通信板串口数量，不足时抛 ``RuntimeError``
    （避免 C++ 端 exit() 直接终止 Python 进程）。构造成功后：
      - ``robot.params``   解析出的参数结构（RobotParams）
      - ``robot.config_path`` 实际使用的配置文件路径

    注意：端口存在但打开失败/握手超时仍可能触发 C++ 端 exit()。
    """

    def __init__(self, config_path: Union[str, Path]):
        if config_path is None:
            raise TypeError("Robot() requires an explicit config_path")
        self.config_path = resolve_config_path(config_path).resolve().as_posix()
        self.params = _parse_robot_params(self.config_path)

        ports = detect_com_ports()
        if len(ports) < self.params.can_port_num:
            raise RuntimeError(
                "需要 %d 个通信板串口 (VID:PID=CAF1:FFFF)，"
                "当前检测到 %d 个：%s" % (self.params.can_port_num, len(ports), ports or "无")
            )

        super().__init__(self.config_path)


__all__ = [
    "Robot",
    "CanPort",
    "Motor",
    "MotorState",
    "FdcanState",
    "FdcanFault",
    "VersionInfo",
    "RobotParams",
    "CanPortParams",
    "MotorParams",
    "parse_robot_params",
    "resolve_config_path",
    "detect_com_ports",
    "motor_state_age",
    "__version__",
]
