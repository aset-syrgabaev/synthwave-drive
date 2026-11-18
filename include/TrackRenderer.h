#ifndef TRACKRENDERER_H
#define TRACKRENDERER_H

#include <map>
#include <cmath>
#include "GLHeaders.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Primitives.h"
#include "Font5x7.h"
#include "Track.h"

class TrackRenderer {
private:
    Mesh cube;
    Mesh cylinder;
    Mesh sphere;
    std::map<char, Mesh> letters;

    glm::vec3 palette(int i) const {
        const glm::vec3 p[5] = {
            glm::vec3(1.00f, 0.15f, 0.60f), glm::vec3(0.10f, 0.90f, 1.00f), glm::vec3(0.60f, 0.25f, 1.00f),
            glm::vec3(1.00f, 0.50f, 0.10f), glm::vec3(0.40f, 1.00f, 0.30f)
        };
        return p[((i % 5) + 5) % 5];
    }

    const Mesh& letterMesh(char c) {
        auto it = letters.find(c);
        if (it != letters.end()) return it->second;
        std::vector<float> v;
        const char* g = findGlyph(c);
        const float px = 0.15f;
        if (g) {
            for (int r = 0; r < 7; ++r)
                for (int col = 0; col < 5; ++col)
                    if (glyphPixel(g, col, r))
                        Primitives::appendBox(v, glm::vec3(((float)col - 2.0f) * px, (3.0f - (float)r) * px, 0.0f), glm::vec3(px * 0.95f, px * 0.95f, 0.26f));
        }
        Mesh m;
        m.upload(v);
        return letters.emplace(c, m).first->second;
    }

    void box(const Shader& s, const glm::mat4& base, const glm::vec3& p, const glm::vec3& size, const glm::vec3& color,
             const glm::vec3& emissive = glm::vec3(0.0f), float shininess = 32.0f, float refl = 0.0f) const {
        drawPart(s, cube, glm::scale(glm::translate(base, p), size), color, emissive, shininess, refl);
    }

    void drawCoin(const Shader& s, const glm::vec3& pos, float time) const {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.12f * std::sin(time * 6.0f + pos.x), 0.0f));
        m = glm::rotate(m, time * 4.0f + 1.5708f, glm::vec3(0, 1, 0));
        drawPart(s, cylinder, glm::scale(m, glm::vec3(0.10f, 0.48f, 0.48f)), glm::vec3(1.0f, 0.78f, 0.10f), glm::vec3(1.3f, 0.9f, 0.1f), 120.0f, 0.3f);
        drawPart(s, cylinder, glm::scale(m, glm::vec3(0.13f, 0.32f, 0.32f)), glm::vec3(1.0f, 0.92f, 0.45f), glm::vec3(1.9f, 1.5f, 0.4f), 120.0f, 0.3f);
    }

    void drawLetter(const Shader& s, const Entity& e, const glm::vec3& pos, float time) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.18f * std::sin(time * 4.0f), 0.0f));
        m = glm::rotate(m, time * 2.2f, glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(1.25f));
        box(s, m, glm::vec3(0.0f), glm::vec3(0.95f, 1.30f, 0.08f), glm::vec3(0.05f, 0.02f, 0.14f), glm::vec3(0.0f, 0.2f, 0.4f), 32.0f, 0.0f);
        static const char word[] = "SYNTH";
        drawPart(s, letterMesh(word[e.index % 5]), m, glm::vec3(1.0f, 0.3f, 0.8f), glm::vec3(2.8f, 0.4f, 1.9f), 64.0f, 0.0f);
    }

    void drawOrb(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const {
        bool fly = e.type == EntityType::FlightOrb;
        glm::vec3 col = fly ? glm::vec3(0.1f, 0.9f, 1.0f) : glm::vec3(1.0f, 0.2f, 0.8f);
        glm::vec3 glow = col * 3.2f;
        glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.22f * std::sin(time * 5.0f), 0.0f));
        m = glm::rotate(m, time * 2.0f, glm::vec3(0, 1, 0));
        drawPart(s, sphere, glm::scale(m, glm::vec3(0.55f)), col, glow, 128.0f, 0.4f);
        if (fly) {
            for (float sx : {-1.0f, 1.0f}) {
                glm::mat4 w = glm::rotate(glm::translate(m, glm::vec3(sx * 0.75f, 0.1f, 0.0f)), glm::radians(sx * 25.0f), glm::vec3(0, 0, 1));
                drawPart(s, cube, glm::scale(w, glm::vec3(0.8f, 0.07f, 0.35f)), glm::vec3(1.0f, 0.9f, 0.3f), glm::vec3(2.2f, 1.8f, 0.3f), 64.0f, 0.0f);
            }
        } else {
            for (float sx : {-1.0f, 1.0f}) {
                glm::mat4 w = glm::translate(m, glm::vec3(sx * 0.62f, -0.15f, 0.0f));
                drawPart(s, cube, glm::scale(w, glm::vec3(0.22f, 0.5f, 0.22f)), sx < 0 ? glm::vec3(1.0f, 0.1f, 0.1f) : glm::vec3(0.2f, 0.4f, 1.0f),
                         sx < 0 ? glm::vec3(2.5f, 0.1f, 0.1f) : glm::vec3(0.3f, 0.6f, 2.5f), 64.0f, 0.0f);
            }
        }
    }

    void drawBarrier(const Shader& s, const glm::vec3& pos, float time) const {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, 0.0f, pos.z));
        float pulse = 0.75f + 0.25f * std::sin(time * 9.0f + pos.z);
        glm::vec3 dark(0.10f, 0.08f, 0.14f);
        box(s, m, glm::vec3(0.0f, 0.42f, 0.0f), glm::vec3(2.0f, 0.55f, 0.5f), dark, glm::vec3(0.0f), 48.0f, 0.2f);
        box(s, m, glm::vec3(-0.92f, 0.45f, 0.0f), glm::vec3(0.16f, 0.9f, 0.4f), dark);
        box(s, m, glm::vec3(0.92f, 0.45f, 0.0f), glm::vec3(0.16f, 0.9f, 0.4f), dark);
        box(s, m, glm::vec3(0.0f, 0.80f, 0.0f), glm::vec3(2.0f, 0.12f, 0.54f), glm::vec3(1.0f, 0.1f, 0.2f), glm::vec3(3.0f, 0.2f, 0.4f) * pulse);
        for (int i = 0; i < 5; ++i) {
            glm::mat4 st = glm::rotate(glm::translate(m, glm::vec3(-0.8f + 0.4f * (float)i, 0.42f, 0.27f)), 0.7854f, glm::vec3(0, 0, 1));
            drawPart(s, cube, glm::scale(st, glm::vec3(0.09f, 0.5f, 0.03f)), glm::vec3(1.0f, 0.85f, 0.1f), glm::vec3(2.0f, 1.5f, 0.1f), 32.0f, 0.0f);
        }
    }

    void drawTruck(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, 0.0f, pos.z));
        glm::vec3 neon = palette(e.index);
        glm::vec3 shell = glm::vec3(0.12f, 0.10f, 0.18f) + neon * 0.10f;
        box(s, m, glm::vec3(0.0f, 1.35f, 0.3f), glm::vec3(1.95f, 2.1f, 3.0f), shell, glm::vec3(0.0f), 64.0f, 0.3f);
        box(s, m, glm::vec3(0.0f, 0.95f, -1.75f), glm::vec3(1.85f, 1.3f, 1.2f), shell * 1.3f, glm::vec3(0.0f), 64.0f, 0.3f);
        box(s, m, glm::vec3(0.0f, 1.25f, -2.36f), glm::vec3(1.6f, 0.45f, 0.05f), glm::vec3(0.02f, 0.05f, 0.1f), glm::vec3(0.0f, 0.2f, 0.4f), 128.0f, 0.8f);
        for (float sx : {-1.0f, 1.0f}) {
            box(s, m, glm::vec3(sx * 0.99f, 1.6f, 0.3f), glm::vec3(0.04f, 0.16f, 2.8f), neon, neon * 3.0f);
            box(s, m, glm::vec3(sx * 0.99f, 0.8f, 0.3f), glm::vec3(0.04f, 0.10f, 2.8f), neon, neon * 2.0f);
            box(s, m, glm::vec3(sx * 0.7f, 0.35f, 1.85f), glm::vec3(0.35f, 0.14f, 0.05f), glm::vec3(0.8f, 0.05f, 0.05f), glm::vec3(3.0f, 0.1f, 0.1f));
            box(s, m, glm::vec3(sx * 0.6f, 0.35f, -2.38f), glm::vec3(0.3f, 0.12f, 0.05f), glm::vec3(0.9f), glm::vec3(3.0f, 3.0f, 2.6f));
            for (float wz : {-1.7f, 0.5f, 1.3f}) {
                glm::mat4 w = glm::translate(m, glm::vec3(sx * 0.95f, 0.38f, wz));
                drawPart(s, cylinder, glm::scale(w, glm::vec3(0.30f, 0.76f, 0.76f)), glm::vec3(0.02f), glm::vec3(0.0f), 8.0f, 0.0f);
            }
        }
        box(s, m, glm::vec3(0.0f, 2.45f, 0.3f), glm::vec3(1.9f, 0.06f, 2.9f), neon, neon * (1.6f + 0.6f * std::sin(time * 5.0f + (float)e.index)));
    }

    void drawGate(const Shader& s, const glm::vec3& pos, float time) const {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, pos.z));
        glm::vec3 dark(0.10f, 0.08f, 0.14f);
        for (float sx : {-1.0f, 1.0f}) box(s, m, glm::vec3(sx * 4.2f, 1.8f, 0.0f), glm::vec3(0.40f, 3.6f, 0.45f), dark, glm::vec3(0.0f), 48.0f, 0.2f);
        box(s, m, glm::vec3(0.0f, 2.05f, 0.0f), glm::vec3(8.8f, 0.6f, 0.45f), dark, glm::vec3(0.0f), 48.0f, 0.2f);
        for (int i = 0; i < 9; ++i) {
            bool red = (i % 2) == 0;
            float pulse = 0.7f + 0.3f * std::sin(time * 8.0f + (float)i);
            glm::vec3 c = red ? glm::vec3(1.0f, 0.1f, 0.15f) : glm::vec3(1.0f, 0.85f, 0.1f);
            box(s, m, glm::vec3(-3.8f + 0.95f * (float)i, 2.05f, 0.24f), glm::vec3(0.55f, 0.46f, 0.06f), c, c * 2.6f * pulse);
        }
    }

public:
    TrackRenderer() : cube(Primitives::cube()), cylinder(Primitives::cylinder(24)), sphere(Primitives::sphere(14, 20)) {}

    void draw(const Shader& s, const Track& track, float distance, float time) {
        for (const Entity& e : track.entities) {
            if (!e.alive) continue;
            float dz = e.dist - distance;
            if (dz > 200.0f || dz < -12.0f) continue;
            glm::vec3 pos(e.x, e.y, -dz);
            switch (e.type) {
            case EntityType::Coin: drawCoin(s, pos, time); break;
            case EntityType::Letter: drawLetter(s, e, pos, time); break;
            case EntityType::FlightOrb:
            case EntityType::MagnetOrb: drawOrb(s, e, pos, time); break;
            case EntityType::Barrier: drawBarrier(s, pos, time); break;
            case EntityType::Truck: drawTruck(s, e, pos, time); break;
            case EntityType::Gate: drawGate(s, pos, time); break;
            }
        }
    }
};

#endif
