#pragma once

#include <GLFW/glfw3.h>

#include "../renderer/window.h"

/**
 * \brief Simple mouse class.
 *
 */
class Mouse
{
  public:
    /**
     * \brief Default constructor
     *
     * \param window [in] Pointer to window class instance
     */
    Mouse(Window *window);

    /**
     * \brief Update mouse position and deltas
     *
     */
    void   update();

    double x{0};  ///< Current X mouse coordinate
    double y{0};  ///< Current Y mouse coordinate
    double dx{0}; ///< Delta of X mouse coordinate since previous call of update()
    double dy{0}; ///< Delta of Y mouse coordinate since previous call of update()

  private:
    Window *window; ///< Pointer to window class instance

    double  oldX{0}, oldY{0};
};
