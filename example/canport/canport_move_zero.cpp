#include "canport.h"

#include <chrono>
#include <cstdio>
#include <thread>

int main()
{
    CanPort can_port(1, {1});

    can_port.motor_zero_pos_reset();

    while (1)
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
