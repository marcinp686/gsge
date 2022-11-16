#include <iostream>

#include "gsge.h"
#include <easy/profiler.h>

#include <spdlog/spdlog.h>

int main()
{
    EASY_PROFILER_ENABLE;
    profiler::startListen();
    
    spdlog::set_level(spdlog::level::trace);

    gsge app;

    try
    {
        app.init();
        app.mainLoop();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        app.cleanup();
        Sleep(5000);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
