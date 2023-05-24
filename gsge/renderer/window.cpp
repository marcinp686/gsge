#include "window.h"
#include <enums.h>

Window::Window()
{
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::createWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (settings.displayMode == ESettings::DisplayMode::Windowed)
    {
        window = glfwCreateWindow(settings.displaySize.width, settings.displaySize.height, "Vulkan", nullptr, nullptr);
        glfwSetWindowPos(window, 700, 100);
    }
    else
    {
        window =
            glfwCreateWindow(settings.displaySize.width, settings.displaySize.height, "Vulkan", glfwGetPrimaryMonitor(), nullptr);
    }
    // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Window::setTitle(const char *title)
{
    glfwSetWindowTitle(window, title);
}

bool Window::framebufferResized()
{
    int w, h;

    glfwGetFramebufferSize(window, &w, &h);
    while (w == 0 || h == 0)
    {
        glfwGetFramebufferSize(window, &w, &h);
        glfwWaitEvents();
    }

    if (w == settings.displaySize.width && h == settings.displaySize.height)
        return false;

    settings.displaySize.width = w;
    settings.displaySize.height = h;

    return true;
}

void Window::setFullScreenMode()
{
    glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, 1920, 1080, GLFW_DONT_CARE);
    settings.displayMode = ESettings::DisplayMode::FullScreen;
}

void Window::setWindowedMode()
{
    glfwSetWindowMonitor(window, NULL, 100, 100, settings.displaySize.width, settings.displaySize.height, GLFW_DONT_CARE);
    settings.displayMode = ESettings::DisplayMode::Windowed;
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
}
