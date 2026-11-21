#include "Car.h"

// ============================================================================
// ИЕРАРХИЧЕСКИЙ РЕНДЕР МОТОЦИКЛА «АКИРА» (CarRenderAkira.cpp) — КРИТЕРИИ R2, R3
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   От корневой матрицы root (computeRootMatrix) строятся дочерние узлы:
//   передняя вилка поворачивается на steerAngle, оба колеса вращаются от скорости
//   (wheelRot) и складываются на 90° при нажатии F (hoverBlend) в реактивные сопла!
// ============================================================================

// Вспомогательные функции для отрисовки цилиндрических трубок и сферических узлов мотоцикла
void Car::tube(const Shader& s, const glm::mat4& root, const glm::vec3& a, const glm::vec3& b, float r,
               const glm::vec3& col, const glm::vec3& em, float shin, float refl) const {
    glm::vec3 d = b - a;
    float len = glm::length(d);
    if (len < 1e-4f) return;
    glm::vec3 dir = d / len;
    glm::mat4 m = glm::translate(glm::mat4(1.0f), (a + b) * 0.5f);
    glm::vec3 axis = glm::cross(glm::vec3(1, 0, 0), dir);
    float al = glm::length(axis);
    if (al > 1e-5f) m = glm::rotate(m, std::atan2(al, glm::dot(glm::vec3(1, 0, 0), dir)), axis / al);
    else if (dir.x < 0.0f) m = glm::rotate(m, 3.14159265f, glm::vec3(0, 1, 0));
    drawPart(s, cylinder, root * glm::scale(m, glm::vec3(len, r, r)), col, em, shin, refl);
}

void Car::ball(const Shader& s, const glm::mat4& root, const glm::vec3& p, const glm::vec3& radii,
               const glm::vec3& col, const glm::vec3& em, float shin, float refl) const {
    drawPart(s, sphere, root * glm::scale(glm::translate(glm::mat4(1.0f), p), radii), col, em, shin, refl);
}

// Отрисовка мотоцикла из аниме "Акира" (без водителя, с открытым кожаным креслом и приборной панелью)
void Car::drawAkiraBike(const Shader& s, const glm::mat4& root, const glm::vec3& bikePaint, const glm::vec3& neonCol,
                   const glm::vec3& darkTrim, float bodyShin, float bodyRefl) const {
    using namespace AkiraGeo;
    float fold = hoverBlend * hoverBlend * (3.0f - 2.0f * hoverBlend);
    glm::vec3 tireRubber(0.026f, 0.026f, 0.032f);
    glm::vec3 chrome(0.84f, 0.88f, 0.94f);
    glm::vec3 silverRim(0.68f, 0.72f, 0.78f);
    glm::vec3 gunmetal(0.24f, 0.23f, 0.26f);
    glm::vec3 innerCowlCol(0.32f, 0.25f, 0.24f);
    glm::vec3 black(0.035f, 0.040f, 0.050f);
    glm::vec3 white(0.95f, 0.96f, 0.98f);
    glm::vec3 leatherMain(0.54f, 0.33f, 0.18f);
    glm::vec3 leatherBolster(0.40f, 0.23f, 0.12f);
    glm::vec3 leatherDark(0.18f, 0.10f, 0.05f);
    glm::vec3 blueDecal(0.10f, 0.28f, 0.82f);
    glm::vec3 redDecal(0.88f, 0.07f, 0.07f);
    glm::vec3 greenDecal(0.10f, 0.62f, 0.28f);
    glm::vec3 yellowDecal(0.98f, 0.80f, 0.12f);
    glm::vec3 darkRed = bikePaint * 0.56f;
    glm::vec3 paintFill = bikePaint * 0.10f;
    const SkinStyle& sk = getSkinStyle(paintIdx);
    glm::vec3 lampWarm = headlights ? sk.headlight : glm::vec3(0.20f);
    glm::vec3 redLed = sk.taillight;
    glm::vec3 amber(2.6f, 1.2f, 0.10f);
    float brakeBoost = (braking || akiraSlideTimer > 0.0f) ? 2.1f : 1.0f;

    // 1. Переднее и заднее колёса с протектором и ступенчатыми колпаками
    const float wz[2] = {-1.30f, 1.26f};
    const float wr[2] = { 0.42f, 0.45f};
    const float hw[2] = { 0.16f, 0.23f};
    const float hubR[2] = {0.31f, 0.24f};
    const float hubDepth[2] = {0.042f, 0.055f};
    for (int i = 0; i < 2; ++i) {
        float wy = (i == 0 ? -0.095f : -0.068f) - 0.04f * fold;
        glm::mat4 hub = glm::translate(root, glm::vec3(0.0f, wy, wz[i]));
        if (i == 0 && fold < 0.25f) hub = glm::rotate(hub, glm::radians(steerAngle * 0.65f), glm::vec3(0, 1, 0));
        hub = glm::rotate(hub, glm::radians(90.0f * fold), glm::vec3(0, 0, 1));
        glm::mat4 spin = glm::rotate(hub, glm::radians(wheelRot * (1.0f - fold)), glm::vec3(1, 0, 0));

        drawPart(s, (i == 0) ? akiraTireFrontMesh : akiraTireRearMesh, spin, tireRubber, glm::vec3(0.0f), 18.0f, 0.05f);

        // У заднего колеса виден обод с подсветкой скина, а у переднего — тормозной диск и суппорт
        glm::vec3 rimEm = (sk.rimGlow > 0.05f) ? (sk.accent * (1.45f * sk.rimGlow)) : glm::vec3(0.0f);
        if (i == 1) {
            drawPart(s, cylinder, spin * glm::scale(glm::mat4(1.0f), glm::vec3(hw[1] * 2.02f, 0.320f, 0.320f)), (sk.rimGlow > 0.05f) ? sk.accent : silverRim, rimEm, 205.0f, 0.82f);
            drawPart(s, cylinder, spin * glm::scale(glm::mat4(1.0f), glm::vec3(hw[1] * 2.05f, 0.270f, 0.270f)), gunmetal, glm::vec3(0.0f), 140.0f, 0.55f);
        } else {
            drawPart(s, cylinder, spin * glm::scale(glm::mat4(1.0f), glm::vec3(hw[0] * 1.96f, 0.305f, 0.305f)), (sk.rimGlow > 0.05f) ? sk.accent : silverRim * 0.9f, rimEm, 210.0f, 0.85f);
            drawPart(s, cube, hub * boxAt(glm::vec3(0.0f, -0.18f, 0.16f), glm::vec3(hw[0] * 2.10f, 0.095f, 0.115f)), glm::vec3(0.78f, 0.58f, 0.18f), glm::vec3(0.0f), 160.0f, 0.72f);
        }

        for (float sx : {-1.0f, 1.0f}) {
            glm::mat4 capM = glm::scale(glm::translate(hub, glm::vec3(sx * hw[i], 0.0f, 0.0f)), glm::vec3(sx, 1.0f, 1.0f));
            drawPart(s, (i == 0) ? akiraHubFrontMesh : akiraHubRearMesh, capM, bikePaint, paintFill, bodyShin, bodyRefl);

            // Кольцо и центральная гайка ступицы
            float cx = sx * (hw[i] + hubDepth[i] * 0.92f);
            drawPart(s, cylinder, glm::scale(glm::translate(hub, glm::vec3(cx, 0.0f, 0.0f)), glm::vec3(0.026f, 0.070f, 0.070f)),
                     gunmetal, glm::vec3(0.0f), 160.0f, 0.6f);
            drawPart(s, cylinder, glm::scale(glm::translate(hub, glm::vec3(cx, 0.0f, 0.0f)), glm::vec3(0.032f, 0.038f, 0.038f)),
                     silverRim, glm::vec3(0.0f), 210.0f, 0.85f);

            // Обтекаемый тонкий кожух стойки передней вилки, плавно уходящий в крыло
            if (i == 0) {
                glm::mat4 forkClamp = glm::translate(hub, glm::vec3(sx * (hw[0] + 0.018f), 0.12f, 0.06f));
                forkClamp = glm::rotate(forkClamp, glm::radians(-28.0f), glm::vec3(1, 0, 0));
                drawPart(s, cube, glm::scale(forkClamp, glm::vec3(0.026f, 0.26f, 0.105f)), bikePaint, paintFill, bodyShin, bodyRefl);
            }

            if (i == 0 && fold < 0.2f) {
                auto onHub = [&](float y, float z) {
                    float r = std::sqrt(y * y + z * z);
                    return glm::vec3(sx * (hw[0] + hubDomeX(r, hubR[0], hubDepth[0]) + 0.003f), y, z);
                };
                // Наклейка SHOEI внизу переднего колпака
                glm::mat4 f1 = faceMatrix(onHub(-0.18f, -0.02f), glm::vec3(sx, 0, 0));
                drawPart(s, cube, hub * f1 * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.002f)), glm::vec3(0.21f, 0.064f, 0.004f)),
                         white, glm::vec3(0.0f), 90.0f, 0.25f);
                drawPart(s, cube, hub * f1 * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.0032f)), glm::vec3(0.185f, 0.046f, 0.004f)),
                         blueDecal, glm::vec3(0.0f), 90.0f, 0.25f);
                drawPart(s, akiraTextShoei, hub * f1 * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.0056f)), glm::vec3(0.0058f, 0.0058f, 1.0f)),
                         white, glm::vec3(0.0f), 90.0f, 0.25f);
                // Треугольная наклейка и жёлтый овал спереди на колпаке
                glm::mat4 f2 = faceMatrix(onHub(0.06f, -0.13f), glm::vec3(sx, 0, 0));
                drawPart(s, akiraDecalTri, hub * f2 * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.0f, 0.0030f)), glm::vec3(0.076f, 0.076f, 1.0f)),
                         black, glm::vec3(0.0f), 90.0f, 0.25f);
                drawPart(s, akiraDecalTri, hub * f2 * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.0f, 0.0040f)), glm::vec3(0.060f, 0.060f, 1.0f)),
                         yellowDecal, glm::vec3(0.0f), 90.0f, 0.25f);
                drawPart(s, akiraDecalTri, hub * f2 * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.004f, 0.0050f)), glm::vec3(0.032f, 0.032f, 1.0f)),
                         redDecal, glm::vec3(0.0f), 90.0f, 0.25f);
                glm::mat4 f3 = faceMatrix(onHub(-0.04f, -0.20f), glm::vec3(sx, 0, 0));
                drawPart(s, akiraDecalDisc, hub * f3 * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.0030f)), glm::vec3(0.044f, 0.064f, 1.0f)),
                         black, glm::vec3(0.0f), 90.0f, 0.25f);
                drawPart(s, akiraDecalDisc, hub * f3 * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.0040f)), glm::vec3(0.035f, 0.052f, 1.0f)),
                         yellowDecal, glm::vec3(0.0f), 90.0f, 0.25f);
            }
        }

        // Свечение турбин при включении режима полёта (F)
        if (fold > 0.05f) {
            float pulse = 0.85f + 0.15f * std::sin(clock * 32.0f + (float)i * 2.5f);
            glm::mat4 jet = glm::translate(hub, glm::vec3(-hw[i] * 0.9f, 0.0f, 0.0f));
            part(s, sphere, jet, glm::scale(glm::mat4(1.0f), glm::vec3(0.15f * pulse, wr[i] * 0.56f, wr[i] * 0.56f)),
                 neonCol, neonCol * (2.8f * fold * pulse), 32.0f, 0.0f);
        }
    }

    // 2. Основные кузовные панели мотоцикла (передние крылья, носовая перемычка, нижнее шасси, задний кожух)
    drawPart(s, akiraWingLMesh,        root, bikePaint, paintFill, bodyShin, bodyRefl);
    drawPart(s, akiraWingRMesh,        root, bikePaint, paintFill, bodyShin, bodyRefl);
    drawPart(s, akiraNoseBridgeMesh,   root, bikePaint, paintFill, bodyShin, bodyRefl);
    drawPart(s, akiraLowerChassisMesh, root, bikePaint, paintFill * 0.85f, bodyShin, bodyRefl);
    drawPart(s, akiraRearPodMesh,      root, bikePaint, paintFill, bodyShin, bodyRefl);

    // 3. Тёмный технический кожух под лобовым стеклом и передняя фара
    glm::mat4 cowlM = glm::translate(root, glm::vec3(0.0f, 0.48f, -0.74f));
    cowlM = glm::rotate(cowlM, glm::radians(-13.0f), glm::vec3(1, 0, 0));
    drawPart(s, cube, glm::scale(cowlM, glm::vec3(0.36f, 0.08f, 0.64f)), innerCowlCol, glm::vec3(0.0f), 90.0f, 0.35f);
    drawPart(s, cube, glm::scale(glm::translate(cowlM, glm::vec3(0.0f, 0.045f, 0.02f)), glm::vec3(0.15f, 0.028f, 0.54f)), gunmetal, glm::vec3(0.0f), 120.0f, 0.45f);
    for (float sx : {-1.0f, 1.0f}) {
        drawPart(s, cube, glm::scale(glm::translate(cowlM, glm::vec3(sx * 0.12f, 0.044f, -0.10f)), glm::vec3(0.075f, 0.018f, 0.20f)), black, glm::vec3(0.0f), 60.0f, 0.2f);
        // Чёрные направляющие рамки вдоль нижнего края ветровика и хромированные стойки вилки
        tube(s, root, glm::vec3(sx * 0.22f, 0.42f, -1.26f), glm::vec3(sx * 0.33f, 0.62f, -0.36f), 0.014f, black, glm::vec3(0.0f), 110.0f, 0.35f);
        tube(s, root, glm::vec3(sx * 0.175f, -0.10f, -1.28f), glm::vec3(sx * 0.175f, 0.34f, -1.04f), 0.022f, chrome, glm::vec3(0.0f), 235.0f, 0.92f);
        // Боковые крепления ветровика и зеркал у руля
        glm::mat4 earM = glm::translate(root, glm::vec3(sx * 0.35f, 0.62f, -0.36f));
        earM = glm::rotate(earM, glm::radians(22.0f), glm::vec3(1, 0, 0));
        drawPart(s, cube, glm::scale(earM, glm::vec3(0.06f, 0.10f, 0.16f)), bikePaint, paintFill, bodyShin, bodyRefl);
    }

    // Утопленная двойная оптика фары в носовом обтекателе + тонкая диодная бровь в цвет скина
    drawPart(s, cube, root * boxAt(glm::vec3(0.0f, 0.312f, -1.545f), glm::vec3(0.22f, 0.064f, 0.055f)), black, glm::vec3(0.0f), 80.0f, 0.2f);
    for (float sx : {-0.052f, 0.052f}) {
        drawPart(s, cube, root * boxAt(glm::vec3(sx, 0.312f, -1.568f), glm::vec3(0.082f, 0.038f, 0.020f)), white, lampWarm, 220.0f, 0.85f);
    }
    drawPart(s, cube, root * boxAt(glm::vec3(0.0f, 0.338f, -1.564f), glm::vec3(0.195f, 0.012f, 0.016f)), sk.accent, lampWarm * 0.85f, 200.0f, 0.7f);

    // Решётка радиатора позади переднего колеса
    glm::mat4 radM = glm::translate(root, glm::vec3(0.0f, -0.06f, -0.84f));
    radM = glm::rotate(radM, glm::radians(34.0f), glm::vec3(1, 0, 0));
    drawPart(s, cube, glm::scale(radM, glm::vec3(0.32f, 0.36f, 0.08f)), gunmetal, glm::vec3(0.0f), 90.0f, 0.3f);
    for (int r = -2; r <= 2; ++r) {
        drawPart(s, cube, glm::scale(glm::translate(radM, glm::vec3(0.0f, (float)r * 0.06f, -0.042f)), glm::vec3(0.28f, 0.018f, 0.015f)), black, glm::vec3(0.0f), 60.0f, 0.2f);
    }

    // 4. Задний маятник и боковые моторные капсулы на нижней раме
    float wyR = -0.068f - 0.04f * fold;
    for (float sx : {-1.0f, 1.0f}) {
        // Тёмный шов между панелями переднего крыла
        glm::mat4 seamF = glm::translate(root, glm::vec3(sx * 0.34f, 0.05f, -0.84f));
        seamF = glm::rotate(seamF, glm::radians(-26.0f), glm::vec3(1, 0, 0));
        drawPart(s, cube, glm::scale(seamF, glm::vec3(0.12f, 0.014f, 0.86f)), black, glm::vec3(0.0f), 60.0f, 0.2f);

        // Передняя и задняя боковые турбо-капсулы внизу рамы
        glm::mat4 pod1 = glm::translate(root, glm::vec3(sx * 0.33f, -0.22f, 0.06f));
        drawPart(s, cube, glm::scale(pod1, glm::vec3(0.12f, 0.18f, 0.34f)), bikePaint, paintFill, bodyShin, bodyRefl);
        drawPart(s, cylinder, glm::scale(glm::translate(root, glm::vec3(sx * 0.365f, -0.23f, 0.08f)), glm::vec3(0.06f, 0.082f, 0.082f)), darkRed, glm::vec3(0.0f), 160.0f, 0.55f);
        drawPart(s, cylinder, glm::scale(glm::translate(root, glm::vec3(sx * 0.375f, -0.23f, 0.08f)), glm::vec3(0.05f, 0.058f, 0.058f)), gunmetal, glm::vec3(0.0f), 160.0f, 0.55f);

        glm::mat4 pod2 = glm::translate(root, glm::vec3(sx * 0.34f, -0.20f, 0.44f));
        drawPart(s, cube, glm::scale(pod2, glm::vec3(0.13f, 0.21f, 0.42f)), bikePaint, paintFill, bodyShin, bodyRefl);
        drawPart(s, cylinder, glm::scale(glm::translate(root, glm::vec3(sx * 0.380f, -0.21f, 0.46f)), glm::vec3(0.06f, 0.115f, 0.115f)), black, glm::vec3(0.0f), 120.0f, 0.4f);
        drawPart(s, cylinder, glm::scale(glm::translate(root, glm::vec3(sx * 0.388f, -0.21f, 0.46f)), glm::vec3(0.05f, 0.092f, 0.092f)), darkRed, glm::vec3(0.0f), 160.0f, 0.55f);

        // Прямоугольный рычаг заднего маятника
        glm::mat4 swingArm = glm::translate(root, glm::vec3(sx * 0.285f, 0.06f, 0.90f));
        swingArm = glm::rotate(swingArm, glm::radians(-26.0f), glm::vec3(1, 0, 0));
        drawPart(s, cube, glm::scale(swingArm, glm::vec3(0.11f, 0.22f, 0.62f)), bikePaint, paintFill, bodyShin, bodyRefl);
        drawPart(s, cube, glm::scale(glm::translate(swingArm, glm::vec3(sx * 0.057f, 0.0f, 0.0f)), glm::vec3(0.006f, 0.225f, 0.016f)), black, glm::vec3(0.0f), 60.0f, 0.2f);
        drawPart(s, cube, glm::scale(glm::translate(swingArm, glm::vec3(sx * 0.057f, -0.04f, 0.0f)), glm::vec3(0.006f, 0.014f, 0.58f)), black, glm::vec3(0.0f), 60.0f, 0.2f);

        // Зажим задней оси
        glm::mat4 hubClamp = glm::translate(root, glm::vec3(sx * 0.275f, wyR, 1.26f));
        drawPart(s, cube, glm::scale(hubClamp, glm::vec3(0.10f, 0.24f, 0.28f)), bikePaint, paintFill, bodyShin, bodyRefl);
        drawPart(s, cylinder, glm::scale(hubClamp, glm::vec3(0.11f, 0.15f, 0.15f)), bikePaint, paintFill, bodyShin, bodyRefl);
        drawPart(s, cube, glm::scale(glm::translate(hubClamp, glm::vec3(sx * 0.053f, 0.0f, 0.0f)), glm::vec3(0.006f, 0.016f, 0.27f)), black, glm::vec3(0.0f), 60.0f, 0.2f);

        // Хромированный амортизатор задней подвески
        tube(s, root, glm::vec3(sx * 0.24f, 0.34f, 0.86f), glm::vec3(sx * 0.26f, 0.06f, 1.18f), 0.018f, chrome, glm::vec3(0.0f), 220.0f, 0.9f);
        tube(s, root, glm::vec3(sx * 0.246f, 0.27f, 0.94f), glm::vec3(sx * 0.256f, 0.13f, 1.10f), 0.030f, black, glm::vec3(0.0f), 90.0f, 0.35f);
    }

    // 5. Открытый кокпит: многосекционное кожаное кресло-ковш и педали
    drawPart(s, cube, root * boxAt(glm::vec3(0.0f, -0.18f, -0.14f), glm::vec3(0.46f, 0.05f, 0.54f)), gunmetal * 0.6f, glm::vec3(0.0f), 60.0f, 0.2f);
    for (float sx : {-1.0f, 1.0f}) {
        drawPart(s, cube, root * boxAt(glm::vec3(sx * 0.24f, -0.04f, -0.10f), glm::vec3(0.05f, 0.26f, 0.62f)), leatherDark, glm::vec3(0.0f), 40.0f, 0.1f);
        glm::mat4 pedM = glm::translate(root, glm::vec3(sx * 0.15f, -0.10f, -0.34f));
        pedM = glm::rotate(pedM, glm::radians(35.0f), glm::vec3(1, 0, 0));
        drawPart(s, cube, glm::scale(pedM, glm::vec3(0.09f, 0.12f, 0.03f)), silverRim, glm::vec3(0.0f), 160.0f, 0.6f);
    }

    // Нижняя подушка сиденья и передний валик под колени
    drawPart(s, cube, root * boxAt(glm::vec3(0.0f, -0.08f, 0.16f), glm::vec3(0.42f, 0.10f, 0.38f)), leatherMain, leatherMain * 0.06f, 52.0f, 0.16f);
    glm::mat4 thighRoll = glm::translate(root, glm::vec3(0.0f, -0.04f, -0.02f));
    thighRoll = glm::rotate(thighRoll, glm::radians(18.0f), glm::vec3(1, 0, 0));
    drawPart(s, cube, glm::scale(thighRoll, glm::vec3(0.42f, 0.11f, 0.14f)), leatherBolster, leatherBolster * 0.06f, 52.0f, 0.16f);
    drawPart(s, cube, root * boxAt(glm::vec3(0.0f, -0.025f, 0.14f), glm::vec3(0.025f, 0.015f, 0.42f)), leatherDark, glm::vec3(0.0f), 30.0f, 0.05f);

    // Трёхсекционная спинка кресла с подголовником и боковыми валиками поддержки
    glm::mat4 backBase = glm::translate(root, glm::vec3(0.0f, 0.25f, 0.40f));
    backBase = glm::rotate(backBase, glm::radians(26.0f), glm::vec3(1, 0, 0));
    drawPart(s, cube, glm::scale(backBase, glm::vec3(0.44f, 0.56f, 0.08f)), leatherDark, glm::vec3(0.0f), 36.0f, 0.1f);
    drawPart(s, cube, glm::scale(glm::translate(backBase, glm::vec3(0.0f, -0.14f, -0.045f)), glm::vec3(0.36f, 0.18f, 0.06f)), leatherMain, leatherMain * 0.06f, 56.0f, 0.16f);
    drawPart(s, cube, glm::scale(glm::translate(backBase, glm::vec3(0.0f,  0.06f, -0.045f)), glm::vec3(0.36f, 0.19f, 0.06f)), leatherMain, leatherMain * 0.06f, 56.0f, 0.16f);
    drawPart(s, cube, glm::scale(glm::translate(backBase, glm::vec3(0.0f,  0.24f, -0.040f)), glm::vec3(0.28f, 0.13f, 0.07f)), leatherBolster, leatherBolster * 0.06f, 56.0f, 0.16f);
    drawPart(s, cube, glm::scale(glm::translate(backBase, glm::vec3(0.0f,  0.02f, -0.078f)), glm::vec3(0.024f, 0.48f, 0.012f)), leatherDark, glm::vec3(0.0f), 30.0f, 0.05f);

    for (float sx : {-1.0f, 1.0f}) {
        glm::mat4 sideBolster = glm::translate(backBase, glm::vec3(sx * 0.20f, -0.02f, -0.07f));
        sideBolster = glm::rotate(sideBolster, glm::radians(-sx * 18.0f), glm::vec3(0, 1, 0));
        drawPart(s, cube, glm::scale(sideBolster, glm::vec3(0.09f, 0.48f, 0.14f)), leatherBolster, leatherBolster * 0.06f, 52.0f, 0.15f);

        glm::mat4 seatSide = glm::translate(root, glm::vec3(sx * 0.20f, -0.01f, 0.12f));
        seatSide = glm::rotate(seatSide, glm::radians(12.0f), glm::vec3(1, 0, 0));
        drawPart(s, cube, glm::scale(seatSide, glm::vec3(0.09f, 0.12f, 0.38f)), leatherBolster, leatherBolster * 0.06f, 52.0f, 0.15f);
    }

    // 6. Бортовой компьютер, приборные экраны и рулевые рукоятки
    glm::mat4 dashM = glm::translate(root, glm::vec3(0.0f, 0.42f, -0.34f));
    dashM = glm::rotate(dashM, glm::radians(-38.0f), glm::vec3(1, 0, 0));
    drawPart(s, cube, glm::scale(dashM, glm::vec3(0.38f, 0.10f, 0.30f)), silverRim * 0.85f, glm::vec3(0.0f), 160.0f, 0.6f);
    drawPart(s, cube, glm::scale(glm::translate(dashM, glm::vec3(0.0f, 0.015f, 0.01f)), glm::vec3(0.34f, 0.09f, 0.26f)), black, glm::vec3(0.0f), 110.0f, 0.4f);
    drawPart(s, cube, glm::scale(glm::translate(dashM, glm::vec3(0.04f, 0.062f, 0.02f)), glm::vec3(0.18f, 0.008f, 0.14f)),
             glm::vec3(0.12f, 0.92f, 0.78f), glm::vec3(0.18f, 1.6f, 1.4f), 90.0f, 0.3f);
    drawPart(s, cube, glm::scale(glm::translate(dashM, glm::vec3(-0.11f, 0.062f, -0.04f)), glm::vec3(0.07f, 0.010f, 0.05f)),
             glm::vec3(1.0f, 0.22f, 0.14f), glm::vec3(1.9f, 0.3f, 0.1f));
    drawPart(s, cube, glm::scale(glm::translate(dashM, glm::vec3(-0.11f, 0.062f,  0.04f)), glm::vec3(0.07f, 0.010f, 0.05f)),
             glm::vec3(0.95f, 0.78f, 0.15f), glm::vec3(1.6f, 1.2f, 0.1f));

    for (float sx : {-1.0f, 1.0f}) {
        tube(s, root, glm::vec3(sx * 0.11f, 0.46f, -0.38f), glm::vec3(sx * 0.16f, 0.60f, -0.32f), 0.020f, silverRim, glm::vec3(0.0f), 180.0f, 0.75f);
        tube(s, root, glm::vec3(sx * 0.16f, 0.60f, -0.32f), glm::vec3(sx * 0.29f, 0.57f, -0.26f), 0.022f, black, glm::vec3(0.0f), 64.0f, 0.2f);
        tube(s, root, glm::vec3(sx * 0.18f, 0.59f, -0.35f), glm::vec3(sx * 0.28f, 0.57f, -0.29f), 0.008f, chrome, glm::vec3(0.0f), 220.0f, 0.9f);
    }

    // 7. Наклейки на боковых крыльях (Canon, CITIZEN, звезда, зелёный круг) и задняя оптика
    auto plateAt = [&](const glm::mat4& f, float ox, float oy, float w, float h, const glm::vec3& col, float oz = 0.0f) {
        drawPart(s, cube, root * f * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(ox, oy, oz + 0.002f)), glm::vec3(w, h, 0.004f)),
                 col, glm::vec3(0.0f), 90.0f, 0.25f);
    };
    auto flatAt = [&](const Mesh& m, const glm::mat4& f, float ox, float oy, float oz, float sx, float sy,
                      const glm::vec3& col, const glm::vec3& em = glm::vec3(0.0f), float rotDeg = 0.0f) {
        glm::mat4 lm = glm::translate(glm::mat4(1.0f), glm::vec3(ox, oy, oz));
        if (rotDeg != 0.0f) lm = glm::rotate(lm, glm::radians(rotDeg), glm::vec3(0, 0, 1));
        drawPart(s, m, root * f * glm::scale(lm, glm::vec3(sx, sy, 1.0f)), col, em, 90.0f, 0.25f);
    };
    for (float sd : {-1.0f, 1.0f}) {
        glm::mat4 fs = wingDecal(sd, -1.04f, 0.34f, 0.018f);
        plateAt(fs, 0.0f, 0.0f, 0.32f, 0.060f, white);
        plateAt(fs, 0.0f,  0.023f, 0.32f, 0.012f, redDecal, 0.0015f);
        plateAt(fs, 0.0f, -0.023f, 0.32f, 0.012f, redDecal, 0.0015f);
        flatAt(akiraDecalDisc, fs, 0.0f, 0.0f, 0.0045f, 0.075f, 0.075f, blueDecal);
        flatAt(akiraDecalStar, fs, 0.0f, 0.0f, 0.0054f, 0.056f, 0.056f, white);

        glm::mat4 fc = wingDecal(sd, -0.54f, 0.44f, 0.018f);
        plateAt(fc, 0.0f, 0.0f, 0.29f, 0.070f, black);
        flatAt(akiraTextCitizen, fc, 0.0f, 0.0f, 0.0048f, 0.0058f, 0.0058f, white);

        glm::mat4 fk = wingDecal(sd, -0.78f, 0.24f, 0.018f);
        plateAt(fk, 0.0f, 0.0f, 0.32f, 0.088f, white);
        flatAt(akiraTextCanon, fk, 0.0f, 0.0f, 0.0048f, 0.0088f, 0.0088f, redDecal);

        glm::mat4 fg = wingDecal(sd, -1.04f, 0.16f, 0.018f);
        flatAt(akiraDecalDisc, fg, 0.0f, 0.0f, 0.0030f, 0.066f, 0.066f, white);
        flatAt(akiraDecalDisc, fg, 0.0f, 0.0f, 0.0040f, 0.052f, 0.052f, greenDecal);
        plateAt(fg, 0.0f, 0.0f, 0.105f, 0.013f, white, 0.0048f);

        bool blinkSide = (turnSignalDir != 0) && ((sd < 0.0f && turnSignalDir < 0) || (sd > 0.0f && turnSignalDir > 0));
        float ambBoost = (blinkSide && (static_cast<int>(clock * 14.0f) % 2 == 0)) ? 2.4f : 0.95f;
        ball(s, root, glm::vec3(sd * 0.27f, 0.56f, 0.74f), glm::vec3(0.032f, 0.038f, 0.068f), black, glm::vec3(0.0f), 80.0f, 0.2f);
        ball(s, root, glm::vec3(sd * 0.278f, 0.56f, 0.74f), glm::vec3(0.028f, 0.028f, 0.052f), amber, amber * ambBoost, 120.0f, 0.4f);

        tube(s, root, glm::vec3(sd * 0.072f, 0.455f, 1.04f), glm::vec3(sd * 0.072f, 0.455f, 1.21f), 0.034f, gunmetal, glm::vec3(0.0f), 185.0f, 0.75f);
        ball(s, root, glm::vec3(sd * 0.072f, 0.455f, 1.215f), glm::vec3(0.026f), glm::vec3(0.05f), redLed * (0.55f + 1.8f * boostGlow), 90.0f, 0.2f);
        drawPart(s, cube, root * boxAt(glm::vec3(sd * 0.078f, 0.502f, 1.15f), glm::vec3(0.092f, 0.024f, 0.026f)), redLed, redLed * brakeBoost, 120.0f, 0.4f);

        tube(s, root, glm::vec3(sd * 0.24f, -0.372f, -0.50f), glm::vec3(sd * 0.24f, -0.372f, 0.55f), 0.010f, neonCol, neonCol * 2.0f, 40.0f, 0.1f);
    }
}
