#include <iostream>
#include <vector>
#include <string_view>

#include "gsge.h"
#include "renderer/settings.h"

#include <easy/profiler.h>

#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{

#ifdef BUILD_WITH_EASY_PROFILER
    EASY_PROFILER_ENABLE;
    profiler::startListen();
#endif // BUILD_WITH_EASY_PROFILER  

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::trace);
#elif NDEBUG
	spdlog::set_level(spdlog::level::info);
#endif    

    GSGE_SETTINGS_INSTANCE_DECL;
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
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
