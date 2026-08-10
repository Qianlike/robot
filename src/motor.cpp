#include "motor.h"



Motor::Motor(CanPort *_can_port, uint8_t _id) : can_port(_can_port), id(_id)
{

}


void Motor::position(float pos)
{
    can_port->position(id, pos);
}


void Motor::velocity(float vel)
{
    can_port->velocity(id, vel);
}


void Motor::torque(float tqe)
{
    can_port->torque(id, tqe);
}


void Motor::vel_acc(float vel, float acc)
{
    can_port->vel_acc(id, vel, acc);
}


void Motor::pos_vel_acc(float pos, float vel, float acc)
{
    can_port->pos_vel_acc(id, pos, vel, acc);
}


void Motor::pos_vel_MAXtqe(float pos, float vel, float max_tqe)
{
    can_port->pos_vel_MAXtqe(id, pos, vel, max_tqe);
}


void Motor::pos_vel_tqe_kp_kd(float pos, float vel, float tqe, float kp, float kd)
{
    can_port->pos_vel_tqe_kp_kd(id, pos, vel, tqe, kp, kd);
}


void Motor::stop()
{
    can_port->stop(id);
}


void Motor::brake()
{
    can_port->brake(id);
}


void Motor::reset()
{
    can_port->reset(id);
}


void Motor::request_motor_state()
{
    can_port->request_motor_state();
}


motor_state_t *Motor::get_motor_state()
{
    return can_port->get_motor_state(id);
}
