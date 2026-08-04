#ifndef _ROBOT_H
#define _ROBOT_H



#include "canport.h"
// #include "motor.h"

#include <memory>



class robot
{
public:
    robot();
    ~robot();
private:
    std::vector<std::unique_ptr<canport>> canports;
    // std::vector<motor> motors;
};




#endif