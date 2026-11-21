#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include "Mesh.h"

// Процедурная генерация базовых 3D-примитивов (куб, цилиндр, сфера) прямо в коде.
// Из них собираются препятствия, поезда, колёса, детали машины и щит без внешних .obj файлов.
namespace Primitives {

// Записываем одну вершину (позицию p и нормаль n) в общий массив float
inline void vert(std::vector<float>& v, const glm::vec3& p, const glm::vec3& n) {
    v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
    v.push_back(n.x); v.push_back(n.y); v.push_back(n.z);
}

// Разбиваем четырёхугольник (a, b, c, d) на два треугольника с общей нормалью n
inline void quad(std::vector<float>& v, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d, const glm::vec3& n) {
    vert(v, a, n); vert(v, b, n); vert(v, c, n);
    vert(v, a, n); vert(v, c, n); vert(v, d, n);
}

// Добавляем в массив вершин 6 граней параллелепипеда с заданным центром и размерами
inline void appendBox(std::vector<float>& v, const glm::vec3& center, const glm::vec3& size) {
    glm::vec3 h = size * 0.5f;
    glm::vec3 p[8] = {
        center + glm::vec3(-h.x, -h.y, -h.z), center + glm::vec3( h.x, -h.y, -h.z),
        center + glm::vec3( h.x,  h.y, -h.z), center + glm::vec3(-h.x,  h.y, -h.z),
        center + glm::vec3(-h.x, -h.y,  h.z), center + glm::vec3( h.x, -h.y,  h.z),
        center + glm::vec3( h.x,  h.y,  h.z), center + glm::vec3(-h.x,  h.y,  h.z)
    };
    quad(v, p[0], p[1], p[2], p[3], glm::vec3(0, 0, -1)); // передняя грань
    quad(v, p[4], p[5], p[6], p[7], glm::vec3(0, 0, 1));  // задняя грань
    quad(v, p[0], p[3], p[7], p[4], glm::vec3(-1, 0, 0)); // левая грань
    quad(v, p[1], p[5], p[6], p[2], glm::vec3(1, 0, 0));  // правая грань
    quad(v, p[0], p[4], p[5], p[1], glm::vec3(0, -1, 0)); // нижняя грань
    quad(v, p[3], p[2], p[6], p[7], glm::vec3(0, 1, 0));  // верхняя грань
}

// Единичный куб 1x1x1 с центром в (0, 0, 0)
inline Mesh cube() {
    std::vector<float> v;
    appendBox(v, glm::vec3(0.0f), glm::vec3(1.0f));
    Mesh m;
    m.upload(v);
    return m;
}

// Цилиндр вдоль оси X с числом граней segments (нужен для колёс, труб, монет и осей)
inline Mesh cylinder(int segments) {
    std::vector<float> v;
    const float twoPi = 6.28318530718f;
    for (int i = 0; i < segments; ++i) {
        float a0 = twoPi * (float)i / (float)segments;
        float a1 = twoPi * (float)(i + 1) / (float)segments;
        float y0 = std::cos(a0), z0 = std::sin(a0);
        float y1 = std::cos(a1), z1 = std::sin(a1);
        glm::vec3 n0(0.0f, y0, z0), n1(0.0f, y1, z1);

        // Боковая поверхность цилиндра (два треугольника на сегмент)
        vert(v, glm::vec3(-0.5f, y0, z0), n0);
        vert(v, glm::vec3( 0.5f, y0, z0), n0);
        vert(v, glm::vec3( 0.5f, y1, z1), n1);
        vert(v, glm::vec3(-0.5f, y0, z0), n0);
        vert(v, glm::vec3( 0.5f, y1, z1), n1);
        vert(v, glm::vec3(-0.5f, y1, z1), n1);

        // Правая круглая крышка (+X)
        vert(v, glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1, 0, 0));
        vert(v, glm::vec3(0.5f, y0, z0), glm::vec3(1, 0, 0));
        vert(v, glm::vec3(0.5f, y1, z1), glm::vec3(1, 0, 0));

        // Левая круглая крышка (-X)
        vert(v, glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(-1, 0, 0));
        vert(v, glm::vec3(-0.5f, y1, z1), glm::vec3(-1, 0, 0));
        vert(v, glm::vec3(-0.5f, y0, z0), glm::vec3(-1, 0, 0));
    }
    Mesh m;
    m.upload(v);
    return m;
}

// Сфера по сферическим координатам (stacks — кольца по широте, slices — сектора по долготе).
// Используется для полупрозрачного щита, бонусных сфер и деталей двигателя.
inline Mesh sphere(int stacks, int slices) {
    std::vector<float> v;
    const float pi = 3.14159265359f;
    auto point = [](float phi, float theta) {
        return glm::vec3(std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta));
    };
    for (int i = 0; i < stacks; ++i) {
        float p0 = pi * (float)i / (float)stacks;
        float p1 = pi * (float)(i + 1) / (float)stacks;
        for (int j = 0; j < slices; ++j) {
            float t0 = 2.0f * pi * (float)j / (float)slices;
            float t1 = 2.0f * pi * (float)(j + 1) / (float)slices;
            glm::vec3 a = point(p0, t0), b = point(p0, t1), c = point(p1, t1), d = point(p1, t0);
            // Для единичной сферы в центре (0,0,0) нормаль совпадает с координатами точки
            vert(v, a, a); vert(v, b, b); vert(v, c, c);
            vert(v, a, a); vert(v, c, c); vert(v, d, d);
        }
    }
    Mesh m;
    m.upload(v);
    return m;
}

}

#endif
