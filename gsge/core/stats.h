#pragma once

#include <limits>

#include "../timer.h"

class stats
{
  public:
    void update();

    float dt = 0.0f;

    size_t averageFrameCountNumber = 400;
    size_t averageFrameTimeCounter = 0;
    float totalFrameTimeCounter = 0.0f;

    float maxFps = 0.0f;
    float currentFps = 0.0f;
    float minFps = std::numeric_limits<float>::max();

  private:
    timer frameTime;
};