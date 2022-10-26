#pragma once

#include <chrono>
#include <string>
#include <spdlog/spdlog.h>

class timer
{
  public:
    timer();
    timer(std::string name);

    float resetTimer();
    void printTimer();

  private:
    std::string name;
    std::chrono::steady_clock::time_point lastTimer;
};
