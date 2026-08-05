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
    canport port1(1, {1, 2, 3});
    canport port2(2, {4, 5, 6});

    while (!exitFlag.load())
    {
        for (auto it : port1.map_motors_state)
        {
            printf("motor[%02d] pos=%.2f, vel=%.2f, tqe=%.2f\n", it.first, it.second.position, it.second.velocity, it.second.torque);
        }
        


        port1.velocity(1, 0.314);
        port1.velocity(2, 0.314);
        port1.send();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}