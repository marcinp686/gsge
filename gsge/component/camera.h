#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

class camera
{
  public:
    camera();

    glm::mat4 &getViewMatrix();
    glm::mat4 &getProjMatrix();
    glm::mat4 &getProjViewMatrix();
    glm::vec3 &getPosition();

    void setPosition(glm::vec3 newPosition);
    void setCenter(glm::vec3 newCenter); /**< Set point in space for the camera to look at */
    void setUpVector(glm::vec3 newUpVector);
    void setFov(float newFov);
    void setAspect(float newAspect);
    void setZNear(float newZNear);
    void setZFar(float newZFar);
    void update(float dt, float mouseDx, float mouseDy);
    void strafeLeft(float dt);
    void strafeRight(float dt);
    void moveForward(float dt);
    void moveBackward(float dt);
    void moveUp(float dt);
    void moveDown(float dt);

  private:
    // Data to calculate view matrix
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 center{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, -1.0f, 0.0f};

    // Data to calculate projection matrix
    float fov{75.f};
    float aspect{1.0f};
    float zNear{0.1f};
    float zFar{150.0f};

    // rotation around x (pitch) and y (yaw) axis
    float pitch{0.0f};
    float yaw{0.0f};

    // camera movement
    const float speed{10.0f};
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
