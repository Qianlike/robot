#include "robot.h"

#include <chrono>
#include <cstdio>
#include <thread>

int main()
{
    Robot robot;

    robot.motor_zero_pos_reset();

    while (1)
    {
        robot.request_motor_state();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        for (Motor& motor : robot.motors)
        {
            const motor_state_t* state = motor.get_motor_state();
            std::printf("ID: %2d, mode: %2d, fluat: %2d, pos: %2.3f, vel: %2.3f, tor: %2.3f\n",
                        motor.get_id(), state->mode, state->fault,
                        state->position, state->velocity, state->torque);
        }
    }

    robot.stop();
    robot.send();
    return 0;
}
