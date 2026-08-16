"""参数解析测试：外部配置文件和 param_file 相对路径语义。"""
import pytest

import hightorque_robot


def _write_robot_config(tmp_path, param_file_value, motors_yaml):
    config = tmp_path / "robot_config.yaml"
    config.write_text('param_file: "%s"\n' % param_file_value)
    (tmp_path / "motors.yaml").write_text(motors_yaml)
    return config


MOTORS_YAML = (
    'robot:\n'
    '  robot_name: "Test_Robot"\n'
    '  canport_num: 1\n'
    '  canport:\n'
    '    canport_1:\n'
    '      canport_id: 1\n'
    '      motor_num: 1\n'
    '      motor:\n'
    '        motor_1:\n'
    '          id: 1\n'
    '          name: "test_motor"\n'
)


def test_parse_external_config(tmp_path):
    config = _write_robot_config(tmp_path, "motors.yaml", MOTORS_YAML)
    params = hightorque_robot.parse_robot_params(config)
    assert params.robot_name == "Test_Robot"
    assert params.can_port_num == 1
    assert len(params.can_ports) == 1

    port = params.can_ports[0]
    assert port.can_port_id == 1
    assert port.motor_num == 1
    assert len(port.motors) == 1

    motor = port.motors[0]
    assert motor.id == 1
    assert motor.name == "test_motor"


def test_param_file_relative_to_config_dir(tmp_path):
    """param_file 相对 config 所在目录解析（不依赖 CWD）。"""
    sub = tmp_path / "sub"
    sub.mkdir()
    (sub / "motors.yaml").write_text(MOTORS_YAML)
    config = tmp_path / "robot_config.yaml"
    config.write_text('param_file: "sub/motors.yaml"\n')

    # 故意把 CWD 切到无关目录，验证不依赖 CWD
    params = hightorque_robot.parse_robot_params(config)
    assert params.robot_name == "Test_Robot"
    assert params.can_ports[0].motors[0].name == "test_motor"


def test_param_file_upward_relative(tmp_path):
    """param_file 用 ../ 指向 config 上级目录。"""
    (tmp_path / "motors.yaml").write_text(MOTORS_YAML)
    config_dir = tmp_path / "cfg"
    config_dir.mkdir()
    config = config_dir / "robot_config.yaml"
    config.write_text('param_file: "../motors.yaml"\n')

    params = hightorque_robot.parse_robot_params(config)
    assert params.robot_name == "Test_Robot"


def test_legacy_cwd_layout_equivalence(monkeypatch, tmp_path):
    """复现 C++ 原始 CWD 布局（../robot_param/），新旧行为等价。"""
    cwd_dir = tmp_path / "proj"
    param_dir = tmp_path / "robot_param"  # CWD 的上级目录
    cwd_dir.mkdir()
    param_dir.mkdir()

    (param_dir / "1dof_params.yaml").write_text(MOTORS_YAML)
    (param_dir / "robot_config.yaml").write_text(
        'param_file: "../robot_param/1dof_params.yaml"\n'
    )

    monkeypatch.chdir(cwd_dir)
    params = hightorque_robot.parse_robot_params()
    assert params.robot_name == "Test_Robot"
    assert params.can_ports[0].motors[0].name == "test_motor"


def test_resolve_config_path_explicit(tmp_path):
    config = tmp_path / "robot_config.yaml"
    config.write_text('param_file: "motors.yaml"\n')
    path = hightorque_robot.resolve_config_path(config)
    assert path == config


def test_resolve_config_path_env(monkeypatch, tmp_path):
    (tmp_path / "robot_config.yaml").write_text('param_file: "motors.yaml"\n')
    (tmp_path / "motors.yaml").write_text(MOTORS_YAML)
    monkeypatch.setenv("HIGHTORQUE_ROBOT_CONFIG", str(tmp_path / "robot_config.yaml"))

    path = hightorque_robot.resolve_config_path()
    assert path == tmp_path / "robot_config.yaml"
    assert hightorque_robot.parse_robot_params().robot_name == "Test_Robot"


def test_resolve_config_path_not_found(tmp_path):
    """显式路径不存在时抛 FileNotFoundError。"""
    with pytest.raises(FileNotFoundError):
        hightorque_robot.resolve_config_path(tmp_path / "no_such.yaml")
