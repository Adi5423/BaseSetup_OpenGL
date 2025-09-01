#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;

    // --- Physics fields ---
    glm::vec3 Velocity = glm::vec3(0.0f); // units/sec
    bool Grounded = false;
    float Height = 1.8f;    // player height in world units
    float Radius = 0.25f;   // player radius for cylinder/AABB approx

    Camera(glm::vec3 position);
    glm::mat4 GetViewMatrix() const;
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset);

    // physics helpers
    void SetPosition(const glm::vec3& pos) { Position = pos; }
    glm::vec3 GetPosition() const { return Position; }

private:
    void updateCameraVectors();
};
