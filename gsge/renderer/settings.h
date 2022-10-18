#pragma once
class graphicsSettings
{
  public:
    enum class windowType
    {
        fullScreen,
        windowed
    } windowType{windowType::windowed};

    struct windowSize
    {
        size_t width = 800;
        size_t height = 600;
    } windowSize;
};
