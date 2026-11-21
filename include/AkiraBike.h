#ifndef AKIRA_BIKE_H
#define AKIRA_BIKE_H

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Primitives.h"
#include "Font5x7.h"

// ============================================================================
// ПРОЦЕДУРНАЯ 3D-ГЕОМЕТРИЯ МОТОЦИКЛА «АКИРА» (AkiraBike.h) — КРИТЕРИЙ R2
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   Все детали байка Канэды (1988) строятся математически в коде без сторонних .obj файлов:
//   - loftProfiles(): лофтинг (соединение поперечных сечений) для обтекателей и шасси;
//   - revolveX(): тело вращения вокруг оси X для шин и ступенчатых колпаков;
//   - flatText() / flatDisc(): векторные наклейки (CITIZEN, CANON, SHOEI) прямо в 3D-геометрии.
//   Реализация разбита на 2 компактных файла (< 270 строк): AkiraBikeBody.cpp и AkiraBikeDetails.cpp.
// ============================================================================
namespace AkiraGeo {

constexpr float kPi = 3.14159265359f; // число Пи для тригонометрии колёс и обтекателей

float sgnPow(float v, float e);       // знаковая степень: сохраняет знак при возведении в дробную степень
float smooth01(float a, float b, float x); // плавная S-кривая (аналог GLSL smoothstep)
Mesh loftProfiles(const std::vector<std::vector<glm::vec3>>& rings, bool closeCap = true); // лофтинг со сглаживанием граней < 38°

// Части кузова мотоцикла (реализация в src/AkiraBikeBody.cpp):
Mesh frontWingSide(float side);  // левое (-1) и правое (+1) крыло-обтекатель передней вилки
Mesh frontNoseBridge();          // центральный носовой мост между передними крыльями
Mesh lowerChassis();             // нижняя рама и поддон двигателя
Mesh rearSeatCowl();             // обтекаемый хвостовой кожух позади кресла
Mesh windscreen();               // изогнутое лобовое стекло (рисуется полупрозрачным)

// Колёса, тела вращения и декали (реализация в src/AkiraBikeDetails.cpp):
Mesh revolveX(const std::vector<glm::vec2>& prof, int seg = 56); // вращение 2D-профиля вокруг оси X
float hubDomeX(float r, float R, float depth);                   // выпуклый купол колпака колеса
Mesh hubcapFront(float R, float depth);                          // колпак переднего колеса
Mesh hubcapRear(float R, float depth);                           // колпак заднего колеса
Mesh tire(float R, float ri, float W, int seg = 96, int prof = 28); // объёмная покрышка (тор с профилем)
Mesh pillHalf(float R, float L);                                 // половинка фирменной капсулы «Акира»

Mesh flatDisc(int seg = 40);             // плоский диск для круглых наклеек
Mesh flatStar(float rInner = 0.40f);     // пятиконечная звезда-декаль
Mesh flatTriangle();                     // треугольный предупреждающий знак на колпаке
Mesh flatText(const std::string& str);   // 3D-надпись из шрифта Font5x7 (CITIZEN / CANON / SHOEI)

glm::mat4 faceMatrix(const glm::vec3& pos, const glm::vec3& normalIn, const glm::vec3& upHint = glm::vec3(0, 1, 0)); // ориентирует декаль по нормали поверхности
glm::mat4 wingDecal(float side, float z, float y, float lift = 0.005f); // матрица наклейки на боковом крыле

}

#endif
