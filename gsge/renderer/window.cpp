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

    // Get list of available monitors - primary monitor is always first
    GLFWmonitor **monitors = glfwGetMonitors(&settings.monitorCount);

    // Get current video mode of the monitor to show the window on
    const GLFWvidmode *mode = glfwGetVideoMode(monitors[settings.monitorIndex]);

    if (settings.monitorCount == 0)
    {
        spdlog::error("No monitors found");
        exit(EXIT_FAILURE);
    }
    else
    {
        spdlog::info("Found {} monitors. Using monitor id={} - {}", settings.monitorCount, settings.monitorIndex,
                     glfwGetMonitorName(monitors[settings.monitorIndex]));
    }

    if (settings.displayMode == ESettings::DisplayMode::Windowed)
    {
        // TODO: Change window title to be stored in settings class
        window =
            glfwCreateWindow(settings.displaySize.width, settings.displaySize.height, settings.appName.c_str(), nullptr, nullptr);
        int monitorX, monitorY; // monitor position in screen coordinates
        glfwGetMonitorPos(monitors[settings.monitorIndex], &monitorX, &monitorY);
        // Center window on monitor
        glfwSetWindowPos(window, monitorX + (mode->width - settings.displaySize.width) / 2,
                         monitorY + (mode->height - settings.displaySize.height) / 2);
    }
    else
    {

        // TODO: Change window title to be stored in settings class
        // TODO: test to see if mode is fullscreen and store it in settings class
        settings.displaySize.width = mode->width;
        settings.displaySize.height = mode->height;
        window = glfwCreateWindow(settings.displaySize.width, settings.displaySize.height, settings.appName.c_str(),
                                  monitors[settings.monitorIndex], nullptr);
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
