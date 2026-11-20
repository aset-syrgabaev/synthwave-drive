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

// Цветовая и материальная стилизация препятствий, поездов и эстакад под каждый из 8 биомов
struct BiomeObsStyle {
    glm::vec3 primaryMat;
    glm::vec3 secondaryMat;
    glm::vec3 accentGlow;
    glm::vec3 warnGlow;
};

// Отрисовщик всех 3D-объектов трассы: поездов, барьеров, туннелей, LRT, бонусов и дронов
class TrackRenderer {
private:
    // Подбирает цвета материалов и неона препятствий под текущий биом (0..7)
    static BiomeObsStyle biomeStyle(int era);

    // Общие 3D-примитивы (куб, цилиндр, сфера, рампа) и кэш объёмных 3D-букв слова
    Mesh cube;
    Mesh cylinder;
    Mesh sphere;
    Mesh rampMesh;
    std::map<char, Mesh> letters;

    static Mesh buildRampMesh();
    glm::vec3 palette(int i) const;
    const Mesh& letterMesh(char c);
    void box(const Shader& s, const glm::mat4& base, const glm::vec3& p, const glm::vec3& size, const glm::vec3& color,
             const glm::vec3& emissive = glm::vec3(0.0f), float shininess = 64.0f, float refl = 0.0f) const;

    // Отрисовка бонусов, монет, букв, колец полёта и трамплинов (реализация в TrackDrawPickups.cpp)
    void drawCoin(const Shader& s, const glm::vec3& pos, float time) const;
    void drawLetter(const Shader& s, const Entity& e, const glm::vec3& pos, float time);
    void drawMysteryBox(const Shader& s, const glm::vec3& pos, float time);
    void drawOrb(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const;
    void drawShieldOrb(const Shader& s, const glm::vec3& pos, float time) const;
    void drawTechToken(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const;
    void drawNitroOrb(const Shader& s, const glm::vec3& pos, float time) const;
    void drawSkyRing(const Shader& s, const glm::vec3& pos, float time) const;
    void drawSkyDrone(const Shader& s, const glm::vec3& pos, float time) const;
    void drawBoostPad(const Shader& s, const glm::vec3& pos, float time) const;
    void drawAkiraCapsule(const Shader& s, const glm::vec3& pos, float time) const;
    void drawJumpPad(const Shader& s, const glm::vec3& pos, float time) const;

    // Отрисовка препятствий, туннеля, поездов и эстакад ЛРТ (реализация в TrackDrawWorld.cpp)
    void drawBarrier(const Shader& s, const glm::vec3& pos, float time, int era) const;
    void drawMovingBarrier(const Shader& s, const glm::vec3& pos, float time) const;
    void drawSpinner(const Shader& s, const glm::vec3& pos, float time) const;
    void drawGate(const Shader& s, const glm::vec3& pos, float time, int era) const;
    void drawTunnel(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const;
    void drawLrtPlatform(const Shader& s, const Entity& e, const glm::vec3& pos, float time, int era) const;
    void drawLrtTrain(const Shader& s, const Entity& e, const glm::vec3& pos, float time, int era) const;
    void drawSkyCatwalk(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const;
    void drawLaserSweep(const Shader& s, const Entity& e, const glm::vec3& pos, float time, int era) const;
    void drawRamp(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const;
    void drawTrain(const Shader& s, const Entity& e, const glm::vec3& pos, float time, int era) const;

public:
    TrackRenderer();
    // Рисует полицейского дрона позади машины после бокового зацепа (Stumble)
    void drawPolicePursuit(const Shader& s, float carX, float carY, float stumbleTimer, float time) const;
    // Рисует мини-босса дрона-охотника с красным лазерным прицелом
    void drawHunterGunship(const Shader& s, float targetLaneX, float roadY, float activeTimer, float cyclePhase, float time) const;
    // Главный цикл: проходит по всем видимым объектам Track::entities и вызывает нужную draw-функцию
    void draw(const Shader& s, const Track& track, float distance, float time, int era = 0);
};

#endif
