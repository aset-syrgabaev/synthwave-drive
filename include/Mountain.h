#ifndef MOUNTAIN_H
#define MOUNTAIN_H

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
#include "Shader.h"

class Mountain {
private:
    unsigned int VAO, VBO;
    int vertexCount;

public:
    Mountain() {
        std::vector<float> vertices;
        // Generate low-poly mountain ridges along left (side = -1) and right (side = 1)
        for (int side : {-1, 1}) {
            float baseX = side * 15.0f;
            float width = side * 28.0f;

            for (int z = 0; z < 25; ++z) {
                float z0 = -z * 8.0f;
                float z1 = -(z + 1) * 8.0f;

                // Height peaks using sine harmonics
                float h0 = 4.0f + 5.5f * sin(z * 1.35f + side * 2.0f) + 3.0f * cos(z * 0.7f);
                float h1 = 4.0f + 5.5f * sin((z + 1) * 1.35f + side * 2.0f) + 3.0f * cos((z + 1) * 0.7f);

                // Triangle 1
                glm::vec3 p1(baseX, 0.0f, z0);
                glm::vec3 p2(baseX + width * 0.5f, h0, z0 - 4.0f);
                glm::vec3 p3(baseX, 0.0f, z1);
                glm::vec3 n1 = glm::normalize(glm::cross(p2 - p1, p3 - p1));

                auto addV = [&](const glm::vec3& p, const glm::vec3& n) {
                    vertices.push_back(p.x); vertices.push_back(p.y); vertices.push_back(p.z);
                    vertices.push_back(n.x); vertices.push_back(n.y); vertices.push_back(n.z);
                };

                addV(p1, n1); addV(p2, n1); addV(p3, n1);

                // Triangle 2
                glm::vec3 p4(baseX + width * 0.5f, h0, z0 - 4.0f);
                glm::vec3 p5(baseX + width * 0.5f, h1, z1 - 4.0f);
                glm::vec3 p6(baseX, 0.0f, z1);
                glm::vec3 n2 = glm::normalize(glm::cross(p5 - p4, p6 - p4));

                addV(p4, n2); addV(p5, n2); addV(p6, n2);
            }
        }

        vertexCount = static_cast<int>(vertices.size() / 6);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& camPos) const {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setVec3("viewPos", camPos);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
};

#endif
