#ifndef HIGHTORQUE_ROBOT_H
#define HIGHTORQUE_ROBOT_H



#include "canport.h"
// #include "motor.h"

#include <memory>



class Robot
{
public:
    Robot();
    ~Robot();
private:
    std::vector<std::unique_ptr<CanPort>> can_ports;
    // std::vector<motor> motors;
};




#endif