#include "robot.h"
#include "parse_robot_params.h"



Robot::Robot()
{
    const RobotParams robot_params = parse_robot_params();
    // const std::vector<std::string> serial_list = get_serial_list("/dev/ttyACM");

    // if (serial_list.size() < robot_params.can_port_num)
    // {
    //     PRINT_ERROR("Not enough serial ports: found %zu, need %d (can_port_num)", serial_list.size(), robot_params.can_port_num);
    //     exit(1);
    // }

    // for (uint8_t i = 0; i < robot_params.can_port_num; i++)
    // {
    //     can_ports.push_back(std::make_unique<CanPort>(i + 1, serial_list[i], robot_params));
    //     for (uint8_t j = 0; j < robot_params.can_ports[i].motor_num; j++)
    //     {
    //         motors.push_back(motor(i + 1, j, can_ports[i]->get_tdata(), robot_params));
    //     }
    // }
}


Robot::~Robot()
{
    
}
