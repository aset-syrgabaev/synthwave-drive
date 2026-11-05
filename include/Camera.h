#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    float Yaw;
    float Pitch;

    Camera(glm::vec3 position = glm::vec3(0.0f, 2.0f, 5.0f)) 
        : Front(glm::vec3(0.0f, -0.2f, -1.0f)), Up(glm::vec3(0.0f, 1.0f, 0.0f)), Yaw(-90.0f), Pitch(-10.0f) {
        Position = position;
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(char direction, float deltaTime) {
        float velocity = 5.0f * deltaTime;
        if (direction == 'W') Position += Front * velocity;
        if (direction == 'S') Position -= Front * velocity;
        if (direction == 'A') Position -= glm::normalize(glm::cross(Front, Up)) * velocity;
        if (direction == 'D') Position += glm::normalize(glm::cross(Front, Up)) * velocity;
    }
};

#endif
