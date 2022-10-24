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

void camera::setPosition(glm::vec3 _position)
{
    position = _position;
    updateViewMatrix();
}

void camera::setCenter(glm::vec3 _center)
{
    center = _center;
    updateViewMatrix();
}

void camera::setUpVector(glm::vec3 _upVector)
{
    upVector = _upVector;
    updateViewMatrix();
}

void camera::setFov(float _fov)
{
    fov = _fov;
    updateProjMatrix();
}

void camera::setAspect(float _aspect)
{
    aspect = _aspect;
    updateProjMatrix();
}

void camera::setZNear(float _zNear)
{
    zNear = _zNear;
    updateProjMatrix();
}

void camera::setZFar(float _zFar)
{
    zFar = _zFar;
    updateProjMatrix();
}

void camera::updateViewMatrix()
{
    viewMatrix = glm::lookAt(position, center, upVector);
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
