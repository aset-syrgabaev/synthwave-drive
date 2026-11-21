#ifndef SPEEDPARTICLES_H
#define SPEEDPARTICLES_H

#include <vector>
#include <cstdlib>
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

// Система частиц для погоды и эффекта скорости (рисуется примитивом GL_LINES).
// В каждой локации свои осадки: дождь в Night City / Neo-Tokyo, снег в Arctic,
// песок в Sahara, светлячки в Jungle, пыль на Марсе и звёздная пыль на орбите.
class SpeedParticles {
private:
    unsigned int VAO, VBO;
    int particleCount;
    std::vector<glm::vec3> offsets; // позиция каждой частицы относительно камеры
    std::vector<float> dropSpeed;   // индивидуальный множитель скорости падения

    // Случайное число от 0.0 до 1.0
    static float frand() {
        return static_cast<float>(rand() % 10000) / 10000.0f;
    }

public:
    SpeedParticles(int count = 440) : particleCount(count) {
        offsets.reserve(particleCount);
        dropSpeed.reserve(particleCount);
        // Распределяем частицы в объёме 32x30x52 метра перед камерой (падают сверху с неба)
        for (int i = 0; i < particleCount; ++i) {
            float x = (frand() - 0.5f) * 32.0f;
            float y = -4.0f + frand() * 30.0f;
            float z = -1.5f - frand() * 52.0f;
            offsets.push_back(glm::vec3(x, y, z));
            dropSpeed.push_back(0.82f + frand() * 0.40f);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // Выделяем динамический буфер под 2 вершины (6 float) на каждую частицу-отрезок
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, particleCount * 2 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    // Двигает все частицы за кадр dt, возвращает улетевшие за границу обратно и рисует их.
    // Внутри туннеля (inTunnel == true) дождь и снег отключаются, остаются только линии нитро.
    void updateAndDraw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                       const glm::vec3& camPos, float speed, float boostFactor, int era, bool inTunnel, float dt) {
        bool rainEra   = ((era == 0) || (era == 2)) && !inTunnel;
        bool snowEra   = (era == 3) && !inTunnel;
        bool spaceEra  = (era == 4) && !inTunnel;
        bool dustEra   = ((era == 5) || (era == 7)) && !inTunnel;
        bool jungleEra = (era == 6) && !inTunnel;
        bool showBoost = (boostFactor >= 0.08f);
        if (!rainEra && !snowEra && !spaceEra && !dustEra && !jungleEra && !showBoost) return;

        std::vector<float> lineVerts;
        lineVerts.reserve(particleCount * 6);

        // Скорость смещения и длина следа зависят от типа погоды и ускорения boostFactor
        float windX = rainEra ? (-2.2f * dt) : (dustEra ? (-6.5f * dt) : (snowEra ? (-1.4f * dt) : (jungleEra ? (0.8f * dt) : 0.0f)));
        float fallY = rainEra ? (-24.0f * dt) : (snowEra ? (-5.5f * dt) : (dustEra ? (-1.8f * dt) : (jungleEra ? (1.2f * dt) : 0.0f)));
        float moveZ = speed * (rainEra ? 0.65f : (snowEra ? 0.45f : (jungleEra ? 0.35f : (spaceEra ? 1.35f : 1.0f)))) * dt;

        float streakX = rainEra ? 0.07f : (dustEra ? 0.45f : (snowEra ? 0.04f : 0.02f));
        float streakY = rainEra ? (0.76f * (1.0f - boostFactor * 0.55f)) : (snowEra ? 0.14f : (jungleEra ? 0.12f : 0.04f));
        float streakZ = rainEra ? (0.18f + boostFactor * 2.2f)
                      : (snowEra || jungleEra ? (0.14f + boostFactor * 1.4f)
                      : (dustEra ? (0.35f + boostFactor * 1.8f)
                      : (spaceEra ? (0.65f + boostFactor * 2.6f) : (1.2f + boostFactor * 2.8f))));

        for (int i = 0; i < particleCount; ++i) {
            float spd = dropSpeed[i];
            offsets[i].x += windX * spd;
            offsets[i].y += fallY * spd;
            offsets[i].z += moveZ * spd;

            // Зацикливаем координаты внутри коробки вокруг камеры, чтобы осадки шли непрерывно
            if (offsets[i].y < -4.0f) {
                offsets[i].y += 30.0f;
                offsets[i].x = (frand() - 0.5f) * 32.0f;
                offsets[i].z = -1.5f - frand() * 52.0f;
            }
            if (offsets[i].y > 26.0f) {
                offsets[i].y -= 30.0f;
                offsets[i].x = (frand() - 0.5f) * 32.0f;
                offsets[i].z = -1.5f - frand() * 52.0f;
            }
            if (offsets[i].z > -1.2f) {
                offsets[i].z -= 52.0f;
                offsets[i].x = (frand() - 0.5f) * 32.0f;
            }
            if (offsets[i].x < -16.0f) offsets[i].x += 32.0f;
            if (offsets[i].x >  16.0f) offsets[i].x -= 32.0f;

            glm::vec3 p0 = camPos + offsets[i];
            glm::vec3 p1 = p0 + glm::vec3(streakX, streakY, -streakZ);

            lineVerts.push_back(p0.x); lineVerts.push_back(p0.y); lineVerts.push_back(p0.z);
            lineVerts.push_back(p1.x); lineVerts.push_back(p1.y); lineVerts.push_back(p1.z);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, lineVerts.size() * sizeof(float), lineVerts.data());

        // Аддитивное смешивание (GL_ONE) делает капли и светлячки слегка светящимися в темноте
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glm::vec3 col(0.24f, 0.44f, 0.60f);
        if (era == 0)      col = glm::vec3(0.24f, 0.44f, 0.60f) * (0.42f + 0.42f * boostFactor);
        else if (era == 2) col = glm::vec3(0.55f, 0.22f, 0.30f) * (0.42f + 0.42f * boostFactor);
        else if (era == 3) col = glm::vec3(0.70f, 0.90f, 1.00f) * (0.48f + 0.35f * boostFactor);
        else if (era == 4) col = glm::vec3(0.45f, 0.72f, 1.00f) * (0.50f + 0.45f * boostFactor);
        else if (era == 5) col = glm::vec3(0.85f, 0.62f, 0.26f) * (0.36f + 0.35f * boostFactor);
        else if (era == 6) col = glm::vec3(0.25f, 0.98f, 0.62f) * (0.58f + 0.35f * boostFactor);
        else if (era == 7) col = glm::vec3(0.88f, 0.38f, 0.20f) * (0.38f + 0.35f * boostFactor);
        else               col = glm::vec3(0.18f, 0.85f, 1.00f) * boostFactor;

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setVec3("color", col);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, particleCount * 2);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
    }
};

#endif
