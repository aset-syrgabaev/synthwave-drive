#ifndef THEME_H
#define THEME_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

// Структура описывает цветовую палитру одной локации (небо, асфальт, неон и горы).
// Переключаются локации вручную кнопкой T или в меню настроек G.
struct Theme {
    std::string name;       // название мира для заголовка окна и меню
    glm::vec3 accent;       // основной акцентный цвет (вывески, элементы декора)
    glm::vec3 glow;         // цвет подсветки и свечения
    glm::vec3 gridLine;     // цвет линий сетки / краёв дороги
    glm::vec3 road;         // базовый цвет дорожного полотна (асфальта)
    glm::vec3 mountainBase; // цвет основания гор и дальнего тумана
    glm::vec3 mountainPeak; // цвет вершин гор
    glm::vec3 skyTop;       // верхняя точка градиента неба (зенит)
    glm::vec3 skyMid;       // середина неба
    glm::vec3 skyHorizon;   // цвет горизонта (используется также для основного света)
};

// Возвращает массив из 8 готовых миров. Стартуем всегда с 0 (тёмный дождливый киберпанк).
inline std::vector<Theme> makeThemes() {
    return {
        {
            // 0: 2077 NIGHT CITY — дождливый киберпанк в духе Blade Runner с неоновыми отражениями
            "2077 NIGHT CITY",
            glm::vec3(0.92f, 0.16f, 0.28f), glm::vec3(0.14f, 0.80f, 0.94f),
            glm::vec3(0.24f, 0.36f, 0.48f), glm::vec3(0.024f, 0.028f, 0.038f),
            glm::vec3(0.018f, 0.024f, 0.038f), glm::vec3(0.065f, 0.095f, 0.145f),
            glm::vec3(0.008f, 0.012f, 0.026f), glm::vec3(0.024f, 0.036f, 0.062f), glm::vec3(0.088f, 0.068f, 0.110f)
        },
        {
            // 1: 1985 LAS VEGAS — классический ретро-бульвар с фиолетовым закатом
            "1985 LAS VEGAS",
            glm::vec3(0.85f, 0.14f, 0.54f), glm::vec3(0.10f, 0.78f, 0.95f),
            glm::vec3(0.66f, 0.14f, 0.46f), glm::vec3(0.018f, 0.010f, 0.036f),
            glm::vec3(0.040f, 0.016f, 0.085f), glm::vec3(0.24f, 0.08f, 0.22f),
            glm::vec3(0.016f, 0.005f, 0.055f), glm::vec3(0.12f, 0.020f, 0.19f), glm::vec3(0.42f, 0.11f, 0.26f)
        },
        {
            // 2: NEO-TOKYO AKIRA — ночное шоссе Нео-Токио 2019 с красным городским заревом
            "NEO-TOKYO AKIRA",
            glm::vec3(0.96f, 0.14f, 0.18f), glm::vec3(0.98f, 0.42f, 0.14f),
            glm::vec3(0.55f, 0.16f, 0.22f), glm::vec3(0.026f, 0.022f, 0.030f),
            glm::vec3(0.028f, 0.018f, 0.036f), glm::vec3(0.20f, 0.065f, 0.090f),
            glm::vec3(0.010f, 0.008f, 0.024f), glm::vec3(0.046f, 0.022f, 0.056f), glm::vec3(0.18f, 0.056f, 0.072f)
        },
        {
            // 3: ARCTIC AURORA — ночная Арктика, ледники и северное сияние
            "ARCTIC AURORA",
            glm::vec3(0.14f, 0.92f, 0.74f), glm::vec3(0.22f, 0.88f, 1.00f),
            glm::vec3(0.14f, 0.56f, 0.72f), glm::vec3(0.016f, 0.024f, 0.040f),
            glm::vec3(0.030f, 0.058f, 0.100f), glm::vec3(0.42f, 0.70f, 0.84f),
            glm::vec3(0.006f, 0.014f, 0.032f), glm::vec3(0.018f, 0.055f, 0.100f), glm::vec3(0.055f, 0.145f, 0.195f)
        },
        {
            // 4: ORBITAL STATION — орбитальная станция в открытом космосе
            "ORBITAL STATION",
            glm::vec3(0.26f, 0.68f, 1.00f), glm::vec3(0.46f, 0.82f, 1.00f),
            glm::vec3(0.18f, 0.42f, 0.82f), glm::vec3(0.012f, 0.014f, 0.024f),
            glm::vec3(0.005f, 0.007f, 0.015f), glm::vec3(0.04f, 0.07f, 0.14f),
            glm::vec3(0.002f, 0.003f, 0.010f), glm::vec3(0.006f, 0.012f, 0.030f), glm::vec3(0.018f, 0.032f, 0.072f)
        },
        {
            // 5: SAHARA PYRAMIDS — ночная пустыня, пирамиды и закатный горизонт
            "SAHARA PYRAMIDS",
            glm::vec3(0.95f, 0.68f, 0.22f), glm::vec3(0.18f, 0.88f, 0.84f),
            glm::vec3(0.72f, 0.46f, 0.16f), glm::vec3(0.032f, 0.020f, 0.014f),
            glm::vec3(0.22f, 0.12f, 0.06f), glm::vec3(0.70f, 0.45f, 0.18f),
            glm::vec3(0.03f, 0.02f, 0.06f), glm::vec3(0.18f, 0.09f, 0.10f), glm::vec3(0.52f, 0.26f, 0.12f)
        },
        {
            // 6: NEON JUNGLE — тёмные джунгли с биолюминесценцией
            "NEON JUNGLE",
            glm::vec3(0.12f, 0.88f, 0.48f), glm::vec3(0.20f, 0.96f, 0.78f),
            glm::vec3(0.10f, 0.56f, 0.34f), glm::vec3(0.010f, 0.020f, 0.015f),
            glm::vec3(0.018f, 0.050f, 0.032f), glm::vec3(0.10f, 0.30f, 0.18f),
            glm::vec3(0.006f, 0.016f, 0.020f), glm::vec3(0.015f, 0.058f, 0.052f), glm::vec3(0.048f, 0.145f, 0.110f)
        },
        {
            // 7: MARS COLONY — марсианская база и пыльная буря
            "MARS COLONY",
            glm::vec3(0.96f, 0.38f, 0.18f), glm::vec3(0.32f, 0.84f, 1.00f),
            glm::vec3(0.66f, 0.24f, 0.14f), glm::vec3(0.034f, 0.014f, 0.012f),
            glm::vec3(0.20f, 0.065f, 0.045f), glm::vec3(0.62f, 0.22f, 0.12f),
            glm::vec3(0.035f, 0.012f, 0.018f), glm::vec3(0.16f, 0.055f, 0.045f), glm::vec3(0.42f, 0.15f, 0.10f)
        }
    };
}

#endif
