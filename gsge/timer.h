#pragma once

#include <chrono>

class timer
{
  public:
    timer();
    float resetTimer();

  private:
    std::chrono::steady_clock::time_point lastTimer;
};
