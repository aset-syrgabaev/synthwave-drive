#ifndef CARMODEL_H
#define CARMODEL_H

#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Primitives.h"

// Опорная секция профиля оригинального 1981 DeLorean DMC-12 (дизайн Джорджетто Джуджаро):
// резкие передние крылья, фирменные продольные выштамповки на капоте и выступающий "клинок" носа над фарами
struct Station {
    float z;
    float wBot;
    float wWaist;
    float wShoulder;
    float wHood;
    float yBot;
    float yWaist;
    float yShoulder;
    float yTop;
    float hoodGroove;
};

struct BikeSection {
    float z;
    float wBot;
    float wMid;
    float wTop;
    float yBot;
    float yMid;
    float yTop;
};

// Процедурный генератор 3D-геометрии DeLorean DMC-12 (кузов, двери, кабина) и световых шлейфов
namespace CarModel {

constexpr int kRing = 18;

void ringPoints(const Station& s, glm::vec3* out);
float tableLerp(const std::vector<std::pair<float, float>>& t, float z);
void quadTwoSided(std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d);
Mesh loftDeLorean(const std::vector<Station>& st);
Mesh body();
Mesh cabinGlass();
Mesh cabinFrame();

Mesh gullwingDoorMetal(float sx);
Mesh gullwingDoorGlass(float sx);
Mesh gullwingDoorTrim(float sx);

Mesh loftSmoothBike(const std::vector<BikeSection>& sec);
Mesh akiraFrontCowl();
Mesh akiraLowerTub();
Mesh akiraRearCowl();
Mesh akiraWindscreen();
Mesh lightTrailRibbon();

}

#endif
