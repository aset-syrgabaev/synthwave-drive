#ifndef SKY_H
#define SKY_H

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

// ============================================================================
// ПРОЦЕДУРНОЕ НЕБО И СОЛНЦЕ 80-Х (Sky.h) — ЯДРО ТЕМЫ #13 + КРИТЕРИЙ R5
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   - Рисует один прямоугольник (2 треугольника = 6 вершин) на весь экран в NDC (-1..+1).
//   - Перед рисованием отключаем glDisable(GL_DEPTH_TEST), чтобы небо легло ровно на задний фон,
//     а потом включаем обратно — вся 3D-сцена рисуется поверх него.
//   - Полосатое солнце (1985 Las Vegas), тучи, северное сияние и космос вычисляются в sky.frag.
// ============================================================================
class Sky {
private:
    unsigned int VAO, VBO;

public:
    // Создаём два треугольника, покрывающие весь экран от -1 до +1 в NDC
    Sky() {
        float quadVertices[] = {
            // позиция (x, y)   текстурные координаты (u, v)
            -1.0f,  1.0f,       0.0f, 1.0f,
            -1.0f, -1.0f,       0.0f, 0.0f,
             1.0f, -1.0f,       1.0f, 0.0f,

            -1.0f,  1.0f,       0.0f, 1.0f,
             1.0f, -1.0f,       1.0f, 0.0f,
             1.0f,  1.0f,       1.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Атрибут 0: экранные координаты
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Атрибут 1: UV-координаты от 0 до 1
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    // Рисуем фон с отключённым тестом глубины, чтобы все 3D-объекты сцены рисовались поверх неба
    void draw(const Shader& shader, float time, int era, const glm::vec2& resolution,
              const glm::vec3& topCol, const glm::vec3& midCol, const glm::vec3& horizonCol) const {
        glDisable(GL_DEPTH_TEST);
        shader.use();
        shader.setFloat("time", time);
        shader.setInt("era", era);
        shader.setVec2("resolution", resolution);
        shader.setVec3("skyTopColor", topCol);
        shader.setVec3("skyMidColor", midCol);
        shader.setVec3("skyHorizonColor", horizonCol);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }
};

#endif
