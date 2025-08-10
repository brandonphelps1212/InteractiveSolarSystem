#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Improved default values for solar system viewing
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 8.0f;        // Slightly faster for large solar system
const float SENSITIVITY = 0.08f; // Lower sensitivity for smoother control
const float ZOOM = 45.0f;

class Camera
{
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;

    // Track mode for following objects
    bool trackingMode = false;
    glm::vec3 targetPosition = glm::vec3(0.0f);
    float trackingDistance = 15.0f;

    // Screen / projection params
    mutable float aspectRatio = 16.0f / 9.0f;
    mutable float nearPlane = 0.1f;
    mutable float farPlane = 1000.0f;

    Camera(glm::vec3 position);

    glm::mat4 GetViewMatrix();
    glm::mat4 getViewMatrix() const { return const_cast<Camera *>(this)->GetViewMatrix(); }

    // Projection (useful later for picking)
    glm::mat4 getProjectionMatrix() const;
    void setAspectRatio(float ratio) { aspectRatio = ratio; }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    void SetTrackingMode(bool enabled, glm::vec3 target = glm::vec3(0.0f));
    void UpdateTracking(float deltaTime);
    void ResetPosition(glm::vec3 position = glm::vec3(0.0f, 5.0f, 20.0f));

private:
    void updateCameraVectors();
};

#endif
