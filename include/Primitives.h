#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include "Mesh.h"

namespace Primitives {

inline void vert(std::vector<float>& v, const glm::vec3& p, const glm::vec3& n) {
    v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
    v.push_back(n.x); v.push_back(n.y); v.push_back(n.z);
}

inline void quad(std::vector<float>& v, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d, const glm::vec3& n) {
    vert(v, a, n); vert(v, b, n); vert(v, c, n);
    vert(v, a, n); vert(v, c, n); vert(v, d, n);
}

inline void appendBox(std::vector<float>& v, const glm::vec3& center, const glm::vec3& size) {
    glm::vec3 h = size * 0.5f;
    glm::vec3 p[8] = {
        center + glm::vec3(-h.x, -h.y, -h.z), center + glm::vec3( h.x, -h.y, -h.z),
        center + glm::vec3( h.x,  h.y, -h.z), center + glm::vec3(-h.x,  h.y, -h.z),
        center + glm::vec3(-h.x, -h.y,  h.z), center + glm::vec3( h.x, -h.y,  h.z),
        center + glm::vec3( h.x,  h.y,  h.z), center + glm::vec3(-h.x,  h.y,  h.z)
    };
    quad(v, p[0], p[1], p[2], p[3], glm::vec3(0, 0, -1));
    quad(v, p[4], p[5], p[6], p[7], glm::vec3(0, 0, 1));
    quad(v, p[0], p[3], p[7], p[4], glm::vec3(-1, 0, 0));
    quad(v, p[1], p[5], p[6], p[2], glm::vec3(1, 0, 0));
    quad(v, p[0], p[4], p[5], p[1], glm::vec3(0, -1, 0));
    quad(v, p[3], p[2], p[6], p[7], glm::vec3(0, 1, 0));
}

inline Mesh cube() {
    std::vector<float> v;
    appendBox(v, glm::vec3(0.0f), glm::vec3(1.0f));
    Mesh m;
    m.upload(v);
    return m;
}

inline Mesh cylinder(int segments) {
    std::vector<float> v;
    const float twoPi = 6.28318530718f;
    for (int i = 0; i < segments; ++i) {
        float a0 = twoPi * (float)i / (float)segments;
        float a1 = twoPi * (float)(i + 1) / (float)segments;
        float y0 = std::cos(a0), z0 = std::sin(a0);
        float y1 = std::cos(a1), z1 = std::sin(a1);
        glm::vec3 n0(0.0f, y0, z0), n1(0.0f, y1, z1);

        vert(v, glm::vec3(-0.5f, y0, z0), n0);
        vert(v, glm::vec3( 0.5f, y0, z0), n0);
        vert(v, glm::vec3( 0.5f, y1, z1), n1);
        vert(v, glm::vec3(-0.5f, y0, z0), n0);
        vert(v, glm::vec3( 0.5f, y1, z1), n1);
        vert(v, glm::vec3(-0.5f, y1, z1), n1);

        vert(v, glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1, 0, 0));
        vert(v, glm::vec3(0.5f, y0, z0), glm::vec3(1, 0, 0));
        vert(v, glm::vec3(0.5f, y1, z1), glm::vec3(1, 0, 0));

        vert(v, glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(-1, 0, 0));
        vert(v, glm::vec3(-0.5f, y1, z1), glm::vec3(-1, 0, 0));
        vert(v, glm::vec3(-0.5f, y0, z0), glm::vec3(-1, 0, 0));
    }
    Mesh m;
    m.upload(v);
    return m;
}

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
