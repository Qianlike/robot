#include "robot.h"

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
    Robot robot;

    exit(1);
    int num = 0;
    while (!exit_flag.load())
    {
        

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}