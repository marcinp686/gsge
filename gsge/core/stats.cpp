#include "stats.h"

void stats::update()
{
    frameNumber++;

    dt = frameTime.resetTimer();
    if (totalRunningTime.getTimeAsSeconds()<2) return;
    
    averageFrameTimeCounter++;
    totalFrameTimeCounter += dt;
    if (averageFrameTimeCounter == averageFrameCountNumber)
    {
        currentFps = 1.0f / (totalFrameTimeCounter / averageFrameCountNumber);
        
        if (currentFps > maxFps)
            maxFps = currentFps;
        if (currentFps < minFps)
            minFps = currentFps;

        SPDLOG_INFO("FPS {:.1f}\tMIN {:.1f}\tMAX {:.1f}", currentFps, minFps, maxFps);

        averageFrameTimeCounter = 0;
        totalFrameTimeCounter = 0;
        averageFrameCountNumber = 1 + static_cast<size_t>(currentFps);
    }
}
