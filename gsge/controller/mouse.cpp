#include "Mouse.h"

Mouse::Mouse(Window *window) : window(window)
{
    glfwSetInputMode(window->get_handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetCursorPos(window->get_handle(), &x, &y);
    oldX = x;
    oldY = y;
}

void Mouse::update()
{
    glfwGetCursorPos(window->get_handle(), &x, &y);
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
