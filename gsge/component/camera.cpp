#include "camera.h"

camera::camera()
{
    updateViewMatrix();
    updateProjMatrix();
}

glm::mat4 &camera::getViewMatrix()
{
    return viewMatrix;
}

glm::mat4 &camera::getProjMatrix()
{
    return projMatrix;
}

glm::mat4 &camera::getProjViewMatrix()
{
    return pvMatrix;
}

glm::vec3 &camera::getPosition()
{
    return position;
}

void camera::setPosition(glm::vec3 newPosition)
{
    position = newPosition;
    updateViewMatrix();
}

void camera::setCenter(glm::vec3 newCenter)
{
    center = newCenter;
    updateViewMatrix();
}

void camera::setUpVector(glm::vec3 newUpVector)
{
    up = newUpVector;
    updateViewMatrix();
}

void camera::setFov(float newFov)
{
    fov = newFov;
    updateProjMatrix();
}

void camera::setAspect(float newAspect)
{
    aspect = newAspect;
    updateProjMatrix();
}

void camera::setZNear(float newZNear)
{
    zNear = newZNear;
    updateProjMatrix();
}

void camera::setZFar(float newZFar)
{
    zFar = newZFar;
    updateProjMatrix();
}

void camera::update(float dt, float mouseDx, float mouseDy)
{
    yaw += (mouseDx * sensitivityX);
    pitch += (mouseDy * sensitivityY);

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction{};
    direction.x = -sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = -sin(glm::radians(pitch));
    direction.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);

    // spdlog::warn("P={:.1f}, Y={:.1f}, [{:.1f}, {:.1f}, {:.1f}] ", pitch, yaw, front.x, front.y, front.z);

    updateViewMatrix();
}

void camera::strafeLeft(float dt)
{
    position -= glm::normalize(glm::cross(front, up)) * speed * dt;
    updateViewMatrix();
}

void camera::strafeRight(float dt)
{
    position += glm::normalize(glm::cross(front, up)) * speed * dt;
    updateViewMatrix();
}

void camera::moveForward(float dt)
{
    position += front * speed * dt;
    updateViewMatrix();
}

void camera::moveBackward(float dt)
{
    position -= front * speed * dt;
    updateViewMatrix();
}

void camera::moveUp(float dt)
{
    position += up * speed * dt;
	updateViewMatrix();
}

void camera::moveDown(float dt)
{
	position -= up * speed * dt;
	updateViewMatrix();
}

void camera::updateViewMatrix()
{
    viewMatrix = glm::lookAt(position, position + front, up);
    updatePVMatrix();
}

void camera::updateProjMatrix()
{
    projMatrix = glm::perspective(fov, aspect, zNear, zFar);
    updatePVMatrix();
}

void camera::updatePVMatrix()
{
    pvMatrix = projMatrix * viewMatrix;
}
