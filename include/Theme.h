#ifndef THEME_H
#define THEME_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Theme {
    std::string name;
    glm::vec3 accent;
    glm::vec3 glow;
    glm::vec3 gridLine;
    glm::vec3 road;
    glm::vec3 mountainBase;
    glm::vec3 mountainPeak;
    glm::vec3 skyTop;
    glm::vec3 skyMid;
    glm::vec3 skyHorizon;
};

inline std::vector<Theme> makeThemes() {
    return {
        {
            "SYNTHWAVE",
            glm::vec3(0.95f, 0.10f, 0.65f), glm::vec3(0.10f, 0.75f, 1.00f),
            glm::vec3(0.95f, 0.15f, 0.65f), glm::vec3(0.04f, 0.01f, 0.09f),
            glm::vec3(0.12f, 0.02f, 0.24f), glm::vec3(0.95f, 0.12f, 0.65f),
            glm::vec3(0.04f, 0.00f, 0.14f), glm::vec3(0.38f, 0.02f, 0.48f), glm::vec3(0.95f, 0.25f, 0.55f)
        },
        {
            "CYBERPUNK",
            glm::vec3(1.00f, 0.85f, 0.05f), glm::vec3(1.00f, 0.55f, 0.10f),
            glm::vec3(0.10f, 0.95f, 0.85f), glm::vec3(0.02f, 0.03f, 0.06f),
            glm::vec3(0.05f, 0.08f, 0.15f), glm::vec3(0.10f, 0.95f, 0.80f),
            glm::vec3(0.01f, 0.02f, 0.08f), glm::vec3(0.15f, 0.18f, 0.35f), glm::vec3(1.00f, 0.75f, 0.15f)
        },
        {
            "VAPORWAVE",
            glm::vec3(0.30f, 0.90f, 0.95f), glm::vec3(0.95f, 0.35f, 0.85f),
            glm::vec3(0.40f, 0.85f, 0.95f), glm::vec3(0.08f, 0.02f, 0.15f),
            glm::vec3(0.20f, 0.05f, 0.30f), glm::vec3(0.95f, 0.45f, 0.70f),
            glm::vec3(0.08f, 0.02f, 0.18f), glm::vec3(0.45f, 0.15f, 0.48f), glm::vec3(0.98f, 0.55f, 0.65f)
        }
    };
}

#endif
