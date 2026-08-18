"""绑定层基础测试：结构体往返和公共类方法名称（不触碰硬件）。"""
from importlib.metadata import version as package_version

import pytest

import hightorque_robot
from hightorque_robot import (
    CanPort,
    FdcanFault,
    FdcanState,
    Motor,
    MotorState,
    Robot,
    VersionInfo,
)
from hightorque_robot import _core


def test_version_consistency():
    assert hightorque_robot.__version__ == _core.__version__
    assert hightorque_robot.__version__.count(".") == 2
    parts = hightorque_robot.__version__.split(".")
    assert all(p.isdigit() for p in parts)


def test_package_metadata_version_consistency():
    assert package_version("hightorque-robot") == hightorque_robot.__version__


def test_motor_state_defaults():
    s = MotorState()
    assert s.mode == 0
    assert s.fault == 0
    assert s.position == 0.0
    assert s.velocity == 0.0
    assert s.torque == 0.0
    assert s.num == 0
    assert s.model == ""
    assert s.name == ""
    assert "MotorState" in repr(s)


def test_motor_state_fields():
    s = MotorState()
    s.mode = 3
    s.fault = 0
    s.position = 1.5
    s.velocity = -0.2
    s.torque = 2.0
    s.num = 42
    s.fw_version.major = 6
    s.name = "joint_1"
    # C++ 侧是 float32，转 Python float 有二进制精度展开
    assert s.position == pytest.approx(1.5, abs=1e-7)
    assert s.velocity == pytest.approx(-0.2, abs=1e-7)
    assert s.torque == pytest.approx(2.0, abs=1e-7)
    assert s.num == 42
    assert s.name == "joint_1"
    assert s.fw_version.major == 6


def test_version_info_union():
    v = VersionInfo()
    v.major = 6
    v.minor = 0
    v.patch = 0
    assert v.major == 6
    assert "v6.0.0" in repr(v)
    # union 别名：写 data32 会覆盖字节字段
    v.data32 = 0
    assert v.major == 0


def test_fdcan_state_defaults():
    s = FdcanState()
    # 零初始化的 fdcan_fault_t 枚举值为 0 == FDCAN_STATUS_OK
    assert s.fault == FdcanFault.OK
    assert s.tx_err_num == 0
    assert s.rx_err_num == 0
    s.tx_err_num = 3
    s.rx_err_num = 1
    assert s.tx_err_num == 3
    assert s.rx_err_num == 1


def test_fdcan_fault_values():
    assert FdcanFault.UNKNOWN == -1
    assert FdcanFault.OK == 0
    assert FdcanFault.ERROR_WARNING == 1
    assert FdcanFault.ERROR_PASSIVE == 2
    assert FdcanFault.BUS_OFF == 3


def test_parse_robot_params_signature():
    # Python 绑定只提供带 config_path 的版本。
    with pytest.raises(TypeError):
        _core.parse_robot_params()


def test_cpp_public_method_names_are_bound_verbatim():
    """C++ public control methods must keep their names in Python."""
    expected_methods = {
        CanPort: {
            "position",
            "velocity",
            "torque",
            "vel_acc",
            "pos_vel_acc",
            "pos_vel_MAXtqe",
            "pos_vel_tqe_kp_kd",
            "stop",
            "brake",
            "reset",
            "request_motor_state",
            "motor_zero_pos_reset",
            "get_motor_state",
            "send",
            "get_can_port_state",
        },
        Motor: {
            "position",
            "velocity",
            "torque",
            "vel_acc",
            "pos_vel_acc",
            "pos_vel_MAXtqe",
            "pos_vel_tqe_kp_kd",
            "stop",
            "brake",
            "reset",
            "request_motor_state",
            "get_motor_state",
            "get_id",
        },
        Robot: {
            "request_motor_state",
            "motor_zero_pos_reset",
            "send",
            "position",
            "velocity",
            "torque",
            "vel_acc",
            "pos_vel_acc",
            "pos_vel_MAXtqe",
            "pos_vel_tqe_kp_kd",
            "stop",
            "brake",
            "reset",
            "get_motor_state",
            "get_can_port_state",
        },
    }

    for cls, names in expected_methods.items():
        assert names <= set(dir(cls)), cls.__name__

    # These were previously renamed by the Python binding and must not be
    # mistaken for the C++ API names.
    assert not hasattr(Motor, "id")
    assert not hasattr(CanPort, "motors_state")
    assert hasattr(CanPort, "map_motors_state")


def test_unnecessary_core_helpers_are_not_exposed():
    """The Python module stays limited to the SDK control/data interface."""
    removed_names = {
        "detect_com_ports",
        "motor_state_age",
        "pos_float2int",
        "vel_float2int",
        "tqe_float2int",
        "acc_float2int",
        "kp_float2int",
        "kd_float2int",
        "pos_int2float",
        "vel_int2float",
        "tqe_int2float",
        "MODE_POSITION",
        "MODE_VELOCITY",
        "MODE_TORQUE",
        "MODE_VOLTAGE",
        "MODE_CURRENT",
        "MODE_STOP",
        "MODE_BRAKE",
        "MODE_RESET",
        "MODE_VEL_ACC",
        "MODE_POS_VEL_TQE",
        "MODE_POS_VEL_ACC",
        "MODE_POS_VEL_TQE_KP_KD",
        "MODE_MOTOR_STATE",
        "MODE_MOTOR_VERSION",
        "MODE_MOTOR_MODEL",
        "MODE_MOTOR_POS_RESET",
        "MY_PI",
        "MY_2PI",
        "can_port_count",
        "motor_count",
        "time_since_epoch_seconds",
    }

    assert removed_names.isdisjoint(dir(_core))
    assert not hasattr(hightorque_robot, "detect_com_ports")
    assert not hasattr(hightorque_robot, "motor_state_age")
