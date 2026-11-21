#ifndef CARMODEL_H
#define CARMODEL_H

#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Primitives.h"

// ============================================================================
// ПРОЦЕДУРНАЯ 3D-ГЕОМЕТРИЯ DELOREAN DMC-12 (CarModel.h) — КРИТЕРИИ R2 И R3
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   Кузов культового 1981 DeLorean DMC-12 генерируется методом лофтинга по 105
//   поперечным сечениям (Station) вдоль оси Z: от переднего бампера (-2.16 м) до заднего (+2.14 м).
//   Двери «крыло чайки» строятся в локальных координатах шарнира на крыше,
//   чтобы в CarRenderDeLorean.cpp их можно было плавно открывать матрицей поворота R_hinge (R3).
// ============================================================================
struct Station {
    float z;          // продольная координата сечения вдоль длины машины
    float wBot;       // полуширина днища
    float wWaist;     // полуширина по поясной линии
    float wShoulder;  // полуширина по верхней кромке крыла
    float wHood;      // полуширина капота/крыши
    float yBot;       // высота днища (поднимается в арках колёс)
    float yWaist;     // высота поясного ребра
    float yShoulder;  // высота плечевой линии
    float yTop;       // высота капота/крыши
    float hoodGroove; // глубина фирменных продольных выштамповок на капоте
};

// Упрощённое сечение для сглаженного лофтинга деталей корпуса
struct BikeSection {
    float z;
    float wBot, wMid, wTop;
    float yBot, yMid, yTop;
};

// Генератор 3D-мешей DeLorean DMC-12 (реализация в CarModelBody.cpp и CarModelParts.cpp)
namespace CarModel {

constexpr int kRing = 18; // 18 контрольных точек в каждом поперечном кольце кузова

void ringPoints(const Station& s, glm::vec3* out);                              // раскладывает Station в 18 3D-точек
float tableLerp(const std::vector<std::pair<float, float>>& t, float z);        // линейная интерполяция ширины/высоты по Z
void quadTwoSided(std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d); // двусторонняя грань (для стёкол)
Mesh loftDeLorean(const std::vector<Station>& st);                              // сшивает кольца в 3D-меш с чёткими рёбрами < 24°
Mesh body();                                                                    // основной стальной корпус DeLorean
Mesh cabinGlass();                                                              // лобовое, боковые стёкла и задний фастбэк
Mesh cabinFrame();                                                              // крыша и стойки A/B/C

// Составные части левой (sx=-1) и правой (sx=+1) двери «крыло чайки» (крепятся на петле крыши, R3):
Mesh gullwingDoorMetal(float sx); // металлическая нижняя панель двери
Mesh gullwingDoorGlass(float sx); // боковое стекло двери с форточкой
Mesh gullwingDoorTrim(float sx);  // чёрная уплотнительная рамка двери

Mesh loftSmoothBike(const std::vector<BikeSection>& sec); // сглаженный лофтинг для обтекателей
Mesh akiraFrontCowl();     // передний обтекатель
Mesh akiraLowerTub();      // нижняя ванна шасси
Mesh akiraRearCowl();      // задний хвостовой обтекатель
Mesh akiraWindscreen();    // ветровое стекло
Mesh lightTrailRibbon();   // объёмная треугольная лента для неоновых шлейфов фар и турбин

}

#endif
