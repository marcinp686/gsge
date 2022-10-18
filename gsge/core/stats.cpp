#include "stats.h"

void stats::update()
{
    dt = frameTime.resetTimer();
    averageFrameTimeCounter++;
    totalFrameTimeCounter += dt;
    if (averageFrameTimeCounter == averageFrameCountNumber)
    {
        currentFps = 1.0f / (totalFrameTimeCounter / averageFrameCountNumber);
        spdlog::info("{:.1f}", currentFps);
        if (currentFps > maxFps)
            maxFps = currentFps;
        if (currentFps < minFps)
            minFps = currentFps;

        averageFrameTimeCounter = 0;
        totalFrameTimeCounter = 0;
        averageFrameCountNumber = 1 + static_cast<size_t>(currentFps);
    }
}
