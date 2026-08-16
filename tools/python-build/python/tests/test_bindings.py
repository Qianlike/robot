"""绑定层基础测试：结构体往返、常量、纯函数（不触碰硬件）。"""
from importlib.metadata import version as package_version

import pytest

import hightorque_robot
from hightorque_robot import FdcanFault, FdcanState, MotorState, VersionInfo
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


def test_motor_state_age_nonnegative():
    s = MotorState()
    assert hightorque_robot.motor_state_age(s) >= 0.0


def test_detect_com_ports_returns_str_list():
    ports = hightorque_robot.detect_com_ports()
    assert isinstance(ports, list)
    for p in ports:
        assert isinstance(p, str)


def test_mode_constants():
    assert _core.MODE_POSITION == 0x80
    assert _core.MODE_VELOCITY == 0x81
    assert _core.MODE_TORQUE == 0x82
    assert _core.MODE_STOP == 0x85
    assert _core.MODE_BRAKE == 0x86
    assert _core.MODE_RESET == 0x87
    assert _core.MODE_VEL_ACC == 0x90
    assert _core.MODE_POS_VEL_TQE == 0x92
    assert _core.MODE_POS_VEL_ACC == 0x95
    assert _core.MODE_POS_VEL_TQE_KP_KD == 0x98
    assert _core.MODE_MOTOR_STATE == 20


def test_pi_constants():
    assert _core.MY_PI == pytest.approx(3.14159, abs=1e-5)
    assert _core.MY_2PI == pytest.approx(6.28318, abs=1e-5)


def test_convert_roundtrip():
    # 注意：convert.cpp 先转圈数（radian/2π）再量化，量化步长为 2π/SCALE
    # pos 步长 2π/10000 ≈ 6.3e-4；vel 步长 2π/4000 ≈ 1.6e-3；
    # tqe 同样过圈数换算（C++ 现有行为），步长 2π/100 ≈ 6.3e-2
    raw = _core.pos_float2int(1.5)
    assert isinstance(raw, int)
    assert _core.pos_int2float(raw) == pytest.approx(1.5, abs=1e-3)

    raw_vel = _core.vel_float2int(-0.5)
    assert _core.vel_int2float(raw_vel) == pytest.approx(-0.5, abs=2e-3)

    raw_tqe = _core.tqe_float2int(2.0)
    assert _core.tqe_int2float(raw_tqe) == pytest.approx(2.0, abs=0.1)


def test_parse_robot_params_signature():
    # Python 绑定只提供带 config_path 的版本。
    with pytest.raises(TypeError):
        _core.parse_robot_params()
