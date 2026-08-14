#include "robot.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <thread>


static std::atomic<bool> exit_flag(false);

int main()
{
    std::signal(SIGINT, [](int) { exit_flag.store(true); });

    Robot robot;

    float pos = 0.314f;
    std::chrono::steady_clock::time_point tick = std::chrono::steady_clock::now();
    while (!exit_flag.load())
    {
        if (std::chrono::steady_clock::now() - tick >= std::chrono::milliseconds(1000))
        {
            tick = std::chrono::steady_clock::now();
            pos *= -1;
        }

        for (Motor& motor : robot.motors)
        {
            motor.pos_vel_acc(pos, 0.314f, 3.14f);
        }
        robot.send();

        for (Motor& motor : robot.motors)
        {
            const motor_state_t* state = motor.get_motor_state();
            std::printf("ID: %2d, mode: %2d, fluat: %2d, pos: %2.3f, vel: %2.3f, tor: %2.3f\n",
                        motor.get_id(), state->mode, state->fault,
                        state->position, state->velocity, state->torque);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    robot.stop();
    robot.send();
    return 0;
}
