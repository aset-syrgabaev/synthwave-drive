#ifndef VEGASCITY_H
#define VEGASCITY_H

#include <vector>
#include <random>
#include <cmath>
#include "GLHeaders.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Primitives.h"

class VegasCity {
private:
    struct Slot {
        glm::vec3 pos;
        glm::vec3 size;
        glm::vec4 params;
    };

    static constexpr float kLoop = 280.0f;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int matrixVBO = 0;
    unsigned int paramVBO = 0;
    int vertexCount = 0;
    std::vector<Slot> slots;

public:
    VegasCity() {
        std::vector<float> v;
        Primitives::appendBox(v, glm::vec3(0.0f), glm::vec3(1.0f));
        vertexCount = static_cast<int>(v.size() / 6);

        std::mt19937 rng(77);
        auto rnd = [&](float a, float b) { return std::uniform_real_distribution<float>(a, b)(rng); };
        const glm::vec3 neon[5] = {
            glm::vec3(1.00f, 0.15f, 0.60f), glm::vec3(0.10f, 0.90f, 1.00f), glm::vec3(0.65f, 0.30f, 1.00f),
            glm::vec3(1.00f, 0.60f, 0.10f), glm::vec3(1.00f, 0.90f, 0.30f)
        };

        for (int side : {-1, 1}) {
            float z = -6.0f;
            int n = 0;
            while (z > -262.0f) {
                float frontage = rnd(9.0f, 14.0f);
                float depth = rnd(10.0f, 16.0f);
                float height = rnd(22.0f, 56.0f);
                float gap = rnd(4.0f, 8.0f);
                glm::vec3 c = neon[(n + (side > 0 ? 2 : 0)) % 5];
                float x = (float)side * (13.5f + depth * 0.5f);
                float zc = z - frontage * 0.5f;

                slots.push_back({glm::vec3(x, height * 0.5f, zc), glm::vec3(depth, height, frontage), glm::vec4(c, rnd(0.0f, 50.0f))});

                glm::vec3 c2 = neon[(n * 3 + 1) % 5];
                slots.push_back({glm::vec3(x, height + 1.8f, zc), glm::vec3(depth * 0.82f, 3.2f, frontage * 0.72f), glm::vec4(c2, -1.0f)});

                if (rnd(0.0f, 1.0f) < 0.8f) {
                    float signY = rnd(6.0f, 12.0f);
                    slots.push_back({glm::vec3((float)side * 13.0f, signY, zc), glm::vec3(0.5f, 2.2f, frontage * 0.88f), glm::vec4(neon[(n + 2) % 5], -1.0f)});
                    slots.push_back({glm::vec3((float)side * 13.0f, signY - 1.6f, zc), glm::vec3(0.35f, 0.9f, frontage * 0.60f), glm::vec4(glm::vec3(1.0f, 0.85f, 0.1f), -1.0f)});
                }

                z -= frontage + gap;
                ++n;
            }
        }

        float pyrX = -36.0f;
        float pyrZ = -135.0f;
        glm::vec3 gold(1.0f, 0.85f, 0.2f);
        glm::vec3 cyan(0.1f, 0.95f, 1.0f);
        slots.push_back({glm::vec3(pyrX, 5.0f, pyrZ), glm::vec3(44.0f, 10.0f, 44.0f), glm::vec4(gold, 12.0f)});
        slots.push_back({glm::vec3(pyrX, 15.0f, pyrZ), glm::vec3(34.0f, 10.0f, 34.0f), glm::vec4(gold, 22.0f)});
        slots.push_back({glm::vec3(pyrX, 25.0f, pyrZ), glm::vec3(24.0f, 10.0f, 24.0f), glm::vec4(gold, 32.0f)});
        slots.push_back({glm::vec3(pyrX, 35.0f, pyrZ), glm::vec3(14.0f, 10.0f, 14.0f), glm::vec4(gold, 42.0f)});
        slots.push_back({glm::vec3(pyrX, 42.0f, pyrZ), glm::vec3(6.0f, 4.0f, 6.0f), glm::vec4(gold, -1.0f)});
        slots.push_back({glm::vec3(pyrX, 130.0f, pyrZ), glm::vec3(2.8f, 180.0f, 2.8f), glm::vec4(cyan, -1.0f)});

        float towerX = 36.0f;
        float towerZ = -195.0f;
        glm::vec3 magenta(1.0f, 0.15f, 0.65f);
        slots.push_back({glm::vec3(towerX, 34.0f, towerZ), glm::vec3(9.0f, 68.0f, 9.0f), glm::vec4(magenta, 18.0f)});
        slots.push_back({glm::vec3(towerX, 72.0f, towerZ), glm::vec3(24.0f, 8.0f, 24.0f), glm::vec4(cyan, -1.0f)});
        slots.push_back({glm::vec3(towerX, 78.0f, towerZ), glm::vec3(18.0f, 6.0f, 18.0f), glm::vec4(gold, -1.0f)});
        slots.push_back({glm::vec3(towerX, 98.0f, towerZ), glm::vec3(1.6f, 36.0f, 1.6f), glm::vec4(magenta, -1.0f)});

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &matrixVBO);
        glGenBuffers(1, &paramVBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
        glBufferData(GL_ARRAY_BUFFER, slots.size() * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        for (int i = 0; i < 4; ++i) {
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
            glEnableVertexAttribArray(3 + i);
            glVertexAttribDivisor(3 + i, 1);
        }

        std::vector<glm::vec4> params;
        for (const Slot& s : slots) params.push_back(s.params);
        glBindBuffer(GL_ARRAY_BUFFER, paramVBO);
        glBufferData(GL_ARRAY_BUFFER, params.size() * sizeof(glm::vec4), params.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
        glEnableVertexAttribArray(7);
        glVertexAttribDivisor(7, 1);
        glBindVertexArray(0);
    }

    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float scroll, const glm::vec3& camPos,
              float time, const glm::vec3& fogColor) {
        std::vector<glm::mat4> mats;
        mats.reserve(slots.size());
        for (const Slot& s : slots) {
            float z = std::fmod(s.pos.z + scroll + 262.0f, kLoop);
            if (z < 0.0f) z += kLoop;
            z -= 262.0f;
            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(s.pos.x, s.pos.y, z));
            mats.push_back(glm::scale(m, s.size));
        }
        glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mats.size() * sizeof(glm::mat4), mats.data());

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("viewPos", camPos);
        shader.setFloat("time", time);
        shader.setVec3("fogColor", fogColor);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, static_cast<int>(slots.size()));
        glBindVertexArray(0);
    }
};

#endif
