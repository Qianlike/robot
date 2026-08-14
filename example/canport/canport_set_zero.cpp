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

    // 以受限速度和加速度持续下发零位目标，Ctrl+C 后停止电机。
    while (!exit_flag.load())
    {
        for (const auto& item : can_port.map_motors_state)
        {
            can_port.pos_vel_acc(item.first, 0.0f, 0.314f, 3.14f);
        }
        can_port.send();

        for (const auto& item : can_port.map_motors_state)
        {
            const int id = item.first;
            const motor_state_t& motor = item.second;
            std::printf("ID: %2d, mode: %2d, fluat: %2d, pos: %2.3f, vel: %2.3f, tor: %2.3f\n",
                        id, motor.mode, motor.fault,
                        motor.position, motor.velocity, motor.torque);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    can_port.stop();
    can_port.send();
    return 0;
}
