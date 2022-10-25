#pragma once

#include <string>

#include <GLFW/glfw3.h>
#include "settings.h"

class Window
{
  public:
    Window();
    ~Window();

    uint32_t width = 800;
    uint32_t height = 600;

    void createWindow();
    void setTitle(const char *title);
    bool framebufferResized(); // did framebuffer dimensions change
    void setFullScreenMode();
    void setWindowedMode();

    GLFWwindow *get_handle() const;

    graphicsSettings settings;

  private:
    GLFWwindow *window = nullptr;
};
