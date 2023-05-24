#pragma once

#include <vector>
#include <string_view>
#include <enums.h>

// A singleton class to engine settings
class Settings
{
  public:
    // Display related settings
    ESettings::DisplayMode displayMode{ESettings::DisplayMode::Windowed};

    struct DisplaySize
    {
        size_t width{800};
        size_t height{600};
    } displaySize;

  public:
    static Settings &getInstance();

  private:
    //static Settings instance;
    const std::vector<std::string_view> cmdParams;

    Settings();
    Settings(const Settings &) = delete;
    Settings &operator=(const Settings &) = delete;
    Settings(Settings &&) = delete;
    Settings &operator=(Settings &&) = delete;
};