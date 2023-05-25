#include "settings.h"

Settings::Settings()
{
}

Settings &Settings::getInstance()
{
    static Settings instance;
    return instance;
}

void Settings::parseCmdParams(std::vector<std::string_view> params)
{
    for (auto &param : params)
    {
        if (param.length() < 5)
        {
            spdlog::warn("Invalid parameter: {}", param);
            continue;
        }

        if (param.find("--monitor=") != param.npos)
        {
            param.remove_prefix(std::min(param.find_last_of('=') + 1, param.size()));
            try
            {
                monitorIndex = std::stoi(param.data());
                spdlog::info("Command line parameter detected - Monitor: {}", monitorIndex);
            }
            catch (const std::invalid_argument &e)
            {
                spdlog::warn("Invalid value for --monitor parameter: {}", param);
            }
        }
        else if (param.find("--fullscreen") != param.npos)
        {
            displayMode = ESettings::DisplayMode::FullScreen;
            spdlog::info("Command line parameter detected - Fullscreen mode");
        }
        else if (param.find("--windowed") != param.npos)
        {
            displayMode = ESettings::DisplayMode::Windowed;
            spdlog::info("Command line parameter detected - Windowed mode");
        }
        else if (param.find("--width=") != param.npos)
        {
            param.remove_prefix(std::min(param.find_last_of('=') + 1, param.size()));
            try
            {
                displaySize.width = std::stoi(param.data());
                spdlog::info("Command line parameter detected - Disply width: {}", displaySize.width);
            }
            catch (const std::invalid_argument &e)
            {
                spdlog::warn("Invalid value for --width parameter: {}", param);
            }
        }
        else if (param.find("--height=") != param.npos)
        {
            param.remove_prefix(std::min(param.find_last_of('=') + 1, param.size()));
            try
            {
                displaySize.height = std::stoi(param.data());
                spdlog::info("Command line parameter detected - Display height: {}", displaySize.height);
            }
            catch (const std::invalid_argument &e)
            {
                spdlog::warn("Invalid value for --height parameter: {}", param);
            }
        }
        else
        {
            spdlog::warn("Invalid parameter: {}", param);
            continue;
        }
    }
}
