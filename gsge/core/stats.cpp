#include "stats.h"

void stats::update()
{
    dt = frameTime.resetTimer();
    averageFrameTimeCounter++;
    totalFrameTimeCounter += dt;
    if (averageFrameTimeCounter == averageFrameCountNumber)
    {
        currentFps = 1.0f / (totalFrameTimeCounter / averageFrameCountNumber);
        spdlog::info("FPS {:.1f}\tMIN {:.1f}\tMAX {:.1f}", currentFps, minFps, maxFps);
        if (currentFps > maxFps)
            maxFps = currentFps;
        if (currentFps < minFps)
            minFps = currentFps;

        averageFrameTimeCounter = 0;
        totalFrameTimeCounter = 0;
        averageFrameCountNumber = 1 + static_cast<size_t>(currentFps);
    }
}
