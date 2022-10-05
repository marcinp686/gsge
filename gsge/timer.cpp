#include "timer.h"

timer::timer()
{
    lastTimer = std::chrono::steady_clock::now();
}

float timer::resetTimer()
{
    auto currentTimer = std::chrono::steady_clock::now();
    float delta = std::chrono::duration<float, std::chrono::seconds::period>(currentTimer - lastTimer).count();
    lastTimer = currentTimer;
    return delta;
}
