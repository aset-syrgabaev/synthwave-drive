#ifndef OBSTACLES_H
#define OBSTACLES_H

#include <vector>
#include <cmath>

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

struct Barrier {
    float x;
    float z;
    bool isLow;
};

class Obstacles {
private:
    unsigned int cubeVAO, cubeVBO;
    std::vector<Barrier> barriers;

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

    void drawBox(const Shader& shader, const glm::mat4& model, const glm::vec3& color, const glm::vec3& emissive, float shininess = 32.0f) {
        shader.setMat4("model", model);
        shader.setVec3("objectColor", color);
        shader.setVec3("emissiveColor", emissive);
        shader.setFloat("shininess", shininess);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

public:
    float hitFlashTimer;

    Obstacles() : hitFlashTimer(0.0f) {
        initCube();

        float lanes[3] = {-2.4f, 0.0f, 2.4f};
        for (int i = 0; i < 16; ++i) {
            float z = -25.0f - (float)i * 16.0f;
            float x = lanes[(i * 2 + 1) % 3];
            bool low = (i % 3 != 0);
            barriers.push_back({x, z, low});
        }
    }

    void update(float carX, float carY, float scrollDist, float dt, bool isHovering) {
        if (hitFlashTimer > 0.0f) {
            hitFlashTimer -= dt;
        }

        if (isHovering) return;

        float loopLength = 260.0f;
        for (const auto& b : barriers) {
            float z = fmod(b.z + scrollDist + 20.0f, loopLength);
            if (z < 0.0f) z += loopLength;
            z = z - loopLength + 20.0f;

            if (abs(z) < 1.3f && abs(carX - b.x) < 1.3f) {
                if (b.isLow) {
                    if (carY < 1.1f) {
                        hitFlashTimer = 0.4f;
                    }
                } else {
                    if (carY < 2.5f) {
                        hitFlashTimer = 0.4f;
                    }
                }
            }
        }
    }

    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float scrollDist) {
        float loopLength = 260.0f;
        float animTime = static_cast<float>(glfwGetTime());

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        glm::vec3 neonRed(1.0f, 0.05f, 0.2f);
        glm::vec3 darkMetal(0.12f, 0.10f, 0.15f);

        for (const auto& b : barriers) {
            float z = fmod(b.z + scrollDist + 20.0f, loopLength);
            if (z < 0.0f) z += loopLength;
            z = z - loopLength + 20.0f;

            glm::mat4 m(1.0f);
            m = glm::translate(m, glm::vec3(b.x, 0.0f, z));

            if (b.isLow) {
                glm::mat4 postL = glm::translate(m, glm::vec3(-0.95f, 0.45f, 0.0f));
                postL = glm::scale(postL, glm::vec3(0.18f, 0.9f, 0.18f));
                drawBox(shader, postL, darkMetal, glm::vec3(0.0f));

                glm::mat4 postR = glm::translate(m, glm::vec3( 0.95f, 0.45f, 0.0f));
                postR = glm::scale(postR, glm::vec3(0.18f, 0.9f, 0.18f));
                drawBox(shader, postR, darkMetal, glm::vec3(0.0f));

                glm::mat4 beam = glm::translate(m, glm::vec3(0.0f, 0.55f, 0.0f));
                beam = glm::scale(beam, glm::vec3(2.0f, 0.25f, 0.15f));
                float pulse = 0.7f + 0.3f * sin(animTime * 10.0f + b.z);
                drawBox(shader, beam, neonRed, neonRed * 2.5f * pulse, 64.0f);
            } else {
                glm::mat4 postL = glm::translate(m, glm::vec3(-1.1f, 1.25f, 0.0f));
                postL = glm::scale(postL, glm::vec3(0.2f, 2.5f, 0.2f));
                drawBox(shader, postL, darkMetal, glm::vec3(0.0f));

                glm::mat4 postR = glm::translate(m, glm::vec3( 1.1f, 1.25f, 0.0f));
                postR = glm::scale(postR, glm::vec3(0.2f, 2.5f, 0.2f));
                drawBox(shader, postR, darkMetal, glm::vec3(0.0f));

                glm::mat4 laser = glm::translate(m, glm::vec3(0.0f, 1.8f, 0.0f));
                laser = glm::scale(laser, glm::vec3(2.3f, 0.35f, 0.15f));
                float pulse = 0.8f + 0.2f * cos(animTime * 12.0f);
                drawBox(shader, laser, glm::vec3(1.0f, 0.8f, 0.0f), glm::vec3(3.0f, 2.0f, 0.0f) * pulse, 128.0f);
            }
        }
    }
};

#endif
