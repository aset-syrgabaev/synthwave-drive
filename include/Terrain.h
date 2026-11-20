#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
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

// Сетка дороги и земли: сгущённая разбивка по краям дороги позволяет шейдеру плавно вырезать глубокий туннель
class Terrain {
private:
    unsigned int VAO, VBO, EBO; // EBO (Element Buffer Object) экономит память за счёт переиспользования вершин
    int indexCount;

public:
    Terrain() {
        // Формируем неравномерную сетку по X: высокая плотность на краях дороги (±4.85..5.10) для вертикальных стен туннеля
        std::vector<float> xCoords;
        for (int i = 0; i <= 28; ++i) xCoords.push_back(-140.0f + (134.8f * (float)i / 28.0f));
        xCoords.push_back(-5.10f);
        xCoords.push_back(-4.85f);
        for (int i = 0; i <= 24; ++i) xCoords.push_back(-4.75f + (9.50f * (float)i / 24.0f));
        xCoords.push_back(4.85f);
        xCoords.push_back(5.10f);
        for (int i = 0; i <= 28; ++i) xCoords.push_back(5.20f + (134.8f * (float)i / 28.0f));

        int gridX = static_cast<int>(xCoords.size()) - 1;
        int gridZ = 240;
        float zMin = -265.0f;
        float zMax =  35.0f;
        float length = zMax - zMin;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Заполняем массив вершин: на каждую вершину кладём 8 float (позиция x,y,z + нормаль nx,ny,nz + UV u,v)
        for (int z = 0; z <= gridZ; ++z) {
            float zPos = zMin + (float)z * (length / (float)gridZ);
            float v = (zPos - zMin) * 0.5f;

            for (int x = 0; x <= gridX; ++x) {
                float xPos = xCoords[x];
                float u = xPos * 0.5f;

                vertices.push_back(xPos);
                vertices.push_back(0.0f);
                vertices.push_back(zPos);

                // Изначально дорога плоская, нормаль смотрит ровно вверх (0, 1, 0)
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);

                // Координаты UV, по которым в terrain.frag рисуется сетка fwidth()
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        // Собираем индексы треугольников (каждая клетка сетки состоит из 2 треугольников) для glDrawElements
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

        // Создаём VAO, VBO и EBO и один раз загружаем всю сетку дороги в видеопамять (GL_STATIC_DRAW)
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Объясняем OpenGL как читать 8 float из каждой вершины:
        // location 0 = позиция (3 float), location 1 = нормаль (3 float), location 2 = UV (2 float)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    // Передаём матрицы, смещение дороги scrollDist, номер биома, время и цвет фар скина в шейдер земли
    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float scrollDist,
              const glm::vec3& camPos, const glm::vec3& carPos, bool headlightsOn,
              const glm::vec3& gridLineColor, const glm::vec3& roadColor, int era = 0, float time = 0.0f,
              const glm::vec3& headlightColor = glm::vec3(0.95f, 0.90f, 0.78f)) const {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setFloat("scrollOffset", scrollDist); // сдвигает сетку и туннель навстречу камере
        shader.setVec3("camPos", camPos);
        shader.setVec3("carPos", carPos);
        shader.setBool("headlightsOn", headlightsOn);
        shader.setVec3("gridLineColor", gridLineColor);
        shader.setVec3("roadColor", roadColor);
        shader.setInt("era", era);
        shader.setFloat("time", time);
        shader.setVec3("headlightColor", headlightColor);

        // Рисуем всю дорогу одним вызовом через индексный буфер EBO
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

#endif
