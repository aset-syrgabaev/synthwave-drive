#ifndef SPEEDPARTICLES_H
#define SPEEDPARTICLES_H

#include <vector>
#include <cstdlib>

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

class SpeedParticles {
private:
    unsigned int VAO, VBO;
    int particleCount;
    std::vector<glm::vec3> particles;

public:
    SpeedParticles(int count = 120) : particleCount(count) {
        for (int i = 0; i < particleCount; ++i) {
            float x = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 16.0f;
            float y = ((float)(rand() % 1000) / 1000.0f) * 4.5f + 0.2f;
            float z = -((float)(rand() % 1000) / 1000.0f) * 60.0f;
            particles.push_back(glm::vec3(x, y, z));
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, particleCount * 2 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void updateAndDraw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float speed, float boostFactor, float dt) {
        if (boostFactor < 0.05f) return;

        std::vector<float> lineVerts;
        lineVerts.reserve(particleCount * 6);

        float lineLength = 1.2f + boostFactor * 2.8f;
        float moveZ = speed * 1.5f * dt;

        for (int i = 0; i < particleCount; ++i) {
            particles[i].z += moveZ;
            if (particles[i].z > 6.0f) {
                particles[i].z = -60.0f - ((float)(rand() % 100) / 100.0f) * 10.0f;
                particles[i].x = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 16.0f;
                particles[i].y = ((float)(rand() % 1000) / 1000.0f) * 4.5f + 0.2f;
            }

            lineVerts.push_back(particles[i].x);
            lineVerts.push_back(particles[i].y);
            lineVerts.push_back(particles[i].z);

            lineVerts.push_back(particles[i].x);
            lineVerts.push_back(particles[i].y);
            lineVerts.push_back(particles[i].z - lineLength);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, lineVerts.size() * sizeof(float), lineVerts.data());

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setVec3("color", glm::vec3(0.2f, 0.95f, 1.0f) * boostFactor);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, particleCount * 2);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
    }
};

#endif
