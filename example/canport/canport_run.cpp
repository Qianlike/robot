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

    float pos = 0.314f;
    std::chrono::steady_clock::time_point tick = std::chrono::steady_clock::now();
    while (!exit_flag.load())
    {
        if (std::chrono::steady_clock::now() - tick >= std::chrono::milliseconds(1000))
        {
            tick = std::chrono::steady_clock::now();
            pos *= -1;
        }

        for (const auto& item : can_port.map_motors_state)
        {
            const float id = item.first;
            can_port.pos_vel_acc(id, pos, 0.314f, 3.14f);
        }
        can_port.send();
        
        for (const auto& item : can_port.map_motors_state)
        {
            const int id = item.first;
            const motor_state_t& motor = item.second;
            std::printf("ID: %2d, mode: %2d, fluat: %2d, pos: %2.3f, vel: %2.3f, tor: %2.3f 11\n",
                        id, motor.mode, motor.fault,
                        motor.position, motor.velocity, motor.torque);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    can_port.stop();
    can_port.send();
    return 0;
}
