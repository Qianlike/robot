#include "robot.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <thread>

static std::atomic<bool> exit_flag(false);

int main()
{
    constexpr auto kControlPeriod = std::chrono::microseconds(1000);
    std::signal(SIGINT, [](int) { exit_flag.store(true); });

    Robot robot;

    auto next_tick = std::chrono::steady_clock::now();

    // 只查询并通过 robot.motors 打印反馈，不下发任何电机控制指令。
    while (!exit_flag.load())
    {
        for (Motor& motor : robot.motors)
        {
            motor.request_motor_state();
        }

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

        next_tick += kControlPeriod;
        std::this_thread::sleep_until(next_tick);
    }

    robot.stop();
    robot.send();
    return 0;
}
