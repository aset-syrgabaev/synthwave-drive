#ifndef MESH_H
#define MESH_H

#include <vector>
#include "GLHeaders.h"
#include <glm/glm.hpp>
#include "Shader.h"

class Mesh {
public:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    int vertexCount = 0;

    void upload(const std::vector<float>& data) {
        vertexCount = static_cast<int>(data.size() / 6);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
};

inline void drawPart(const Shader& shader, const Mesh& mesh, const glm::mat4& model, const glm::vec3& color,
                     const glm::vec3& emissive = glm::vec3(0.0f), float shininess = 32.0f, float reflectivity = 0.0f) {
    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setVec3("emissiveColor", emissive);
    shader.setFloat("shininess", shininess);
    shader.setFloat("reflectivity", reflectivity);
    mesh.draw();
}

#endif
