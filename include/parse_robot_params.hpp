#ifndef PARSE_ROBOT_PARAMS_HPP
#define PARSE_ROBOT_PARAMS_HPP

#include <yaml-cpp/yaml.h>

struct MotorParams {
    std::string type;
    int id;
    std::string name;
    int num;
    bool pos_limit_enable;
    double pos_upper;
    double pos_lower;
    bool tor_limit_enable;
    double tor_upper;
    double tor_lower;
};

struct CANPortParams {
    int serial_id;
    int motor_num;    
    std::map<std::string, MotorParams> motors;
};

struct CANBoardParams {
    int CANport_num;
    std::map<std::string, CANPortParams> CANports;
};

struct RobotParams {
    int motor_timeout_ms;
    std::string robot_name;
    std::string Serial_Type;
    int Seial_baudrate;
    int control_type;
    int CANboard_num;
    std::map<std::string, CANBoardParams> CANboards;
};

RobotParams parseRobotParams(const std::string& filePath);

#endif
