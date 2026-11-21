#ifndef MOUNTAIN_H
#define MOUNTAIN_H

#include <vector>
#include <cmath>
#include <algorithm>

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

// Высокодетализированный процедурный рельеф гор и песчаных дюн по бокам трассы.
// Генерирует две отдельные сетки высокого разрешения (38 x 120):
// 1) скалистые горные хребты (для Night City, Vegas, Neo-Tokyo, Arctic, Jungle, Mars),
// 2) плавные выдутые ветром песчаные горы и барханы (специально для Сахары, era == 5).
class Mountain {
private:
    unsigned int rockVAO = 0, rockVBO = 0;
    unsigned int duneVAO = 0, duneVBO = 0;
    int rockVertexCount = 0;
    int duneVertexCount = 0;

    // Многооктавный рельеф настоящих горных хребтов с искажением координат (domain warping)
    static float ridgeHeight(float xDist, float z, float side) {
        float u = std::clamp(xDist / 115.0f, 0.0f, 1.0f);
        float envelope = std::pow(std::max(std::sin(u * 3.1415926f), 0.0f), 1.15f);

        // Искажаем координаты мягкими волнами, чтобы хребты не выглядели регулярной синусоидой
        float wx = xDist + 11.0f * std::sin(z * 0.028f + side * 1.1f);
        float wz = z + 9.5f * std::cos(xDist * 0.036f - side * 0.8f);

        float n1 = 1.0f - std::fabs(std::sin(wz * 0.024f + side * 1.4f) * std::cos(wx * 0.032f + 0.5f));
        float n2 = 1.0f - std::fabs(std::sin(wz * 0.058f - wx * 0.048f + side * 2.3f));
        float n3 = 1.0f - std::fabs(std::cos(wz * 0.115f + wx * 0.095f + 1.2f));
        float n4 = 0.5f + 0.5f * std::sin(wz * 0.22f - wx * 0.18f);

        float h = n1 * n1 * 34.0f + n2 * n2 * 16.0f + n3 * 6.5f + n4 * 2.2f;
        return std::max(0.0f, h * envelope);
    }

    // Реалистичные песчаные горы и длинные S-образные барханы Сахары (без острых угловатых "пирамидок")
    static float duneHeight(float xDist, float z, float side) {
        float u = std::clamp(xDist / 120.0f, 0.0f, 1.0f);
        float envelope = std::pow(std::max(std::sin(u * 3.1415926f), 0.0f), 0.85f);

        // Изогнутые ветром гребни барханов
        float drift = z * 0.022f + std::sin(xDist * 0.038f + side * 1.3f) * 1.6f;
        float crest1 = 1.0f - std::fabs(std::sin(drift + xDist * 0.026f));
        // Мягкое сглаживание вершины бархана (sqrt), чтобы склон был плавным и натуральным
        crest1 = std::pow(std::max(0.0f, crest1), 1.45f);

        float drift2 = z * 0.046f - std::cos(xDist * 0.052f - side) * 1.2f;
        float crest2 = 0.5f + 0.5f * std::sin(drift2 + xDist * 0.045f);

        // Дальний массив песчаных гор за барханами
        float mesaMask = std::clamp((xDist - 38.0f) / 48.0f, 0.0f, 1.0f);
        float desertRange = (0.5f + 0.5f * std::sin(z * 0.019f + side * 2.0f) * std::cos(xDist * 0.029f)) * 26.0f * mesaMask;

        float h = (crest1 * 18.0f + crest2 * 6.5f + desertRange);
        return std::max(0.0f, h * envelope);
    }

    // Универсальный построитель сетки рельефа со сглаженными вершинными нормалями
    static void buildMesh(bool isDunes, unsigned int& outVAO, unsigned int& outVBO, int& outCount) {
        const int NX = 36;
        const int NZ = 116;
        const float startX = isDunes ? 26.0f : 56.0f; // барханы начинаются ближе к трассе для красивого вида пустыни
        const float spanX  = isDunes ? 120.0f : 115.0f;
        const float startZ = 28.0f;
        const float stepZ  = 2.6f;

        std::vector<float> vertices;
        vertices.reserve(2 * NX * NZ * 6 * 6);

        for (int side : {-1, 1}) {
            std::vector<glm::vec3> pos((NX + 1) * (NZ + 1));
            std::vector<glm::vec3> nrm((NX + 1) * (NZ + 1), glm::vec3(0.0f));

            for (int iz = 0; iz <= NZ; ++iz) {
                float z = startZ - (float)iz * stepZ;
                for (int ix = 0; ix <= NX; ++ix) {
                    float dx = ((float)ix / (float)NX) * spanX;
                    float x  = (float)side * (startX + dx);
                    float y  = isDunes ? duneHeight(dx, z, (float)side) : ridgeHeight(dx, z, (float)side);
                    pos[iz * (NX + 1) + ix] = glm::vec3(x, y, z);
                }
            }

            // Усредняем нормали соседних треугольников для гладкого освещения склонов
            for (int iz = 0; iz < NZ; ++iz) {
                for (int ix = 0; ix < NX; ++ix) {
                    int i00 = iz * (NX + 1) + ix;
                    int i10 = iz * (NX + 1) + (ix + 1);
                    int i01 = (iz + 1) * (NX + 1) + ix;
                    int i11 = (iz + 1) * (NX + 1) + (ix + 1);

                    glm::vec3 a = pos[i00], b = pos[i10], c = pos[i01], d = pos[i11];
                    glm::vec3 fn1 = glm::cross(b - a, c - a);
                    if (fn1.y < 0.0f) fn1 = -fn1;
                    glm::vec3 fn2 = glm::cross(d - b, c - b);
                    if (fn2.y < 0.0f) fn2 = -fn2;

                    nrm[i00] += fn1; nrm[i10] += fn1 + fn2; nrm[i01] += fn1 + fn2; nrm[i11] += fn2;
                }
            }
            for (auto& n : nrm) {
                float l = glm::length(n);
                n = (l > 1e-5f) ? (n / l) : glm::vec3(0.0f, 1.0f, 0.0f);
            }

            auto pushV = [&](int idx) {
                vertices.push_back(pos[idx].x); vertices.push_back(pos[idx].y); vertices.push_back(pos[idx].z);
                vertices.push_back(nrm[idx].x); vertices.push_back(nrm[idx].y); vertices.push_back(nrm[idx].z);
            };

            for (int iz = 0; iz < NZ; ++iz) {
                for (int ix = 0; ix < NX; ++ix) {
                    int i00 = iz * (NX + 1) + ix;
                    int i10 = iz * (NX + 1) + (ix + 1);
                    int i01 = (iz + 1) * (NX + 1) + ix;
                    int i11 = (iz + 1) * (NX + 1) + (ix + 1);

                    pushV(i00); pushV(i10); pushV(i01);
                    pushV(i10); pushV(i11); pushV(i01);
                }
            }
        }

        outCount = static_cast<int>(vertices.size() / 6);

        glGenVertexArrays(1, &outVAO);
        glGenBuffers(1, &outVBO);
        glBindVertexArray(outVAO);
        glBindBuffer(GL_ARRAY_BUFFER, outVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

public:
    Mountain() {
        buildMesh(false, rockVAO, rockVBO, rockVertexCount);
        buildMesh(true,  duneVAO, duneVBO, duneVertexCount);
    }

    // Отрисовка гор или песчаных дюн с учётом текущего биома (era)
    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& camPos, const glm::vec3& mountainBaseColor, const glm::vec3& mountainPeakColor,
              const glm::vec3& fogCol, int era = 0) const {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setVec3("viewPos", camPos);
        shader.setVec3("baseColor", mountainBaseColor);
        shader.setVec3("peakColor", mountainPeakColor);
        shader.setVec3("fogColor", fogCol);
        shader.setInt("era", era);

        bool useDunes = (era == 5);
        glBindVertexArray(useDunes ? duneVAO : rockVAO);
        glDrawArrays(GL_TRIANGLES, 0, useDunes ? duneVertexCount : rockVertexCount);
        glBindVertexArray(0);
    }
};

#endif
