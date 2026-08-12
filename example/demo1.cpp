#include "canport.h"

#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>




std::atomic<bool> exit_flag(false);
void signal_handler(int signum) 
{
    exit_flag.store(true);
}

int main()
{
    std::signal(SIGINT, signal_handler);
    CanPort port1(1, {1, 2});
    // CanPort port2(2, {4, 5, 6});

    int num = 0;
    printf("%-4s %-6s %-9s %-9s %-9s\n", "id", "fault", "pos(rad)", "vel(rad/s)", "tqe(Nm)");
    while (!exit_flag.load())
    {
        for (auto it : port1.map_motors_state)
        {
            // printf("%-4d %-2d %-9.3f %-9.3f %-9.3f\n", it.first, it.second.fault, it.second.position, it.second.velocity, it.second.torque);
            printf("%-5.2f ", it.second.position);
        }
        printf("\n");


        port1.velocity(1, 0.314);
        port1.velocity(2, -0.314);
        port1.send();
        // if (++num > 10)
        // {
        //     port1.stop();
        //     port1.send();
        //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //     num = 0;
        //     port1.motor_zero_pos_reset();
        //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}