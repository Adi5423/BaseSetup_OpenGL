// Camera.cpp
// Basic FPS-style camera: position + yaw/pitch + movement helpers.
// Mouse rotates camera; keyboard moves using ProcessKeyboard or directly
// manipulating camera.Position (used in main).

#include "Camera.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(5.0f),
    MouseSensitivity(0.2f),
    Yaw(-90.0f),
    Pitch(0.0f),
    WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)) {
    Position = position;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

// Move camera along local axes (not used by main; main performs its own movement)
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD) Position += Front * velocity;
    if (direction == BACKWARD) Position -= Front * velocity;
    if (direction == LEFT) Position -= Right * velocity;
    if (direction == RIGHT) Position += Right * velocity;
}

// Mouse look - xoffset / yoffset are the raw pixel deltas.
// Yaw/Pitch updated; clamp pitch slightly to avoid gimbal flip.
void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // keep pitch in meaningful range to avoid flipping camera
    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;

    updateCameraVectors();
}

// Recompute orthonormal basis vectors from yaw/pitch
void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
