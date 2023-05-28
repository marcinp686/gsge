#pragma once

#include <string>

#include <GLFW/glfw3.h>
#include "settings.h"

class Window
{
  public:
    Window();
    ~Window();

    void createWindow();
    void setTitle(const char *title);
    bool framebufferResized(); // did framebuffer dimensions change
    void setFullScreenMode();
    void setWindowedMode();
    void getMonitors();

    Settings &settings = Settings::getInstance();

    operator GLFWwindow *()
    {
        return window;
    }

  private:
    GLFWwindow *window = nullptr;
    GLFWmonitor **monitors = nullptr;
};
