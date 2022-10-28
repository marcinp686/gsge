#include <iostream>

#include "gsge.h"
#include <easy/profiler.h>

int main()
{
    EASY_PROFILER_ENABLE;
    profiler::startListen();
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
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
