#pragma once

#pragma warning(suppress : 4275 6285 26498 26451 26800)
#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

/**
 * \brief Simple class for a camera.
 */
class camera
{
  public:
    camera();

    glm::mat4 &getViewMatrix();
    glm::mat4 &getProjMatrix();
    glm::mat4 &getProjViewMatrix();
    glm::vec3 &getPosition();

    void setPosition(glm::vec3 newPosition);
    void setCenter(glm::vec3 newCenter);                 //!< Set point in space for the camera to look at
    void setUpVector(glm::vec3 newUpVector);             //!< Set up vector for the camera
    void setFov(float newFov);                           //!< Set field of view for the camera
    void setAspect(float newAspect);                     //!< Set aspect ratio for the camera
    void setZNear(float newZNear);                       //!< Set near clipping plane for the camera
    void setZFar(float newZFar);                         //!< Set far clipping plane for the camera
    void update(float dt, float mouseDx, float mouseDy); //!< Update camera position and orientation
    void strafeLeft(float dt);                           //!< Move camera to the left
    void strafeRight(float dt);                          //!< Move camera to the right
    void moveForward(float dt);                          //!< Move camera forward
    void moveBackward(float dt);                         //!< Move camera backward
    void moveUp(float dt);                               //!< Move camera up
    void moveDown(float dt);                             //!< Move camera down

  private:
    // Data to calculate view matrix
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 center{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};

    // Data to calculate projection matrix
    float fov{60.f};    //!< Field of view in degrees
    float aspect{1.0f}; //!< Aspect ratio of the camera (width/height)
    float zNear{0.1f};  //!< Near clipping plane
    float zFar{150.0f}; //!< Far clipping plane

    // rotation around x (pitch) and y (yaw) axis
    float pitch{0.0f};
    float yaw{0.0f};

    // camera movement
    const float speed{12.0f};
    float sensitivityX{0.1f};
    float sensitivityY{0.1f};

    // camera direction vectors
    glm::vec3 front{0.0f, 0.0f, 1.0f};

    // matrices necessary for rendering
    glm::mat4 projMatrix{glm::identity<glm::mat4>()};
    glm::mat4 viewMatrix{glm::identity<glm::mat4>()};
    glm::mat4 pvMatrix{glm::identity<glm::mat4>()};

    // methods for updating projection and view matrices
    void updateViewMatrix();
    void updateProjMatrix();
    void updatePVMatrix();
};
