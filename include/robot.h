#ifndef HIGHTORQUE_ROBOT_H
#define HIGHTORQUE_ROBOT_H



#include "canport.h"
#include "motor.h"

#include <memory>



class Robot
{
public:
    Robot();
    ~Robot();

    
    
    void request_motor_state();
    void check_motor_pos_reset();
    void send();

    void position(uint8_t can_port_id, uint8_t id, float pos);
    void velocity(uint8_t can_port_id, uint8_t id, float vel);
    void torque(uint8_t can_port_id, uint8_t id, float tqe);
    void vel_acc(uint8_t can_port_id, uint8_t id, float vel, float acc);
    void pos_vel_acc(uint8_t can_port_id, uint8_t id, float pos, float vel, float acc);
    void pos_vel_MAXtqe(uint8_t can_port_id, uint8_t id, float pos, float vel, float max_tqe);
    void pos_vel_tqe_kp_kd(uint8_t can_port_id, uint8_t id, float pos, float vel, float tqe, float kp, float kd);

    void stop();
    void stop(uint8_t can_port_id, uint8_t id);

    void brake();
    void brake(uint8_t can_port_id, uint8_t id);

    void reset();
    void reset(uint8_t can_port_id, uint8_t id);

    motor_state_t *get_motor_state(uint8_t can_port_id, uint8_t id);
    fdcan_state_s *get_can_port_state(uint8_t can_port_id);

    std::vector<std::unique_ptr<CanPort>> can_ports;
    std::vector<Motor> motors;

private:
    CanPort *get_can_port(uint8_t can_port_id);
};



#endif
