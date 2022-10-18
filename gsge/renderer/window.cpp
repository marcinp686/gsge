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

GLFWwindow *Window::getWindow()
{
    return window;
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
}
