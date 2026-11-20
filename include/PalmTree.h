#ifndef PALMTREE_H
#define PALMTREE_H

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

// Пальмы вдоль обочин (показываются в Las Vegas, Sahara и Jungle).
// Используют OpenGL Instancing: все 40 деревьев рисуются за один вызов glDrawArraysInstanced.
class PalmTree {
private:
    unsigned int VAO, VBO, instanceVBO;
    int vertexCount;
    int instanceCount;
    std::vector<glm::vec3> basePositions; // базовые координаты каждой пальмы до прокрутки

    // Строит меш одной пальмы (изогнутый ствол из 5 секций + 6 листьев наверху)
    void buildGeometry(std::vector<float>& vertices) {
        auto addQuad = [&](const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec3& color) {
            glm::vec3 n1 = glm::normalize(glm::cross(p2 - p1, p3 - p1));
            glm::vec3 n2 = glm::normalize(glm::cross(p4 - p3, p1 - p3));

            auto push = [&](const glm::vec3& p, const glm::vec3& n) {
                vertices.push_back(p.x); vertices.push_back(p.y); vertices.push_back(p.z);
                vertices.push_back(n.x); vertices.push_back(n.y); vertices.push_back(n.z);
                vertices.push_back(color.r); vertices.push_back(color.g); vertices.push_back(color.b);
            };

            push(p1, n1); push(p2, n1); push(p3, n1);
            push(p1, n2); push(p3, n2); push(p4, n2);
        };

        int trunkSegments = 5;
        float hStep = 0.9f;
        float rBase = 0.22f;
        float rTop = 0.12f;
        glm::vec3 trunkColor(0.22f, 0.05f, 0.35f);

        // Ствол слегка сужается и наклоняется к дороге с каждой секцией
        for (int i = 0; i < trunkSegments; ++i) {
            float y0 = (float)i * hStep;
            float y1 = (float)(i + 1) * hStep;
            float r0 = rBase - (rBase - rTop) * ((float)i / (float)trunkSegments);
            float r1 = rBase - (rBase - rTop) * ((float)(i + 1) * 1.0f / (float)trunkSegments);

            float lean0 = (float)(i * i) * 0.035f;
            float lean1 = (float)((i + 1) * (i + 1)) * 0.035f;

            for (int a = 0; a < 4; ++a) {
                float a0 = (float)a * 1.57079f;
                float a1 = (float)(a + 1) * 1.57079f;

                glm::vec3 p1(cos(a0) * r0 + lean0, y0, sin(a0) * r0);
                glm::vec3 p2(cos(a1) * r0 + lean0, y0, sin(a1) * r0);
                glm::vec3 p3(cos(a1) * r1 + lean1, y1, sin(a1) * r1);
                glm::vec3 p4(cos(a0) * r1 + lean1, y1, sin(a0) * r1);

                addQuad(p1, p2, p3, p4, trunkColor);
            }
        }

        // Крона: 6 длинных изогнутых листьев по кругу вокруг вершины ствола
        float topY = (float)trunkSegments * hStep;
        float topLean = (float)(trunkSegments * trunkSegments) * 0.035f;
        glm::vec3 crown(topLean, topY, 0.0f);
        glm::vec3 frondColor(0.0f, 0.85f, 0.78f);

        int leafCount = 6;
        for (int l = 0; l < leafCount; ++l) {
            float angle = (float)l * (6.28318f / (float)leafCount);
            glm::vec3 dir(cos(angle), 0.0f, sin(angle));
            glm::vec3 mid = crown + dir * 1.3f + glm::vec3(0.0f, 0.35f, 0.0f);
            glm::vec3 tip = crown + dir * 2.3f - glm::vec3(0.0f, 0.65f, 0.0f);

            glm::vec3 sideDir = glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f))) * 0.36f;

            glm::vec3 pBase = crown;
            glm::vec3 pMidL = mid - sideDir;
            glm::vec3 pMidR = mid + sideDir;
            glm::vec3 pTip = tip;

            addQuad(pBase, pMidL, pTip, pMidR, frondColor);
        }
    }

public:
    PalmTree() {
        std::vector<float> vertices;
        buildGeometry(vertices);
        vertexCount = static_cast<int>(vertices.size() / 9);

        // Расставляем по 20 пальм с левой (-7.8) и правой (+7.8) стороны трассы
        for (int z = 0; z < 20; ++z) {
            float zPos = -10.0f - (float)z * 14.0f;
            basePositions.push_back(glm::vec3(-7.8f, 0.0f, zPos));
            basePositions.push_back(glm::vec3( 7.8f, 0.0f, zPos));
        }
        instanceCount = static_cast<int>(basePositions.size());

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &instanceVBO);

        glBindVertexArray(VAO);

        // Основной буфер вершин (позиция, нормаль, цвет — всего 9 float на вершину)
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Буфер матриц инстансов: mat4 занимает 4 слота атрибутов (location 3..6),
        // glVertexAttribDivisor(3+i, 1) говорит OpenGL обновлять матрицу 1 раз на целое дерево
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);

        for (int i = 0; i < 4; ++i) {
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
            glEnableVertexAttribArray(3 + i);
            glVertexAttribDivisor(3 + i, 1);
        }

        glBindVertexArray(0);
    }

    // Обновляем зацикленные позиции всех пальм по scrollDist и рисуем их одним вызовом
    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float scrollDist,
              const glm::vec3& lightPos, const glm::vec3& lightColor, const glm::vec3& camPos) {
        float loopLength = 280.0f;
        std::vector<glm::mat4> modelMatrices;
        modelMatrices.reserve(instanceCount);

        for (int i = 0; i < instanceCount; ++i) {
            glm::vec3 pos = basePositions[i];
            // Зацикливаем Z-координату через остаток от деления (fmod) для бесконечной дороги
            float wrappedZ = std::fmod(pos.z + scrollDist + 268.0f, loopLength);
            if (wrappedZ < 0.0f) wrappedZ += loopLength;
            wrappedZ -= 268.0f;

            glm::mat4 m(1.0f);
            m = glm::translate(m, glm::vec3(pos.x, pos.y, wrappedZ));
            if (pos.x < 0.0f) {
                m = glm::rotate(m, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            float scale = 0.92f + 0.12f * sin((float)i * 1.7f);
            m = glm::scale(m, glm::vec3(scale));
            modelMatrices.push_back(m);
        }

        // Отправляем обновлённые матрицы на видеокарту без пересоздания буфера
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data());

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("viewPos", camPos);

        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instanceCount);
        glBindVertexArray(0);
    }
};

#endif
