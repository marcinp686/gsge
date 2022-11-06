#include "window.h"

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
    if (settings.windowType == graphicsSettings::windowType::windowed)
    {
        window = glfwCreateWindow(settings.windowSize.width, settings.windowSize.height, "Vulkan", nullptr, nullptr);
        glfwSetWindowPos(window, 700, 100);
    }
    else
    {
        window =
            glfwCreateWindow(settings.windowSize.width, settings.windowSize.height, "Vulkan", glfwGetPrimaryMonitor(), nullptr);
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

    if (w == width && h == height)
        return false;

    width = w;
    height = h;

    return true;
}

void Window::setFullScreenMode()
{
    glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, 1920, 1080, GLFW_DONT_CARE);
    settings.windowType = graphicsSettings::windowType::fullScreen;
}

void Window::setWindowedMode()
{
    glfwSetWindowMonitor(window, NULL, 100, 100, settings.windowSize.width, settings.windowSize.height, GLFW_DONT_CARE);
    settings.windowType = graphicsSettings::windowType::windowed;
}

GLFWwindow *Window::get_handle() const
{
    return window;
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
}
