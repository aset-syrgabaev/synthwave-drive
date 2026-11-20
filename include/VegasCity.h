#ifndef VEGASCITY_H
#define VEGASCITY_H

#include <vector>
#include <cmath>
#include <algorithm>
#include "GLHeaders.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Primitives.h"

// Движок 8 совершенно разных, гармоничных 3D-локаций (переключаются клавишей T или в настройках G):
// 0: 2077 NIGHT CITY  — депрессивный киберпанк-мегаполис с аркологиями и летающим трафиком
// 1: 1985 LAS VEGAS   — отели, фонтаны Bellagio, Luxor, High Roller, MSG Sphere и Эйфелева башня
// 2: NEO-TOKYO AKIRA  — гигантские красные вантовые мосты, токийские высотки и поезда-пули
// 3: ARCTIC AURORA    — кристаллические ледники, научные купола, вращающиеся радары и северное сияние
// 4: ORBITAL STATION  — космические арки Dyson-кольца, солнечные батареи, реакторы и шаттлы
// 5: SAHARA PYRAMIDS  — величественные золотые и обсидиановые пирамиды, обелиски и арки оазиса
// 6: NEON JUNGLE      — биолюминесцентные гигантские деревья, древние храмы и светящиеся лианы
// 7: MARS COLONY      — марсианские купола-теплицы, космодром с ракетами и шлюзовые мосты
// Генератор и отрисовщик окружения по бокам трассы для всех 8 локаций.
// Все здания, арки, фонари и электронные билборды рисуются одним вызовом через OpenGL Instancing.
class VegasCity {
private:
    struct Slot {
        glm::vec3 pos;
        glm::vec3 size;
        // rgb = цвет, a: >=0 фасад небоскрёба, -1 неон, -2 вывеска, -3 LED/теплица, -4 кристальный лёд, -5 титан, -6 камень/песчаник/кора
        glm::vec4 params;
        int animType;     // 0 статика, 1/3 колесо, 2 фонтаны, 4 сфера, 5 трафик, 6 вращение, 7 поезд/шаттл/дрон
        float animPhase;
        float rotZ;
    };

    static constexpr float kLoop = 280.0f;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int matrixVBO = 0;
    unsigned int paramVBO = 0;
    int vertexCount = 0;
    int activeEra = -1;
    std::vector<Slot> eraSlots[8];


    void addSlot(int era, glm::vec3 pos, glm::vec3 size, glm::vec4 params, int animType = 0, float animPhase = 0.0f, float rotZ = 0.0f);
    void addResortTower(int era, float x, float z, float depth, float height, float frontage,
                        const glm::vec3& c1, const glm::vec3& c2, float seed);
    void addPyramid(int era, float x, float z, float baseW, float totalH, int tiers,
                    const glm::vec3& bodyCol, const glm::vec3& trimCol, bool withBeam);
    void addJungleTree(int era, float x, float z, float trunkH, float crownW, const glm::vec3& bark,
                       const glm::vec3& leaf1, const glm::vec3& leaf2, const glm::vec3& bioGlow);

    void buildEra0_NightCity2077();
    void buildEra1_LasVegas1985();
    void buildEra2_NeoTokyoAkira();
    void buildEra3_ArcticAurora();
    void buildEra4_OrbitalStation();
    void buildEra5_SaharaPyramids();
    void buildEra6_NeonJungle();
    void buildEra7_MarsColony();
    void uploadEraParams(int era);

public:
    VegasCity();
    void draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float scroll, const glm::vec3& camPos,
              float time, int era, const glm::vec3& fogColor);
};

#endif
