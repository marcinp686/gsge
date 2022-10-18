#pragma once

#include <string>

#include <GLFW/glfw3.h>
#include "settings.h"

class Window
{
  public:
    Window();
    ~Window();

    int width = 800;
    int height = 600;

    void createWindow();
    void setTitle(const char *title);
    bool framebufferResized(); // did framebuffer dimensions change

    GLFWwindow *getWindow();

    graphicsSettings settings;

  private:
    GLFWwindow *window = nullptr;
};
