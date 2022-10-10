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

    void setPosition(glm::vec3 _position);
    void setCenter(glm::vec3 _center);
    void setUpVector(glm::vec3 _upVector);
    void setFov(float _fov);
    void setAspect(float _aspect);
    void setZNear(float _zNear);
    void setZFar(float _zFar);

  private:
    // Data to calculate view matrix
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 center{0.0f, 0.0f, 0.0f};
    glm::vec3 upVector{0.0f, -1.0f, 0.0f};

    // Data to calculate projection matrix
    float fov{75};
    float aspect{1920.f / 1080.f};
    float zNear{0.1};
    float zFar{100};

    glm::mat4 projMatrix{glm::identity<glm::mat4>()};
    glm::mat4 viewMatrix{glm::identity<glm::mat4>()};
    glm::mat4 pvMatrix{glm::identity<glm::mat4>()};

    void updateViewMatrix();
    void updateProjMatrix();
    void updatePVMatrix();
};
