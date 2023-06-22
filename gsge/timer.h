#pragma once

#include <chrono>
#include <string>

#pragma warning(suppress : 4275 6285 26498 26451 26800)
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
