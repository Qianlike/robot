#include "robot.h"
#include "parse_robot_params.h"



Robot::Robot()
{
    const RobotParams robot_params = parse_robot_params();

    for (uint8_t i = 0; i < robot_params.can_port_num; i++)
    {
        const auto& port_params = robot_params.can_ports[i];

        std::map<uint8_t, std::string> map_id_name;
        for (const auto& motor_param : port_params.motors)
        {
            map_id_name.insert({static_cast<uint8_t>(motor_param.id), motor_param.name});
        }

        can_ports.push_back(std::make_unique<CanPort>(i + 1, map_id_name));

        for (size_t j = 0; j < port_params.motors.size(); j++)
        {
            motors.push_back(Motor(can_ports[i].get(), static_cast<uint8_t>(port_params.motors[j].id)));
        }
    }
}


Robot::~Robot()
{

}


CanPort *Robot::get_can_port(uint8_t can_port_id)
{
    if (can_port_id < 1 || can_port_id > can_ports.size())
    {
        PRINT_ERROR("can_port_id err: got %d, valid range [1, %zu]", can_port_id, can_ports.size());
        return nullptr;
    }
    return can_ports[can_port_id - 1].get();
}


void Robot::position(uint8_t can_port_id, uint8_t id, float pos)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->position(id, pos);
}


void Robot::velocity(uint8_t can_port_id, uint8_t id, float vel)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->velocity(id, vel);
}


void Robot::torque(uint8_t can_port_id, uint8_t id, float tqe)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->torque(id, tqe);
}


void Robot::vel_acc(uint8_t can_port_id, uint8_t id, float vel, float acc)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->vel_acc(id, vel, acc);
}


void Robot::pos_vel_acc(uint8_t can_port_id, uint8_t id, float pos, float vel, float acc)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->pos_vel_acc(id, pos, vel, acc);
}


void Robot::pos_vel_MAXtqe(uint8_t can_port_id, uint8_t id, float pos, float vel, float max_tqe)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->pos_vel_MAXtqe(id, pos, vel, max_tqe);
}


void Robot::pos_vel_tqe_kp_kd(uint8_t can_port_id, uint8_t id, float pos, float vel, float tqe, float kp, float kd)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->pos_vel_tqe_kp_kd(id, pos, vel, tqe, kp, kd);
}


void Robot::stop()
{
    for (auto& can_port : can_ports)
    {
        can_port->stop();
    }
}


void Robot::stop(uint8_t can_port_id, uint8_t id)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->stop(id);
}


void Robot::brake()
{
    for (auto& can_port : can_ports)
    {
        can_port->brake();
    }
}


void Robot::brake(uint8_t can_port_id, uint8_t id)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->brake(id);
}


void Robot::reset()
{
    for (auto& can_port : can_ports)
    {
        can_port->reset();
    }
}


void Robot::reset(uint8_t can_port_id, uint8_t id)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return;
    }
    can_port->reset(id);
}


void Robot::request_motor_state()
{
    for (auto& can_port : can_ports)
    {
        can_port->request_motor_state();
    }
}


void Robot::motor_zero_pos_reset()
{
    for (auto& can_port : can_ports)
    {
        can_port->motor_zero_pos_reset();
    }
}


motor_state_t *Robot::get_motor_state(uint8_t can_port_id, uint8_t id)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return nullptr;
    }
    return can_port->get_motor_state(id);
}


void Robot::send()
{
    for (auto& can_port : can_ports)
    {
        can_port->send();
    }
}


fdcan_state_s *Robot::get_can_port_state(uint8_t can_port_id)
{
    CanPort *can_port = get_can_port(can_port_id);
    if (can_port == nullptr)
    {
        return nullptr;
    }
    return can_port->get_can_port_state();
}
