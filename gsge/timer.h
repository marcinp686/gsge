#pragma once

#include <chrono>
#include <spdlog/spdlog.h>

class timer
{
  public:
    timer();
    float resetTimer();
    void printTimer();

  private:
    std::chrono::steady_clock::time_point lastTimer;
};
