#ifndef HIGHTORQUE_MOTOR_H
#define HIGHTORQUE_MOTOR_H

#include "canport.h"

class Motor
{
public:
    Motor(CanPort *_can_port, uint8_t _id);

    void position(float pos);
    void velocity(float vel);
    void torque(float tqe);
    void vel_acc(float vel, float acc);
    void pos_vel_acc(float pos, float vel, float acc);
    void pos_vel_MAXtqe(float pos, float vel, float max_tqe);
    void pos_vel_tqe_kp_kd(float pos, float vel, float tqe, float kp, float kd);

    void stop();
    void brake();
    void reset();

    void request_motor_state();

    motor_state_t *get_motor_state();

    uint8_t get_id() const { return id; }

private:
    CanPort *can_port = nullptr;    // 所属 canport，非拥有
    uint8_t id = 0;                 // 电机在 canport 上的 id
};

#endif
