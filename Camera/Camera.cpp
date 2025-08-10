#include "Camera.h"
#include <algorithm>

Camera::Camera(glm::vec3 position)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM)
{
    Position = position;
    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    Yaw = YAW;
    Pitch = PITCH;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    if (trackingMode)
        return glm::lookAt(Position, targetPosition, Up);
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(Zoom), aspectRatio, nearPlane, farPlane);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;

    if (trackingMode)
    {
        glm::vec3 toTarget = targetPosition - Position;
        glm::vec3 right = glm::normalize(glm::cross(toTarget, WorldUp));
        glm::vec3 forward = glm::normalize(glm::cross(WorldUp, right));

        if (direction == FORWARD)
            Position += forward * velocity;
        if (direction == BACKWARD)
            Position -= forward * velocity;
        if (direction == LEFT)
            Position -= right * velocity;
        if (direction == RIGHT)
            Position += right * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;
    }
    else
    {
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::SetTrackingMode(bool enabled, glm::vec3 target)
{
    trackingMode = enabled;
    targetPosition = target;

    if (enabled)
    {
        glm::vec3 direction = glm::normalize(Position - target);
        Position = target + direction * trackingDistance;
    }
}

void Camera::UpdateTracking(float)
{
    if (trackingMode)
    {
        glm::vec3 toTarget = glm::normalize(targetPosition - Position);
        float distance = glm::length(targetPosition - Position);

        if (distance > trackingDistance * 1.2f || distance < trackingDistance * 0.8f)
            Position = targetPosition - toTarget * trackingDistance;
    }
}

void Camera::ResetPosition(glm::vec3 position)
{
    Position = position;
    Yaw = YAW;
    Pitch = PITCH;
    trackingMode = false;
    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
