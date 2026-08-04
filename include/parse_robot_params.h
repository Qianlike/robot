#ifndef _PARSE_ROBOT_PATAMS_H
#define _PARSE_ROBOT_PATAMS_H


#include <string>
#include <vector>
#include "common_macros.h"


struct MotorParams
{
    int id = 0;
    std::string name;
};

struct CANPortParams
{
    int canport_id = 0;
    int motor_num = 0;
    std::vector<MotorParams> motors;
};

struct RobotParams
{
    std::string robot_name;
    int canport_num = 0;
    std::vector<CANPortParams> canports;
};


RobotParams parse_robot_params(void);


#endif
