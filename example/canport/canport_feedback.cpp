#include "canport.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <thread>

static std::atomic<bool> exit_flag(false);

int main()
{
    std::signal(SIGINT, [](int) { exit_flag.store(true); });

    CanPort can_port(1, {1});

    // 只查询并打印反馈，不下发任何电机控制指令。
    while (!exit_flag.load())
    {
        can_port.request_motor_state();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        for (const auto& item : can_port.map_motors_state)
        {
            const int id = item.first;
            const motor_state_t& motor = item.second;
            std::printf("ID: %2d, mode: %2d, fluat: %2d, pos: %2.3f, vel: %2.3f, tor: %2.3f\n",
                        id, motor.mode, motor.fault,
                        motor.position, motor.velocity, motor.torque);
        }
    }

    can_port.stop();
    can_port.send();
    return 0;
}
