#ifndef TERRAIN_H
#define TERRAIN_H

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
#include "Shader.h"

class Terrain {
private:
    unsigned int VAO, VBO, EBO;
    int indexCount;

public:
    Terrain() {
        int gridX = 80;
        int gridZ = 120;
        float width = 160.0f;
        float length = 240.0f;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for (int z = 0; z <= gridZ; ++z) {
            float zPos = -length + (float)z * (length / (float)gridZ);
            float v = (float)z / (float)gridZ * 120.0f;

            for (int x = 0; x <= gridX; ++x) {
                float xPos = -width * 0.5f + (float)x * (width / (float)gridX);
                float u = (float)x / (float)gridX * 80.0f;

                vertices.push_back(xPos);
                vertices.push_back(0.0f);
                vertices.push_back(zPos);

                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);

                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        for (int z = 0; z < gridZ; ++z) {
            for (int x = 0; x < gridX; ++x) {
                unsigned int topLeft = z * (gridX + 1) + x;
                unsigned int topRight = topLeft + 1;
                unsigned int bottomLeft = (z + 1) * (gridX + 1) + x;
                unsigned int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        indexCount = static_cast<int>(indices.size());

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float scrollDist, const glm::vec3& camPos, const glm::vec3& carPos, bool headlightsOn, const glm::vec3& gridLineColor, const glm::vec3& roadColor) const {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setFloat("scrollOffset", scrollDist);
        shader.setVec3("camPos", camPos);
        shader.setVec3("carPos", carPos);
        shader.setBool("headlightsOn", headlightsOn);
        shader.setVec3("gridLineColor", gridLineColor);
        shader.setVec3("roadColor", roadColor);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

#endif
