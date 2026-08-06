#include "canport.h"

#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>




std::atomic<bool> exitFlag(false);
void signalHandler(int signum) 
{
    exitFlag.store(true);
}

int main()
{
    std::signal(SIGINT, signalHandler);
    canport port1(1, {1, 2});
    // canport port2(2, {4, 5, 6});

    int num = 0;
    while (!exitFlag.load())
    {
        for (auto it : port1.map_motors_state)
        {
            printf("motor[%02d] fault=%d, pos=%.2f, vel=%.2f, tqe=%.2f\n", it.first, it.second.fault, it.second.position, it.second.velocity, it.second.torque);
        }
        


        // port1.velocity(1, 0.314);
        // port1.velocity(2, 0.314);
        // port1.send();
        // port1.request_motor_state();
        // port1.stop(1);
        // port1.brake(1);
        // port1.brake(2);
        // port1.send();

        // port1.check_motor_pos_reset();

        port1.velocity(1, 0.314);
        port1.velocity(2, -0.314);
        port1.send();
        if (++num > 10)
        {
            port1.stop();
            port1.send();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            num = 0;
            port1.check_motor_pos_reset();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}