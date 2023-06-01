#include "settings.h"

Settings::Settings()
{
}

Settings &Settings::getInstance()
{
    static Settings instance;
    return instance;
}
// TODO: Add --help parameter to display all available parameters
void Settings::parseCmdParams(std::vector<std::string_view> params)
{
    for (auto &param : params)
    {
        if (param.length() < 5)
        {
            SPDLOG_WARN("[Settings] Invalid parameter: {}", param);
            continue;
        }

        if (param.find("--monitor=") != param.npos)
        {
            param.remove_prefix(std::min(param.find_last_of('=') + 1, param.size()));
            try
            {
                monitorIndex = std::stoi(param.data());
                SPDLOG_INFO("[Settings] Command line parameter detected - Monitor: {}", monitorIndex);
            }
            catch (const std::invalid_argument &e)
            {
                SPDLOG_WARN("[Settings] Invalid value for --monitor parameter: {}", param);
            }
        }
        else if (param.find("--fullscreen") != param.npos)
        {
            displayMode = ESettings::DisplayMode::FullScreen;
            SPDLOG_INFO("[Settings] Command line parameter detected - Fullscreen mode");
        }
        else if (param.find("--windowed") != param.npos)
        {
            displayMode = ESettings::DisplayMode::Windowed;
            SPDLOG_INFO("[Settings] Command line parameter detected - Windowed mode");
        }
        else if (param.find("--width=") != param.npos)
        {
            param.remove_prefix(std::min(param.find_last_of('=') + 1, param.size()));
            try
            {
                displaySize.width = std::stoi(param.data());
                SPDLOG_INFO("[Settings] Command line parameter detected - Disply width: {}", displaySize.width);
            }
            catch (const std::invalid_argument &e)
            {
                SPDLOG_WARN("[Settings] Invalid value for --width parameter: {}", param);
            }
        }
        else if (param.find("--height=") != param.npos)
        {
            param.remove_prefix(std::min(param.find_last_of('=') + 1, param.size()));
            try
            {
                displaySize.height = std::stoi(param.data());
                SPDLOG_INFO("[Settings] Command line parameter detected - Display height: {}", displaySize.height);
            }
            catch (const std::invalid_argument &e)
            {
                SPDLOG_WARN("[Settings] Invalid value for --height parameter: {}", param);
            }
        }
        else
        {
            SPDLOG_WARN("[Settings] Invalid parameter: {}", param);
            continue;
        }
    }
}
