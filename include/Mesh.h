#ifndef MESH_H
#define MESH_H

#include <vector>
#include "GLHeaders.h"
#include <glm/glm.hpp>
#include "Shader.h"

// ============================================================================
// МОДУЛЬ 3D-МЕША (Mesh.h) — отвечает за критерии R1 (VAO/VBO) и B4 (эффективность)
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   - VBO (Vertex Buffer Object) хранит массив вершин в памяти видеокарты (VRAM).
//   - VAO (Vertex Array Object) запоминает, как читать VBO: первые 3 float — позиция (location 0),
//     следующие 3 float — нормаль (location 1), шаг (stride) = 6 * sizeof(float).
//   - Флаг GL_STATIC_DRAW значит, что геометрия загружается на GPU ОДИН РАЗ при запуске (не каждый кадр!).
// ============================================================================
class Mesh {
public:
    unsigned int VAO = 0; // Vertex Array Object — запоминает формат атрибутов вершин
    unsigned int VBO = 0; // Vertex Buffer Object — сам массив вершин в памяти видеокарты
    int vertexCount = 0;  // общее количество вершин для glDrawArrays

    // Загружаем массив вершин на видеокарту один раз при запуске (GL_STATIC_DRAW)
    void upload(const std::vector<float>& data) {
        vertexCount = static_cast<int>(data.size() / 6);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        // Атрибут 0 (location = 0 в шейдере): позиция вершины (3 float, шаг 6 float)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Атрибут 1 (location = 1 в шейдере): нормаль поверхности для расчёта света
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    // Отрисовка всех треугольников детали
    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
};

// Вспомогательная функция отрисовки детали с настройкой материала в шейдере:
// model — матрица положения/поворота/масштаба, color — базовый цвет, emissive — самосвечение,
// shininess — резкость блика, reflectivity — сила отражения среды (0 = матовый, 0.9 = металл).
inline void drawPart(const Shader& shader, const Mesh& mesh, const glm::mat4& model, const glm::vec3& color,
                     const glm::vec3& emissive = glm::vec3(0.0f), float shininess = 32.0f, float reflectivity = 0.0f) {
    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    shader.setVec3("emissiveColor", emissive);
    shader.setFloat("shininess", shininess);
    shader.setFloat("reflectivity", reflectivity);
    shader.setFloat("shieldAlpha", 0.0f); // 0 значит обычная непрозрачная деталь
    mesh.draw();
}

#endif
