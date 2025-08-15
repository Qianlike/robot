#include <iostream>
#include "parse_robot_params.hpp"

RobotParams parseRobotParams(const std::string& filePath) {
    RobotParams params;
    YAML::Node config = YAML::LoadFile(filePath);

    if (config["robot"]) {
        YAML::Node robotNode = config["robot"];
        params.robot_name = robotNode["robot_name"].as<std::string>();
        params.Serial_Type = robotNode["Serial_Type"].as<std::string>();
        params.Seial_baudrate = robotNode["Seial_baudrate"].as<int>();
        params.control_type = robotNode["control_type"].as<int>();
        params.motor_timeout_ms = robotNode["motor_timeout_ms"].as<int>();
        params.CANboard_num = robotNode["CANboard_num"].as<int>();

        if (robotNode["CANboard"]) {
            YAML::Node CANboardNode = robotNode["CANboard"];
            for (YAML::const_iterator it = CANboardNode.begin(); it != CANboardNode.end(); ++it) {
                std::string boardName = it->first.as<std::string>();
                YAML::Node boardNode = it->second;
                CANBoardParams board;
                board.CANport_num = boardNode["CANport_num"].as<int>();

                if (boardNode["CANport"]) {
                    YAML::Node CANportNode = boardNode["CANport"];
                    for (YAML::const_iterator portIt = CANportNode.begin(); portIt != CANportNode.end(); ++portIt) {
                        std::string portName = portIt->first.as<std::string>();
                        YAML::Node portNode = portIt->second;
                        CANPortParams port;
                        port.serial_id = portNode["serial_id"].as<int>();
                        port.motor_num = portNode["motor_num"].as<int>();

                        if (portNode["motor"]) {
                            YAML::Node motorNode = portNode["motor"];
                            for (YAML::const_iterator motorIt = motorNode.begin(); motorIt != motorNode.end(); ++motorIt) {
                                std::string motorName = motorIt->first.as<std::string>();
                                YAML::Node motorData = motorIt->second;
                                MotorParams motor;
                                motor.type = motorData["type"].as<std::string>();
                                motor.id = motorData["id"].as<int>();
                                motor.name = motorData["name"].as<std::string>();
                                motor.num = motorData["num"].as<int>();
                                motor.pos_limit_enable = motorData["pos_limit_enable"].as<bool>();
                                motor.pos_upper = motorData["pos_upper"].as<double>();
                                motor.pos_lower = motorData["pos_lower"].as<double>();
                                motor.tor_limit_enable = motorData["tor_limit_enable"].as<bool>();
                                motor.tor_upper = motorData["tor_upper"].as<double>();
                                motor.tor_lower = motorData["tor_lower"].as<double>();
                                port.motors[motorName] = motor;
                            }
                        }
                        board.CANports[portName] = port;
                    }
                }
                params.CANboards[boardName] = board;
            }
        }
    }

    return params;
}