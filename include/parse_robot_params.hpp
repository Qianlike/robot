#ifndef PARSE_ROBOT_PARAMS_HPP
#define PARSE_ROBOT_PARAMS_HPP

#include <yaml-cpp/yaml.h>


struct MotorNameComparator 
{
    bool operator()(const std::string& a, const std::string& b) const 
    {
        int numA = extractNumber(a);
        int numB = extractNumber(b);

        return numA < numB;
    }

private:
    int extractNumber(const std::string& str) const 
    {
        std::string numberStr;
        

        for (char ch : str) 
        {
            if (std::isdigit(ch)) 
            {
                numberStr += ch;
            } 
            else if (!numberStr.empty()) 
            {
                break;
            }
        }
        
        if (numberStr.empty()) 
        {
            throw std::invalid_argument("No numeric value found in string: " + str);
        }

        return std::stoi(numberStr);
    }
};


struct MotorParams 
{
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

struct CANPortParams 
{
    int serial_id;
    int motor_num;    
    std::map<std::string, MotorParams, MotorNameComparator> motors;
};

struct CANBoardParams {
    int CANport_num;
    std::map<std::string, CANPortParams, MotorNameComparator> CANports;
};

struct RobotParams 
{
    int motor_timeout_ms;
    std::string robot_name;
    std::string Serial_Type;
    int Seial_baudrate;
    int CANboard_num;
    bool board_special_flag;
    bool canport_error_output_flag;
    std::map<std::string, CANBoardParams, MotorNameComparator> CANboards;
};

RobotParams parseRobotParams(const std::string& filePath);

#endif
