#include "Mouse.h"

Mouse::Mouse(std::shared_ptr<Window> &window) : window(window)
{
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(*window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwGetCursorPos(*window, &x, &y);
    oldX = x;
    oldY = y;
}

void Mouse::update()
{
    glfwGetCursorPos(*window, &x, &y);
    if (oldX != x)
    {
        dx = oldX - x;
        oldX = x;
    }
    else
        dx = 0;

    if (oldY != y)
    {
        dy = oldY - y;
        oldY = y;
    }
    else
        dy = 0;
}
