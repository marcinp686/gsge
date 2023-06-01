#pragma once

#include <vector>
#include <string_view>

#include <enums.h>

#include <spdlog/spdlog.h>

#define GSGE_SETTINGS_INSTANCE_DECL Settings &settings = Settings::getInstance()

// Engine settings class
class Settings
{
  public:
    // Display related settings
    ESettings::DisplayMode displayMode{ESettings::DisplayMode::Windowed};

    struct DisplaySize
    {
        uint32_t width{800};
        uint32_t height{600};
    } displaySize;

    static Settings &getInstance();
    void parseCmdParams(std::vector<std::string_view> params);

    int monitorIndex{0};
    int monitorCount{0};    

    // Application related settings
    std::string appName{"GSGE"};

    // Vulkan debugger related settings
    struct Debugger
    {
        bool enableValidationLayers{true};
        bool debugInstanceCreation{false};
    } Debugger;
    

  private:
    // Private constructor to prevent instancing
    Settings();

    // Private copy constructor to prevent copying
    Settings(const Settings &) = delete;
    // Private assignment operator to prevent assignment
    Settings &operator=(const Settings &) = delete;
    // Private move constructor to prevent moving
    Settings(Settings &&) = delete;
    // Private move assignment operator to prevent moving
    Settings &operator=(Settings &&) = delete;
};