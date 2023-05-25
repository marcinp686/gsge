#include <iostream>
#include <vector>
#include <string_view>

#include "gsge.h"
#include "renderer/settings.h"

#include <easy/profiler.h>

#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{
    EASY_PROFILER_ENABLE;
    profiler::startListen();

    spdlog::set_level(spdlog::level::trace);

    Settings &settings = Settings::getInstance();
    settings.parseCmdParams(std::vector<std::string_view>{argv + 1, argv + argc});
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
