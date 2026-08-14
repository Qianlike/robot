#include "robot.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <thread>

constexpr float kVelocity = 0.314f;  // rad/s
constexpr float kAcceleration = 3.14f; // rad/s^2

static std::atomic<bool> exit_flag(false);

int main()
{
    std::signal(SIGINT, [](int) { exit_flag.store(true); });

    Robot robot;

    // 通过 robot.motors 以受限速度和加速度持续下发零位目标。
    while (!exit_flag.load())
    {
        for (Motor& motor : robot.motors)
        {
            motor.pos_vel_acc(0.0f, kVelocity, kAcceleration);
        }
        robot.send();

        for (Motor& motor : robot.motors)
        {
            const motor_state_t* state = motor.get_motor_state();
            if (state != nullptr)
            {
                std::printf("ID: %2d, mode: %2d, fluat: %2d, pos: %2.3f, vel: %2.3f, tor: %2.3f\n",
                            motor.get_id(), state->mode, state->fault,
                            state->position, state->velocity, state->torque);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    robot.stop();
    robot.send();
    return 0;
}
