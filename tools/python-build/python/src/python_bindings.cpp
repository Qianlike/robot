// hightorque_fdcan SDK 的 pybind11 绑定（模块 _core）
//
// 设计约定：
//   - get_motor_state()/get_can_port_state() 返回 C++ 裸指针（指向内部对象、可能为
//     nullptr，且 recv 后台线程并发更新），一律按值拷贝为快照返回
//   - parse_robot_params 只绑定带路径版本；配置文件由 Python 调用方提供，
//     且核心代码中 exit(-1)/exit(1) 会直接终止 Python 进程，故不暴露

#include "robot.h"
#include "canport.h"
#include "motor.h"
#include "parse_robot_params.h"
#include "serial_struct.h"
#include "version.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>

namespace py = pybind11;

// 仅绑定 Robot、Motor、CanPort 的 public 控制方法，以及控制所需的数据结构和
// 配置解析函数；C++ private 方法和底层协议辅助接口不对 Python 开放。
PYBIND11_MODULE(_core, m)
{
    m.doc() = "hightorque_fdcan high-torque motor SDK Python bindings";

    // ==================== 版本 ====================
    m.attr("__version__") = py::str(
        std::to_string(static_cast<int>(VER_GET_MAJOR(HIGHTORQUE_ROBOT_VERSION))) + "." +
        std::to_string(static_cast<int>(VER_GET_MINOR(HIGHTORQUE_ROBOT_VERSION))) + "." +
        std::to_string(static_cast<int>(VER_GET_PATCH(HIGHTORQUE_ROBOT_VERSION))));

    // ==================== 版本号结构（version_s） ====================
    // 注意：major/minor/patch 与 data32 是同一联合体的别名，写入会互相覆盖
    py::class_<version_s>(m, "VersionInfo")
        .def(py::init([]() {
            version_s v;
            v.data32 = 0;
            return v;
        }))
        .def_readwrite("patch", &version_s::patch)
        .def_readwrite("minor", &version_s::minor)
        .def_readwrite("major", &version_s::major)
        .def_readwrite("data32", &version_s::data32)
        .def("__repr__", [](const version_s& v) {
            return "v" + std::to_string(static_cast<int>(v.major)) + "." +
                   std::to_string(static_cast<int>(v.minor)) + "." +
                   std::to_string(static_cast<int>(v.patch));
        });

    // ==================== 电机状态（motor_state_t） ====================
    // time 是 steady_clock::time_point，无 pybind11 caster，不绑定。
    py::class_<motor_state_t>(m, "MotorState")
        .def(py::init([]() {
            motor_state_t s = {};
            return s;
        }))
        .def_readwrite("mode", &motor_state_t::mode)
        .def_readwrite("fault", &motor_state_t::fault)
        .def_readwrite("position", &motor_state_t::position)
        .def_readwrite("velocity", &motor_state_t::velocity)
        .def_readwrite("torque", &motor_state_t::torque)
        .def_readwrite("num", &motor_state_t::num)
        .def_readwrite("fw_version", &motor_state_t::fw_version)
        .def_readwrite("model", &motor_state_t::model)
        .def_readwrite("name", &motor_state_t::name)
        .def_readwrite("flag", &motor_state_t::flag)
        .def("__repr__", [](const motor_state_t& s) {
            return "MotorState(id=" + std::to_string(0) + ", mode=" + std::to_string(s.mode) +
                   ", fault=" + std::to_string(s.fault) +
                   ", pos=" + std::to_string(s.position) +
                   ", vel=" + std::to_string(s.velocity) +
                   ", tqe=" + std::to_string(s.torque) + ")";
        });

    // ==================== FDCAN 状态 ====================
    py::enum_<fdcan_fault_t>(m, "FdcanFault")
        .value("UNKNOWN", FDCAN_STATUS_UNKNOWN)
        .value("OK", FDCAN_STATUS_OK)
        .value("ERROR_WARNING", FDCAN_STATUS_ERROR_WARNING)
        .value("ERROR_PASSIVE", FDCAN_STATUS_ERROR_PASSIVE)
        .value("BUS_OFF", FDCAN_STATUS_BUS_OFF);

    py::class_<fdcan_state_s>(m, "FdcanState")
        .def(py::init([]() {
            fdcan_state_s s = {};
            return s;
        }))
        .def_readwrite("fault", &fdcan_state_s::fault)
        .def_readwrite("tx_err_num", &fdcan_state_s::tx_err_num)
        .def_readwrite("rx_err_num", &fdcan_state_s::rx_err_num);

    // ==================== 参数结构 ====================
    py::class_<MotorParams>(m, "MotorParams")
        .def(py::init([]() {
            MotorParams p;
            return p;
        }))
        .def_readwrite("id", &MotorParams::id)
        .def_readwrite("name", &MotorParams::name);

    py::class_<CanPortParams>(m, "CanPortParams")
        .def(py::init([]() {
            CanPortParams p;
            return p;
        }))
        .def_readwrite("can_port_id", &CanPortParams::can_port_id)
        .def_readwrite("motor_num", &CanPortParams::motor_num)
        .def_readwrite("motors", &CanPortParams::motors);

    py::class_<RobotParams>(m, "RobotParams")
        .def(py::init([]() {
            RobotParams p;
            return p;
        }))
        .def_readwrite("robot_name", &RobotParams::robot_name)
        .def_readwrite("can_port_num", &RobotParams::can_port_num)
        .def_readwrite("can_ports", &RobotParams::can_ports);

    // ==================== 参数解析 ====================
    // 只暴露带路径版本（无参版本按 CWD 相对路径读取且出错会 exit(-1)）
    m.def("parse_robot_params",
          static_cast<RobotParams (*)(const std::string&)>(&parse_robot_params),
          py::arg("config_path"), "解析 robot_config.yaml，返回 RobotParams");

    // ==================== CanPort ====================
    py::class_<CanPort>(m, "CanPort", "单个通信板的 FDCAN 端口（构造需要硬件串口）")
        .def(py::init<uint8_t, const std::map<uint8_t, std::string>&>(),
             py::arg("can_port_id"), py::arg("map_id_name"), "按 电机id->名字 映射构造")
        .def(py::init([](uint8_t can_port_id, const py::sequence& ids) {
                 std::map<uint8_t, std::string> map_id_name;
                 for (const auto& item : ids)
                 {
                     map_id_name.emplace(item.cast<int>(), std::string());
                 }
                 return std::unique_ptr<CanPort>(new CanPort(can_port_id, map_id_name));
             }),
             py::arg("can_port_id"), py::arg("ids"), "按电机 id 列表构造")
        .def("position", &CanPort::position, py::arg("id"), py::arg("pos"))
        .def("velocity", &CanPort::velocity, py::arg("id"), py::arg("vel"))
        .def("torque", &CanPort::torque, py::arg("id"), py::arg("tqe"))
        .def("vel_acc", &CanPort::vel_acc, py::arg("id"), py::arg("vel"), py::arg("acc"))
        .def("pos_vel_acc", &CanPort::pos_vel_acc, py::arg("id"), py::arg("pos"), py::arg("vel"), py::arg("acc"))
        .def("pos_vel_MAXtqe", &CanPort::pos_vel_MAXtqe, py::arg("id"), py::arg("pos"), py::arg("vel"), py::arg("max_tqe"))
        .def("pos_vel_tqe_kp_kd", &CanPort::pos_vel_tqe_kp_kd, py::arg("id"), py::arg("pos"), py::arg("vel"), py::arg("tqe"), py::arg("kp"), py::arg("kd"))
        .def("stop", py::overload_cast<>(&CanPort::stop))
        .def("stop", py::overload_cast<uint8_t>(&CanPort::stop), py::arg("id"))
        .def("brake", py::overload_cast<>(&CanPort::brake))
        .def("brake", py::overload_cast<uint8_t>(&CanPort::brake), py::arg("id"))
        .def("reset", py::overload_cast<>(&CanPort::reset))
        .def("reset", py::overload_cast<uint8_t>(&CanPort::reset), py::arg("id"))
        .def("request_motor_state", &CanPort::request_motor_state)
        .def("motor_zero_pos_reset", &CanPort::motor_zero_pos_reset)
        .def("send", &CanPort::send)
        .def("get_motor_state", &CanPort::get_motor_state, py::arg("id"),
             py::return_value_policy::copy, "电机状态快照（None 表示电机不存在）")
        .def("get_can_port_state", &CanPort::get_can_port_state,
             py::return_value_policy::copy, "FDCAN 端口状态快照")
        .def_property_readonly("map_motors_state", [](const CanPort& c) {
            return c.map_motors_state;
        }, "电机状态字典快照 {id: MotorState}")
        .def_property_readonly("can_port_state", [](const CanPort& c) {
            return c.can_port_state;
        }, "FDCAN 端口状态快照");

    // ==================== Motor ====================
    py::class_<Motor>(m, "Motor", "电机句柄（非拥有，引用所属 CanPort，需保持 CanPort 存活）")
        .def(py::init<CanPort*, uint8_t>(), py::arg("can_port"), py::arg("id"),
             py::keep_alive<1, 2>())
        .def("position", &Motor::position, py::arg("pos"))
        .def("velocity", &Motor::velocity, py::arg("vel"))
        .def("torque", &Motor::torque, py::arg("tqe"))
        .def("vel_acc", &Motor::vel_acc, py::arg("vel"), py::arg("acc"))
        .def("pos_vel_acc", &Motor::pos_vel_acc, py::arg("pos"), py::arg("vel"), py::arg("acc"))
        .def("pos_vel_MAXtqe", &Motor::pos_vel_MAXtqe, py::arg("pos"), py::arg("vel"), py::arg("max_tqe"))
        .def("pos_vel_tqe_kp_kd", &Motor::pos_vel_tqe_kp_kd, py::arg("pos"), py::arg("vel"), py::arg("tqe"), py::arg("kp"), py::arg("kd"))
        .def("stop", &Motor::stop)
        .def("brake", &Motor::brake)
        .def("reset", &Motor::reset)
        .def("request_motor_state", &Motor::request_motor_state)
        .def("get_motor_state", &Motor::get_motor_state, py::return_value_policy::copy)
        .def("get_id", &Motor::get_id);

    // ==================== Robot ====================
    py::class_<Robot>(m, "Robot", "机器人控制接口（构造需要硬件串口，config_path 必传）")
        .def(py::init<const std::string&>(), py::arg("config_path"),
             "robot_config.yaml 的外部路径（由 Python 调用方提供）")
        .def("request_motor_state", &Robot::request_motor_state)
        .def("motor_zero_pos_reset", &Robot::motor_zero_pos_reset)
        .def("send", &Robot::send)
        .def("position", &Robot::position, py::arg("can_port_id"), py::arg("id"), py::arg("pos"))
        .def("velocity", &Robot::velocity, py::arg("can_port_id"), py::arg("id"), py::arg("vel"))
        .def("torque", &Robot::torque, py::arg("can_port_id"), py::arg("id"), py::arg("tqe"))
        .def("vel_acc", &Robot::vel_acc, py::arg("can_port_id"), py::arg("id"), py::arg("vel"), py::arg("acc"))
        .def("pos_vel_acc", &Robot::pos_vel_acc, py::arg("can_port_id"), py::arg("id"), py::arg("pos"), py::arg("vel"), py::arg("acc"))
        .def("pos_vel_MAXtqe", &Robot::pos_vel_MAXtqe, py::arg("can_port_id"), py::arg("id"), py::arg("pos"), py::arg("vel"), py::arg("max_tqe"))
        .def("pos_vel_tqe_kp_kd", &Robot::pos_vel_tqe_kp_kd, py::arg("can_port_id"), py::arg("id"), py::arg("pos"), py::arg("vel"), py::arg("tqe"), py::arg("kp"), py::arg("kd"))
        .def("stop", py::overload_cast<>(&Robot::stop))
        .def("stop", py::overload_cast<uint8_t, uint8_t>(&Robot::stop), py::arg("can_port_id"), py::arg("id"))
        .def("brake", py::overload_cast<>(&Robot::brake))
        .def("brake", py::overload_cast<uint8_t, uint8_t>(&Robot::brake), py::arg("can_port_id"), py::arg("id"))
        .def("reset", py::overload_cast<>(&Robot::reset))
        .def("reset", py::overload_cast<uint8_t, uint8_t>(&Robot::reset), py::arg("can_port_id"), py::arg("id"))
        .def("get_motor_state", &Robot::get_motor_state, py::arg("can_port_id"), py::arg("id"),
             py::return_value_policy::copy, "电机状态快照（None 表示电机不存在）")
        .def("get_can_port_state", &Robot::get_can_port_state, py::arg("can_port_id"),
             py::return_value_policy::copy, "FDCAN 端口状态快照")
        .def_property_readonly("can_ports", [](Robot& r) {
            py::object self = py::cast(&r, py::return_value_policy::reference);
            py::tuple result(r.can_ports.size());
            for (size_t i = 0; i < r.can_ports.size(); i++)
            {
                result[i] = py::cast(r.can_ports[i].get(),
                                     py::return_value_policy::reference_internal, self);
            }
            return result;
        }, "CanPort 列表（元素与 Robot 生命周期绑定）")
        .def_property_readonly("motors", [](Robot& r) {
            py::object self = py::cast(&r, py::return_value_policy::reference);
            py::list result;
            for (const auto& motor : r.motors)
            {
                result.append(py::cast(motor, py::return_value_policy::reference_internal, self));
            }
            return result;
        }, "Motor 列表（元素与 Robot 生命周期绑定）");
}
