#ifndef HIGHTORQUE_PARSE_ROBOT_PARAMS_H
#define HIGHTORQUE_PARSE_ROBOT_PARAMS_H


#include <string>
#include <vector>
#include "common_macros.h"


struct MotorParams
{
    int id = 0;
    std::string name;
};

struct CanPortParams
{
    int can_port_id = 0;
    int motor_num = 0;
    std::vector<MotorParams> motors;
};

struct RobotParams
{
    std::string robot_name;
    int can_port_num = 0;
    std::vector<CanPortParams> can_ports;
};


RobotParams parse_robot_params(void);
RobotParams parse_robot_params(const std::string& config_path);


#endif
