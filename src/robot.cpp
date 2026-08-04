#include "robot.h"
#include "parse_robot_params.h"



robot::robot()
{
    const RobotParams robot_params = parse_robot_params();
    // const std::vector<std::string> serial_list = get_serial_list("/dev/ttyACM");

    // if (serial_list.size() < robot_params.canport_num)
    // {
    //     PRINT_ERROR("Not enough serial ports: found %zu, need %d (canport_num)", serial_list.size(), robot_params.canport_num);
    //     exit(1);
    // }

    // for (uint8_t i = 0; i < robot_params.canport_num; i++)
    // {
    //     canports.push_back(std::make_unique<canport>(i + 1, serial_list[i], robot_params));
    //     for (uint8_t j = 0; j < robot_params.canports[i].motor_num; j++)
    //     {
    //         motors.push_back(motor(i + 1, j, canports[i]->get_tdata(), robot_params));
    //     }
    // }
}


robot::~robot()
{
    
}
