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
        port1.velocity(1, 0.314);
        port1.velocity(2, 0.314);
        port1.send();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}