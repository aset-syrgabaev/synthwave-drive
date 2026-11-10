#ifndef CAR_H
#define CAR_H

#include <vector>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#elif __has_include(<GL/glew.h>)
#include <GL/glew.h>
#elif __has_include(<glad/glad.h>)
#include <glad/glad.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

class Car {
private:
    unsigned int cubeVAO, cubeVBO;

    void initCube() {
        float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void drawPart(const Shader& shader, const glm::mat4& model, const glm::vec3& color, const glm::vec3& emissive = glm::vec3(0.0f), float shininess = 32.0f) {
        shader.setMat4("model", model);
        shader.setVec3("objectColor", color);
        shader.setVec3("emissiveColor", emissive);
        shader.setFloat("shininess", shininess);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

public:
    float posX = 0.0f;
    float steeringTilt = 0.0f;
    float wheelRotation = 0.0f;

    Car() {
        initCube();
    }

    void update(float targetSteer, float speed, float dt) {
        posX += targetSteer * 10.0f * dt;
        if (posX < -3.8f) posX = -3.8f;
        if (posX >  3.8f) posX =  3.8f;

        float targetTilt = -targetSteer * 14.0f;
        steeringTilt += (targetTilt - steeringTilt) * 10.0f * dt;
        wheelRotation += speed * 30.0f * dt;
    }

    void draw(const Shader& shader) {
        glm::mat4 root = glm::mat4(1.0f);
        root = glm::translate(root, glm::vec3(posX, 0.42f, 0.0f));
        root = glm::rotate(root, glm::radians(steeringTilt), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 body = glm::scale(root, glm::vec3(1.7f, 0.35f, 3.4f));
        drawPart(shader, body, glm::vec3(0.12f, 0.02f, 0.22f), glm::vec3(0.0f), 64.0f);

        glm::mat4 cabin = glm::translate(root, glm::vec3(0.0f, 0.32f, 0.1f));
        cabin = glm::scale(cabin, glm::vec3(1.2f, 0.32f, 1.7f));
        drawPart(shader, cabin, glm::vec3(0.05f, 0.1f, 0.2f), glm::vec3(0.0f, 0.15f, 0.25f), 128.0f);

        glm::mat4 spoiler = glm::translate(root, glm::vec3(0.0f, 0.40f, 1.55f));
        spoiler = glm::scale(spoiler, glm::vec3(1.8f, 0.06f, 0.35f));
        drawPart(shader, spoiler, glm::vec3(0.95f, 0.1f, 0.6f), glm::vec3(0.4f, 0.0f, 0.2f), 32.0f);

        glm::mat4 mountL = glm::translate(root, glm::vec3(-0.6f, 0.26f, 1.55f));
        mountL = glm::scale(mountL, glm::vec3(0.06f, 0.22f, 0.1f));
        drawPart(shader, mountL, glm::vec3(0.1f, 0.1f, 0.1f));
        glm::mat4 mountR = glm::translate(root, glm::vec3( 0.6f, 0.26f, 1.55f));
        mountR = glm::scale(mountR, glm::vec3(0.06f, 0.22f, 0.1f));
        drawPart(shader, mountR, glm::vec3(0.1f, 0.1f, 0.1f));

        glm::mat4 tailLight = glm::translate(root, glm::vec3(0.0f, 0.08f, 1.71f));
        tailLight = glm::scale(tailLight, glm::vec3(1.5f, 0.10f, 0.04f));
        drawPart(shader, tailLight, glm::vec3(1.0f, 0.05f, 0.2f), glm::vec3(1.8f, 0.1f, 0.3f));

        glm::mat4 headL = glm::translate(root, glm::vec3(-0.65f, 0.0f, -1.71f));
        headL = glm::scale(headL, glm::vec3(0.32f, 0.12f, 0.04f));
        drawPart(shader, headL, glm::vec3(0.0f, 1.0f, 0.9f), glm::vec3(0.0f, 2.0f, 1.8f));

        glm::mat4 headR = glm::translate(root, glm::vec3( 0.65f, 0.0f, -1.71f));
        headR = glm::scale(headR, glm::vec3(0.32f, 0.12f, 0.04f));
        drawPart(shader, headR, glm::vec3(0.0f, 1.0f, 0.9f), glm::vec3(0.0f, 2.0f, 1.8f));

        glm::vec3 wheelOffsets[4] = {
            glm::vec3(-0.90f, -0.15f,  1.0f),
            glm::vec3( 0.90f, -0.15f,  1.0f),
            glm::vec3(-0.90f, -0.15f, -1.0f),
            glm::vec3( 0.90f, -0.15f, -1.0f)
        };

        for (int i = 0; i < 4; ++i) {
            glm::mat4 w = glm::translate(root, wheelOffsets[i]);
            w = glm::rotate(w, glm::radians(wheelRotation), glm::vec3(1.0f, 0.0f, 0.0f));
            w = glm::scale(w, glm::vec3(0.24f, 0.50f, 0.50f));
            drawPart(shader, w, glm::vec3(0.08f, 0.08f, 0.12f), glm::vec3(0.0f, 0.2f, 0.3f), 16.0f);
        }
    }
};

#endif
