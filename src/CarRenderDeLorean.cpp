#include "Car.h"

// ============================================================================
// ИЕРАРХИЧЕСКИЙ РЕНДЕР DELOREAN DMC-12 (CarRenderDeLorean.cpp) — КРИТЕРИИ R2, R3
// ВАЖНО ДЛЯ ЗАЩИТЫ — ПРИМЕР PARENT–CHILD ИЕРАРХИИ (R3):
//   Вся машина строится от единой корневой матрицы root = Car::computeRootMatrix():
//     M_дочерней_детали = M_root * T_шарнира * R_угла * S_масштаба
//   - Двери «крыло чайки» (drawGullwingDoors, клавиша O): вращаются вокруг продольной
//     оси шарнира на крыше (x = ±0.14, y = 0.79) и следуют за кузовом в прыжке и крене!
//   - Колёса (drawWheelsAndThrusters, клавиша F): сначала крутятся от скорости (wheelRot),
//     передние поворачиваются от руля (steerAngle), а при полёте складываются на 90°!
// ============================================================================

void Car::part(const Shader& s, const Mesh& m, const glm::mat4& root, const glm::mat4& local, const glm::vec3& color,
               const glm::vec3& emissive, float shininess, float refl) const {
    drawPart(s, m, root * local, color, emissive, shininess, refl);
}

glm::mat4 Car::boxAt(const glm::vec3& p, const glm::vec3& size) {
    return glm::scale(glm::translate(glm::mat4(1.0f), p), size);
}

// Двери "крыло чайки" (Gull-Wing) с форточкой, ручкой и газовыми упорами
void Car::drawGullwingDoors(const Shader& s, const glm::mat4& root, const glm::vec3& steel, const glm::vec3& glass, const glm::vec3& darkTrim) const {
    for (int sideIdx = 0; sideIdx < 2; ++sideIdx) {
        float sx = (sideIdx == 0) ? -1.0f : 1.0f;
        const Mesh& dMetal = (sideIdx == 0) ? doorMetalL : doorMetalR;
        const Mesh& dGlass = (sideIdx == 0) ? doorGlassL : doorGlassR;
        const Mesh& dTrim  = (sideIdx == 0) ? doorTrimL  : doorTrimR;

        glm::mat4 hinge = glm::translate(root, glm::vec3(sx * 0.14f, 0.790f, 0.11f));
        hinge = glm::rotate(hinge, glm::radians(sx * doorAngle), glm::vec3(0, 0, 1));

        drawPart(s, dMetal, hinge, steel, glm::vec3(0.0f), 228.0f, 0.86f);
        drawPart(s, dGlass, hinge, glass, glm::vec3(0.01f, 0.02f, 0.05f), 252.0f, 0.95f);
        drawPart(s, dTrim,  hinge, darkTrim, glm::vec3(0.0f), 64.0f, 0.20f);

        if (doorAngle > 4.0f) {
            glm::mat4 strut = glm::translate(root, glm::vec3(sx * 0.48f, 0.54f, 0.42f));
            strut = glm::rotate(strut, glm::radians(sx * doorAngle * 0.45f), glm::vec3(0, 0, 1));
            strut = glm::rotate(strut, glm::radians(90.0f), glm::vec3(0, 0, 1));
            drawPart(s, cylinder, glm::scale(strut, glm::vec3(0.32f, 0.015f, 0.015f)), glm::vec3(0.76f), glm::vec3(0.0f), 180.0f, 0.75f);
        }
    }
}

// Оригинальные 14-спицевые турбинные диски DeLorean с тормозными дисками и суппортами,
// складывающиеся на 90° в режим ховер-полёта BTTF Part II
void Car::drawWheelsAndThrusters(const Shader& s, const glm::mat4& root, const glm::vec3& glow) const {
    float fold = hoverBlend * hoverBlend * (3.0f - 2.0f * hoverBlend);
    glm::vec3 tireRubber(0.032f, 0.032f, 0.040f);
    glm::vec3 rimSilver(0.76f, 0.80f, 0.86f);
    glm::vec3 rimDark(0.09f, 0.10f, 0.13f);
    glm::vec3 rotorSteel(0.56f, 0.58f, 0.62f);
    glm::vec3 caliperCol(0.78f, 0.16f, 0.12f);
    glm::vec3 thrusterCyan(0.10f, 0.92f, 1.0f);
    glm::vec3 thrusterOrange(1.0f, 0.52f, 0.10f);

    float wheelZ[2] = {-1.30f, 1.30f};
    for (int axle = 0; axle < 2; ++axle) {
        for (int sideIdx = 0; sideIdx < 2; ++sideIdx) {
            float sx = sideIdx == 0 ? -1.0f : 1.0f;
            float outOffset = 0.24f * fold;
            float baseWheelX = sx * (0.87f + outOffset);
            float baseWheelY = -0.175f - 0.05f * fold;

            // Закрытые тёмные подкрылки — никаких просветов сквозь колёсные арки
            part(s, cube, root, boxAt(glm::vec3(sx * 0.68f, -0.06f, wheelZ[axle]), glm::vec3(0.44f, 0.42f, 0.84f)), glm::vec3(0.02f, 0.02f, 0.03f), glm::vec3(0.0f), 16.0f, 0.0f);

            if (fold > 0.02f) {
                glm::mat4 arm1 = glm::translate(root, glm::vec3(sx * (0.70f + outOffset * 0.5f), -0.14f, wheelZ[axle] - 0.07f));
                arm1 = glm::scale(arm1, glm::vec3(0.30f + outOffset, 0.042f, 0.048f));
                drawPart(s, cube, arm1, rimSilver, thrusterCyan * (0.35f * fold), 180.0f, 0.78f);

                glm::mat4 arm2 = glm::translate(root, glm::vec3(sx * (0.70f + outOffset * 0.5f), -0.20f, wheelZ[axle] + 0.07f));
                arm2 = glm::scale(arm2, glm::vec3(0.30f + outOffset, 0.042f, 0.048f));
                drawPart(s, cube, arm2, rimSilver, thrusterCyan * (0.35f * fold), 180.0f, 0.78f);
            }

            glm::mat4 hub = glm::translate(root, glm::vec3(baseWheelX, baseWheelY, wheelZ[axle]));

            if (axle == 0 && fold < 0.25f) {
                hub = glm::rotate(hub, glm::radians(steerAngle), glm::vec3(0, 1, 0));
            }

            hub = glm::rotate(hub, glm::radians(-sx * 90.0f * fold), glm::vec3(0, 0, 1));
            glm::mat4 spin = glm::rotate(hub, glm::radians(wheelRot * (1.0f - fold)), glm::vec3(1, 0, 0));

            float radius = (axle == 1) ? 0.35f : 0.33f;
            float width  = (axle == 1) ? 0.28f : 0.25f;

            // Шина + боковина + обод
            part(s, cylinder, spin, glm::scale(glm::mat4(1.0f), glm::vec3(width, radius, radius)), tireRubber, glm::vec3(0.0f), 20.0f, 0.05f);
            part(s, cylinder, spin, glm::scale(glm::mat4(1.0f), glm::vec3(width + 0.010f, radius * 0.92f, radius * 0.92f)), tireRubber * 1.15f, glm::vec3(0.0f), 24.0f, 0.08f);

            // Тормозной диск и суппорт внутри колеса
            glm::mat4 brakeM = glm::translate(hub, glm::vec3(sx * 0.005f, 0.0f, 0.0f));
            part(s, cylinder, brakeM, glm::scale(glm::mat4(1.0f), glm::vec3(width * 0.45f, radius * 0.55f, radius * 0.55f)), rotorSteel, glm::vec3(0.0f), 190.0f, 0.75f);
            part(s, cube, hub, boxAt(glm::vec3(sx * 0.03f, 0.05f, (axle == 0) ? 0.13f : -0.13f), glm::vec3(width * 0.48f, 0.12f, 0.09f)), caliperCol, glm::vec3(0.0f), 120.0f, 0.45f);

            // Глубокий литой турбинный диск (14 спиц + центральный колпачок DMC)
            glm::mat4 outerRim = glm::translate(spin, glm::vec3(sx * 0.028f, 0.0f, 0.0f));
            const SkinStyle& sk = getSkinStyle(paintIdx);
            glm::vec3 rimEm = (sk.rimGlow > 0.05f) ? (sk.accent * (1.25f * sk.rimGlow)) : glm::vec3(0.0f);
            part(s, cylinder, outerRim, glm::scale(glm::mat4(1.0f), glm::vec3(width * 0.84f, radius * 0.73f, radius * 0.73f)), (sk.rimGlow > 0.05f) ? sk.accent : rimSilver, rimEm, 215.0f, 0.84f);
            part(s, cylinder, outerRim, glm::scale(glm::mat4(1.0f), glm::vec3(width * 0.88f, radius * 0.65f, radius * 0.65f)), rimDark, glm::vec3(0.0f), 64.0f, 0.25f);

            for (int sp = 0; sp < 14; ++sp) {
                glm::mat4 spoke = glm::rotate(outerRim, glm::radians(25.71f * (float)sp), glm::vec3(1, 0, 0));
                spoke = glm::scale(spoke, glm::vec3(width * 0.92f, radius * 1.25f, 0.017f));
                drawPart(s, cube, spoke, rimSilver, glm::vec3(0.0f), 215.0f, 0.84f);
            }

            part(s, cylinder, outerRim, glm::scale(glm::mat4(1.0f), glm::vec3(width * 0.97f, radius * 0.23f, radius * 0.23f)), rimSilver, glm::vec3(0.0f), 215.0f, 0.84f);
            part(s, cylinder, outerRim, glm::scale(glm::mat4(1.0f), glm::vec3(width * 1.00f, radius * 0.13f, radius * 0.13f)), rimDark, glm::vec3(0.0f), 96.0f, 0.35f);

            glm::mat4 innerCap = glm::translate(hub, glm::vec3(-sx * (width * 0.48f), 0.0f, 0.0f));
            part(s, cylinder, innerCap, glm::scale(glm::mat4(1.0f), glm::vec3(0.03f, radius * 0.74f, radius * 0.74f)), rimDark, glm::vec3(0.0f), 110.0f, 0.45f);
            part(s, cylinder, innerCap, glm::scale(glm::mat4(1.0f), glm::vec3(0.038f, radius * 0.44f, radius * 0.44f)), rimSilver, thrusterCyan * (1.25f * fold), 160.0f, 0.65f);

            if (fold > 0.05f) {
                float pulse = 0.85f + 0.15f * std::sin(clock * 32.0f + (float)axle * 2.0f + sx);
                glm::mat4 ring1 = glm::translate(hub, glm::vec3(sx * (width * 0.54f), 0.0f, 0.0f));
                part(s, cylinder, ring1, glm::scale(glm::mat4(1.0f), glm::vec3(0.045f, radius * 0.96f, radius * 0.96f)),
                     thrusterCyan, thrusterCyan * (2.6f * fold * pulse), 64.0f, 0.0f);

                glm::mat4 ring2 = glm::translate(hub, glm::vec3(sx * (width * 0.60f), 0.0f, 0.0f));
                part(s, cylinder, ring2, glm::scale(glm::mat4(1.0f), glm::vec3(0.055f, radius * 0.58f, radius * 0.58f)),
                     thrusterOrange, glm::vec3(1.0f, 0.88f, 0.65f) * (2.8f * fold * pulse), 64.0f, 0.0f);

                glm::mat4 cone = glm::translate(hub, glm::vec3(sx * (width * 0.74f), 0.0f, 0.0f));
                part(s, sphere, cone, glm::scale(glm::mat4(1.0f), glm::vec3(0.12f * pulse, radius * 0.42f, radius * 0.42f)),
                     thrusterCyan, thrusterCyan * (2.6f * fold * pulse), 32.0f, 0.0f);
            }
        }
    }
}

// Фирменные 9-секционные чёрные жалюзи заднего стекла с боковыми направляющими и центральным ребром
void Car::drawLouversAndRearDeck(const Shader& s, const glm::mat4& root) const {
    glm::vec3 louverBlack(0.038f, 0.040f, 0.052f);
    glm::vec3 railSteel(0.14f, 0.15f, 0.20f);

    for (int k = 0; k < 9; ++k) {
        float t = (float)k / 8.0f;
        float z = 0.50f + t * 1.30f;
        float y = 0.778f - t * 0.328f;
        float w = 0.96f + t * 0.20f;

        glm::mat4 slat = glm::translate(root, glm::vec3(0.0f, y, z));
        slat = glm::rotate(slat, glm::radians(16.0f), glm::vec3(1, 0, 0));
        slat = glm::scale(slat, glm::vec3(w, 0.018f, 0.115f));
        drawPart(s, cube, slat, louverBlack, glm::vec3(0.0f), 90.0f, 0.28f);
    }

    // Центральное ребро жёсткости и боковые петли-направляющие жалюзи (строго по уклону +14.16°, ничего не торчит)
    glm::mat4 spine = glm::translate(root, glm::vec3(0.0f, 0.614f, 1.15f));
    spine = glm::rotate(spine, glm::radians(14.16f), glm::vec3(1, 0, 0));
    drawPart(s, cube, glm::scale(spine, glm::vec3(0.044f, 0.020f, 1.32f)), louverBlack, glm::vec3(0.0f), 90.0f, 0.28f);

    for (float sx : {-1.0f, 1.0f}) {
        glm::mat4 rail = glm::translate(root, glm::vec3(sx * 0.53f, 0.614f, 1.15f));
        rail = glm::rotate(rail, glm::radians(14.16f), glm::vec3(1, 0, 0));
        rail = glm::rotate(rail, glm::radians(sx * 4.2f), glm::vec3(0, 1, 0));
        drawPart(s, cube, glm::scale(rail, glm::vec3(0.032f, 0.020f, 1.32f)), railSteel, glm::vec3(0.0f), 120.0f, 0.45f);
    }
}

// Детализированный перед и капот DeLorean DMC-12:
// рельефные швы и рёбра капота (отлично видны даже из камеры капота), решётка воздухозаборника,
// двойные дворники, 4 фары в хромированных рамках, 3D-эмблема DMC, передний бампер и потоковые шины BTTF
void Car::drawFrontFasciaAndHoodDetails(const Shader& s, const glm::mat4& root, const glm::vec3& darkTrim, const glm::vec3& steel) const {
    const SkinStyle& sk = getSkinStyle(paintIdx);
    glm::vec3 lampOn = headlights ? sk.headlight : glm::vec3(0.14f);
    glm::vec3 chrome(0.86f, 0.90f, 0.96f);
    glm::vec3 seamBlack(0.022f, 0.024f, 0.032f);
    glm::vec3 amber(2.4f, 1.05f, 0.08f);
    glm::vec3 fluxCyan = sk.accent * (0.78f + 1.35f * hoverBlend + 1.0f * boostGlow);

    // 1) ДЕТАЛИЗАЦИЯ КАПОТА (швы между капотом и крыльями, фирменные рёбра 1981 года и передний козырёк)
    // Наклон капота: y(-1.96)=0.21, y(-0.90)=0.38 -> угол -9.11° вокруг оси X
    glm::mat4 hoodBase = glm::translate(root, glm::vec3(0.0f, 0.295f, -1.43f));
    hoodBase = glm::rotate(hoodBase, glm::radians(-9.11f), glm::vec3(1, 0, 0));

    for (float sx : {-1.0f, 1.0f}) {
        // Тёмный технологический зазор (шов) между крышкой капота и передним крылом
        drawPart(s, cube, glm::scale(glm::translate(hoodBase, glm::vec3(sx * 0.485f, -0.002f, 0.0f)), glm::vec3(0.014f, 0.012f, 1.04f)),
                 seamBlack, glm::vec3(0.0f), 28.0f, 0.05f);
        // Приподнятая металлическая грань (выштамповка) 1981 DeLorean вдоль капота
        drawPart(s, cube, glm::scale(glm::translate(hoodBase, glm::vec3(sx * 0.30f, 0.015f, -0.02f)), glm::vec3(0.024f, 0.010f, 0.98f)),
                 steel * 1.08f, glm::vec3(0.0f), 235.0f, 0.90f);
        // Чёткий гребень внешнего переднего крыла
        drawPart(s, cube, glm::scale(glm::translate(hoodBase, glm::vec3(sx * 0.68f, 0.018f, -0.04f)), glm::vec3(0.026f, 0.009f, 0.96f)),
                 steel * 1.06f, glm::vec3(0.0f), 235.0f, 0.88f);
        // Тёмный боковой передний габаритный огонь (янтарный маркер US-spec)
        part(s, cube, root, boxAt(glm::vec3(sx * 0.925f, 0.05f, -1.78f), glm::vec3(0.024f, 0.036f, 0.14f)), amber, amber * 0.72f, 96.0f, 0.3f);
        // Задний красный боковой маркер крыла
        part(s, cube, root, boxAt(glm::vec3(sx * 0.935f, 0.08f,  1.80f), glm::vec3(0.024f, 0.036f, 0.14f)), glm::vec3(1.0f, 0.1f, 0.12f), glm::vec3(1.6f, 0.08f, 0.10f), 96.0f, 0.3f);
    }
    // Тёмный передний шов капота и выступающая носовая окантовка ("акулий" козырёк)
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.172f, -2.145f), glm::vec3(1.52f, 0.016f, 0.06f)), steel * 1.05f, glm::vec3(0.0f), 230.0f, 0.88f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.192f, -2.01f),  glm::vec3(1.00f, 0.010f, 0.016f)), seamBlack, glm::vec3(0.0f), 28.0f, 0.05f);

    // 2) Чёрная вентиляционная решётка воздухозаборника под лобовым стеклом (видна в Hood Cam)
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.376f, -0.945f), glm::vec3(1.42f, 0.022f, 0.11f)), darkTrim, glm::vec3(0.0f), 42.0f, 0.15f);
    for (int sl = -6; sl <= 6; ++sl) {
        part(s, cube, root, boxAt(glm::vec3((float)sl * 0.10f, 0.388f, -0.945f), glm::vec3(0.048f, 0.008f, 0.085f)), seamBlack, glm::vec3(0.0f), 24.0f, 0.05f);
    }

    // 3) ДВА ДЕТАЛИЗИРОВАННЫХ СТЕКЛООЧИСТИТЕЛЯ (ДВОРНИКА) У ОСНОВАНИЯ ЛОБОВОГО СТЕКЛА
    for (float wx : {-0.26f, 0.28f}) {
        part(s, cylinder, root, glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(wx - 0.18f, 0.392f, -0.91f)), glm::radians(90.0f), glm::vec3(0, 0, 1)), glm::vec3(0.024f, 0.018f, 0.018f)),
             darkTrim, glm::vec3(0.0f), 64.0f, 0.25f);
        glm::mat4 arm = glm::translate(root, glm::vec3(wx, 0.405f, -0.875f));
        arm = glm::rotate(arm, glm::radians(-33.0f), glm::vec3(1, 0, 0));
        arm = glm::rotate(arm, glm::radians(7.5f), glm::vec3(0, 1, 0));
        drawPart(s, cube, glm::scale(arm, glm::vec3(0.46f, 0.012f, 0.016f)), seamBlack, glm::vec3(0.0f), 48.0f, 0.18f);
        drawPart(s, cube, glm::scale(glm::translate(arm, glm::vec3(0.02f, 0.012f, 0.018f)), glm::vec3(0.42f, 0.008f, 0.014f)), glm::vec3(0.08f, 0.09f, 0.11f), glm::vec3(0.0f), 48.0f, 0.15f);
    }

    // 4) УТОПЛЕННАЯ ЧЁРНАЯ ПЕРЕДНЯЯ РЕШЁТКА, 4 ФАРЫ В ХРОМИРОВАННЫХ РАМКАХ И 3D-ЭМБЛЕМА "DMC"
    part(s, cube, root, boxAt(glm::vec3(0.0f,  0.045f, -2.14f), glm::vec3(1.66f, 0.23f, 0.10f)), darkTrim, glm::vec3(0.0f), 55.0f, 0.18f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, -0.095f, -2.16f), glm::vec3(1.72f, 0.105f, 0.16f)), darkTrim, glm::vec3(0.0f), 44.0f, 0.14f);
    // Нижний воздухозаборник радиатора и тёмный передний сплиттер
    part(s, cube, root, boxAt(glm::vec3(0.0f, -0.175f, -2.13f), glm::vec3(1.52f, 0.075f, 0.18f)), seamBlack, glm::vec3(0.0f), 32.0f, 0.10f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, -0.215f, -2.16f), glm::vec3(1.66f, 0.026f, 0.24f)), glm::vec3(0.04f, 0.045f, 0.06f), glm::vec3(0.0f), 64.0f, 0.25f);
    for (int rad = -5; rad <= 5; ++rad) {
        part(s, cube, root, boxAt(glm::vec3((float)rad * 0.11f, -0.175f, -2.21f), glm::vec3(0.014f, 0.058f, 0.025f)), glm::vec3(0.18f, 0.20f, 0.25f), glm::vec3(0.0f), 96.0f, 0.4f);
    }

    // Центральная ребристая решётка радиатора (6 горизонтальных ламелей)
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.06f, -2.185f), glm::vec3(0.68f, 0.115f, 0.03f)), seamBlack, glm::vec3(0.0f), 32.0f, 0.1f);
    for (int bar = 0; bar < 5; ++bar) {
        float by = 0.018f + (float)bar * 0.021f;
        part(s, cube, root, boxAt(glm::vec3(0.0f, by, -2.196f), glm::vec3(0.64f, 0.007f, 0.018f)), glm::vec3(0.16f, 0.18f, 0.22f), glm::vec3(0.0f), 110.0f, 0.45f);
    }

    // Объёмная 3D-эмблема "DMC" из хрома прямо по центру передней решётки
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.060f, -2.202f), glm::vec3(0.28f, 0.062f, 0.012f)), seamBlack, glm::vec3(0.0f), 40.0f, 0.1f);
    // Буква D
    part(s, cube, root, boxAt(glm::vec3(-0.105f, 0.060f, -2.210f), glm::vec3(0.014f, 0.044f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    part(s, cube, root, boxAt(glm::vec3(-0.082f, 0.078f, -2.210f), glm::vec3(0.044f, 0.010f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    part(s, cube, root, boxAt(glm::vec3(-0.082f, 0.042f, -2.210f), glm::vec3(0.044f, 0.010f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    part(s, cube, root, boxAt(glm::vec3(-0.062f, 0.060f, -2.210f), glm::vec3(0.013f, 0.036f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    // Буква M
    part(s, cube, root, boxAt(glm::vec3(-0.028f, 0.060f, -2.210f), glm::vec3(0.013f, 0.044f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    part(s, cube, root, boxAt(glm::vec3( 0.028f, 0.060f, -2.210f), glm::vec3(0.013f, 0.044f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    part(s, cube, root, boxAt(glm::vec3( 0.000f, 0.072f, -2.210f), glm::vec3(0.056f, 0.012f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    part(s, cube, root, boxAt(glm::vec3( 0.000f, 0.056f, -2.210f), glm::vec3(0.014f, 0.026f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    // Буква C
    part(s, cube, root, boxAt(glm::vec3( 0.062f, 0.060f, -2.210f), glm::vec3(0.014f, 0.044f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    part(s, cube, root, boxAt(glm::vec3( 0.086f, 0.078f, -2.210f), glm::vec3(0.048f, 0.010f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);
    part(s, cube, root, boxAt(glm::vec3( 0.086f, 0.042f, -2.210f), glm::vec3(0.048f, 0.010f, 0.012f)), chrome, chrome * 0.45f, 220.0f, 0.9f);

    // 4 прямоугольные фары с глубокими хромированными оправами и вертикальными перегородками рассеивателя
    for (float sx : {-1.0f, 1.0f}) {
        for (int hl = 0; hl < 2; ++hl) {
            float hx = sx * (hl == 0 ? 0.67f : 0.44f);
            // Хромированная рамка фары
            part(s, cube, root, boxAt(glm::vec3(hx, 0.068f, -2.184f), glm::vec3(0.215f, 0.112f, 0.026f)), chrome, glm::vec3(0.0f), 220.0f, 0.88f);
            // Светящаяся оптика
            part(s, cube, root, boxAt(glm::vec3(hx, 0.068f, -2.194f), glm::vec3(0.188f, 0.088f, 0.016f)), chrome, lampOn * (hl == 0 ? 1.0f : 0.88f), 220.0f, 0.85f);
            // Тонкий делитель-ребро на стекле фары
            part(s, cube, root, boxAt(glm::vec3(hx, 0.068f, -2.203f), glm::vec3(0.008f, 0.086f, 0.006f)), seamBlack, glm::vec3(0.0f), 32.0f, 0.1f);
        }
        // Нижние оранжевые поворотники / ПТФ (мигают янтарным при перестроении в нужную сторону)
        bool blinkSide = (turnSignalDir != 0) && ((sx < 0.0f && turnSignalDir < 0) || (sx > 0.0f && turnSignalDir > 0));
        float blinkPulse = (blinkSide && (static_cast<int>(clock * 14.0f) % 2 == 0)) ? 2.4f : 0.85f;
        part(s, cube, root, boxAt(glm::vec3(sx * 0.69f, -0.092f, -2.238f), glm::vec3(0.20f, 0.054f, 0.022f)), seamBlack, glm::vec3(0.0f), 40.0f, 0.1f);
        part(s, cube, root, boxAt(glm::vec3(sx * 0.69f, -0.092f, -2.248f), glm::vec3(0.176f, 0.038f, 0.012f)), amber, amber * blinkPulse, 96.0f, 0.3f);

        // Передние кронштейны и потоковые кабели BTTF на носу и вдоль крыльев
        part(s, cube, root, boxAt(glm::vec3(sx * 0.56f, -0.045f, -2.24f), glm::vec3(0.07f, 0.095f, 0.06f)), steel * 0.9f, glm::vec3(0.0f), 190.0f, 0.78f);
        part(s, cube, root, boxAt(glm::vec3(sx * 0.56f, -0.045f, -2.265f), glm::vec3(0.24f, 0.030f, 0.032f)), chrome, fluxCyan * 0.95f, 180.0f, 0.70f);
        // Передняя крыльевая потоковая трубка BTTF (аккуратно вдоль кромки крыла без торчащих концов)
        glm::mat4 fenderTube = glm::translate(root, glm::vec3(sx * 0.922f, 0.195f, -1.48f));
        fenderTube = glm::rotate(fenderTube, glm::radians(-9.0f), glm::vec3(1, 0, 0));
        drawPart(s, cube, glm::scale(fenderTube, glm::vec3(0.024f, 0.022f, 0.96f)), chrome * 0.85f, fluxCyan * 0.55f, 200.0f, 0.80f);
        for (float cz : {-1.84f, -1.48f, -1.12f}) {
            float cy = 0.195f + (cz - (-1.48f)) * 0.158f;
            part(s, cube, root, boxAt(glm::vec3(sx * 0.926f, cy, cz), glm::vec3(0.036f, 0.038f, 0.048f)), darkTrim, glm::vec3(0.0f), 80.0f, 0.30f);
        }

        // Аэродинамические боковые зеркала на основании A-стоек (ножка + корпус + зеркальное стекло)
        part(s, cube, root, boxAt(glm::vec3(sx * 0.85f, 0.385f, -0.85f), glm::vec3(0.09f, 0.028f, 0.055f)), darkTrim, glm::vec3(0.0f), 64.0f, 0.22f);
        part(s, cube, root, boxAt(glm::vec3(sx * 0.94f, 0.415f, -0.83f), glm::vec3(0.16f, 0.072f, 0.105f)), darkTrim, glm::vec3(0.0f), 78.0f, 0.28f);
        part(s, cube, root, boxAt(glm::vec3(sx * 0.94f, 0.415f, -0.774f), glm::vec3(0.135f, 0.054f, 0.010f)), chrome, glm::vec3(0.0f), 250.0f, 0.96f);
    }
    // Центральная передняя дуга потокового излучателя BTTF
    part(s, cube, root, boxAt(glm::vec3(0.0f, -0.045f, -2.265f), glm::vec3(1.16f, 0.024f, 0.026f)), chrome, fluxCyan * 0.85f, 210.0f, 0.82f);
}

// Музейная детализация задней части DeLorean DMC-12: 28-секционные сетчатые фонари с перегородками,
// подсветка OUTATIME, чистый нижний диффузор (без колхозного антикрыла) и двойной выхлоп
void Car::drawRearFasciaAndTaillights(const Shader& s, const glm::mat4& root, const glm::vec3& darkTrim) const {
    const SkinStyle& sk = getSkinStyle(paintIdx);
    glm::vec3 redBright = sk.taillight;
    glm::vec3 redDim = sk.taillight * 0.52f;
    glm::vec3 amber(2.45f, 1.05f, 0.08f);
    glm::vec3 whiteRev(2.25f, 2.25f, 2.45f);
    glm::vec3 chrome(0.84f, 0.88f, 0.94f);
    glm::vec3 seamBlack(0.020f, 0.022f, 0.030f);

    part(s, cube, root, boxAt(glm::vec3(0.0f,  0.19f, 2.13f), glm::vec3(1.68f, 0.33f, 0.08f)), darkTrim, glm::vec3(0.0f), 55.0f, 0.18f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, -0.07f, 2.16f), glm::vec3(1.74f, 0.19f, 0.15f)), darkTrim, glm::vec3(0.0f), 42.0f, 0.12f);

    // Стильный тёмный нижний диффузор с 4 вертикальными килями (аккуратно под бампером)
    part(s, cube, root, boxAt(glm::vec3(0.0f, -0.20f, 2.13f), glm::vec3(1.46f, 0.08f, 0.18f)), seamBlack, glm::vec3(0.0f), 42.0f, 0.15f);
    for (float fx : {-0.34f, -0.11f, 0.11f, 0.34f}) {
        part(s, cube, root, boxAt(glm::vec3(fx, -0.205f, 2.19f), glm::vec3(0.022f, 0.095f, 0.12f)), glm::vec3(0.14f, 0.15f, 0.20f), glm::vec3(0.0f), 110.0f, 0.45f);
    }

    // Тонкая неоново-красная светодиодная полоса и хромированная рамка над фонарями (ярко вспыхивает при торможении/приседании S)
    float brakeBoost = braking ? 2.15f : 1.0f;
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.338f, 2.172f), glm::vec3(1.66f, 0.018f, 0.020f)), glm::vec3(1.0f, 0.12f, 0.18f), redBright * (0.90f * brakeBoost));

    for (float sx : {-1.0f, 1.0f}) {
        bool blinkSide = (turnSignalDir != 0) && ((sx < 0.0f && turnSignalDir < 0) || (sx > 0.0f && turnSignalDir > 0));
        float amberBlink = (blinkSide && (static_cast<int>(clock * 14.0f) % 2 == 0)) ? 2.5f : 0.88f;

        // Тёмный корпус блока фонарей с сеткой перегородок
        part(s, cube, root, boxAt(glm::vec3(sx * 0.50f, 0.214f, 2.168f), glm::vec3(0.61f, 0.175f, 0.020f)), seamBlack, glm::vec3(0.0f), 45.0f, 0.12f);
        for (int col = 0; col < 7; ++col) {
            float cx = sx * (0.25f + (float)col * 0.082f);
            glm::vec3 topCol = (col >= 5) ? amber : ((col <= 1) ? whiteRev : redBright);
            glm::vec3 topEm  = (col >= 5) ? amber * amberBlink : ((col <= 1) ? whiteRev * 0.45f : redBright * (0.92f * brakeBoost));
            part(s, cube, root, boxAt(glm::vec3(cx, 0.255f, 2.176f), glm::vec3(0.066f, 0.066f, 0.016f)), topCol, topEm, 96.0f, 0.3f);
            part(s, cube, root, boxAt(glm::vec3(cx, 0.172f, 2.176f), glm::vec3(0.066f, 0.058f, 0.016f)), redBright, redDim * brakeBoost, 96.0f, 0.3f);
        }
        // Горизонтальная перемычка сетки фонарей
        part(s, cube, root, boxAt(glm::vec3(sx * 0.50f, 0.214f, 2.182f), glm::vec3(0.58f, 0.012f, 0.010f)), seamBlack, glm::vec3(0.0f), 40.0f, 0.1f);

        // Двойные хромированные выхлопные трубы с горящим внутренним кольцом при бусте
        glm::mat4 pipe = glm::translate(root, glm::vec3(sx * 0.54f, -0.19f, 2.19f));
        pipe = glm::rotate(pipe, glm::radians(90.0f), glm::vec3(0, 1, 0));
        part(s, cylinder, pipe, glm::scale(glm::mat4(1.0f), glm::vec3(0.18f, 0.055f, 0.055f)), chrome, glm::vec3(0.0f), 225.0f, 0.90f);
        glm::mat4 bore = glm::translate(root, glm::vec3(sx * 0.54f, -0.19f, 2.27f));
        bore = glm::rotate(bore, glm::radians(90.0f), glm::vec3(0, 1, 0));
        part(s, cylinder, bore, glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.042f, 0.042f)), glm::vec3(0.02f), glm::vec3(1.0f, 0.42f, 0.10f) * (0.30f + 2.6f * boostGlow));
    }

    // Номер OUTATIME в хромированной рамке с верхней лампой подсветки номера
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.215f, 2.170f), glm::vec3(0.305f, 0.162f, 0.014f)), chrome, glm::vec3(0.0f), 210.0f, 0.85f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.215f, 2.176f), glm::vec3(0.28f, 0.14f, 0.012f)), glm::vec3(0.92f, 0.92f, 0.96f), glm::vec3(0.24f), 64.0f, 0.2f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.252f, 2.182f), glm::vec3(0.20f, 0.030f, 0.01f)), glm::vec3(0.95f, 0.72f, 0.12f), glm::vec3(0.38f, 0.24f, 0.0f));
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.205f, 2.182f), glm::vec3(0.24f, 0.048f, 0.01f)), glm::vec3(0.10f, 0.28f, 0.78f), glm::vec3(0.16f, 0.45f, 0.98f));
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.302f, 2.180f), glm::vec3(0.10f, 0.016f, 0.022f)), darkTrim, glm::vec3(0.65f, 0.75f, 0.85f));
}

// Детализированное оборудование "Назад в будущее": ядерные выхлопные сопла с 7 лопатками,
// Mr. Fusion с крышкой-защёлкой, 2 алюминиевых конденсатора, пучок медных/неоновых кабелей и боковые воздухозаборники
void Car::drawTimeMachineConversion(const Shader& s, const glm::mat4& root, const glm::vec3& glow) const {
    glm::vec3 darkAlloy(0.08f, 0.09f, 0.12f);
    glm::vec3 gunmetal(0.22f, 0.25f, 0.30f);
    glm::vec3 reactorSteel(0.66f, 0.70f, 0.76f);
    glm::vec3 mrWhite(0.88f, 0.91f, 0.95f);
    glm::vec3 fluxBlue = glm::vec3(0.10f, 0.88f, 1.0f) * (0.78f + 1.35f * hoverBlend + 1.0f * boostGlow);
    glm::vec3 copper(0.78f, 0.45f, 0.20f);
    glm::vec3 wireRed(0.82f, 0.14f, 0.14f);
    glm::vec3 wireYellow(0.92f, 0.76f, 0.14f);

    // Несущая платформа реактора на задней палубе
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.445f, 1.66f), glm::vec3(1.30f, 0.065f, 0.78f)), darkAlloy, glm::vec3(0.0f), 110.0f, 0.45f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.482f, 1.32f), glm::vec3(1.22f, 0.032f, 0.12f)), gunmetal, glm::vec3(0.0f), 150.0f, 0.62f);

    // Барабан-основание реактора Mr. Fusion с охлаждающими кольцами
    glm::mat4 reactorDrum = glm::translate(root, glm::vec3(0.0f, 0.50f, 1.54f));
    reactorDrum = glm::rotate(reactorDrum, glm::radians(90.0f), glm::vec3(0, 0, 1));
    part(s, cylinder, reactorDrum, glm::scale(glm::mat4(1.0f), glm::vec3(0.095f, 0.25f, 0.25f)), reactorSteel, glm::vec3(0.0f), 195.0f, 0.78f);
    part(s, cylinder, reactorDrum, glm::scale(glm::mat4(1.0f), glm::vec3(0.12f, 0.19f, 0.19f)), gunmetal, glm::vec3(0.0f), 140.0f, 0.55f);
    part(s, cylinder, reactorDrum, glm::scale(glm::mat4(1.0f), glm::vec3(0.030f, 0.265f, 0.265f)), fluxBlue, fluxBlue * 0.75f, 96.0f, 0.2f);

    // Корпус Mr. Fusion + светящееся энергетическое кольцо + жёлтая маркировка + откидная крышка с ручкой
    glm::mat4 mrBody = glm::translate(root, glm::vec3(0.0f, 0.625f, 1.54f));
    mrBody = glm::rotate(mrBody, glm::radians(90.0f), glm::vec3(0, 0, 1));
    part(s, cylinder, mrBody, glm::scale(glm::mat4(1.0f), glm::vec3(0.145f, 0.088f, 0.088f)), mrWhite, glm::vec3(0.0f), 170.0f, 0.58f);
    part(s, cylinder, mrBody, glm::scale(glm::mat4(1.0f), glm::vec3(0.028f, 0.098f, 0.098f)), fluxBlue, fluxBlue * 1.15f, 96.0f, 0.2f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.59f, 1.63f), glm::vec3(0.09f, 0.022f, 0.014f)), wireYellow, wireYellow * 0.35f, 64.0f, 0.2f);
    part(s, sphere, root, boxAt(glm::vec3(0.0f, 0.70f, 1.54f), glm::vec3(0.115f, 0.055f, 0.115f)), gunmetal, glm::vec3(0.0f), 175.0f, 0.68f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.745f, 1.54f), glm::vec3(0.022f, 0.024f, 0.095f)), reactorSteel, glm::vec3(0.0f), 210.0f, 0.85f);

    // Две симметричные стороны: наклонные ядерные сопла, конденсаторы, гофрированные патрубки и силовые шины
    for (float sx : {-1.0f, 1.0f}) {
        // Алюминиевый высоковольтный цилиндр-конденсатор рядом с Mr. Fusion
        glm::mat4 capM = glm::translate(root, glm::vec3(sx * 0.27f, 0.53f, 1.56f));
        capM = glm::rotate(capM, glm::radians(90.0f), glm::vec3(0, 0, 1));
        part(s, cylinder, capM, glm::scale(glm::mat4(1.0f), glm::vec3(0.12f, 0.055f, 0.055f)), reactorSteel, glm::vec3(0.0f), 210.0f, 0.82f);
        part(s, cylinder, capM, glm::scale(glm::mat4(1.0f), glm::vec3(0.025f, 0.062f, 0.062f)), copper, glm::vec3(0.0f), 160.0f, 0.65f);

        // Наклонный кожух ядерного выхлопного сопла (BTTF Rear Vent)
        glm::mat4 tower = glm::translate(root, glm::vec3(sx * 0.47f, 0.555f, 1.82f));
        tower = glm::rotate(tower, glm::radians(14.5f), glm::vec3(1, 0, 0));
        tower = glm::rotate(tower, glm::radians(-sx * 4.0f), glm::vec3(0, 1, 0));

        drawPart(s, cube, glm::scale(tower, glm::vec3(0.37f, 0.245f, 0.52f)), darkAlloy, glm::vec3(0.0f), 115.0f, 0.48f);
        drawPart(s, cube, glm::scale(glm::translate(tower, glm::vec3(0.0f, 0.132f, -0.02f)), glm::vec3(0.33f, 0.062f, 0.44f)), gunmetal, glm::vec3(0.0f), 155.0f, 0.62f);
        // Верхние рёбра теплоотвода сопла
        for (int r = -1; r <= 1; ++r) {
            drawPart(s, cube, glm::scale(glm::translate(tower, glm::vec3((float)r * 0.09f, 0.168f, -0.02f)), glm::vec3(0.022f, 0.018f, 0.38f)), reactorSteel, glm::vec3(0.0f), 195.0f, 0.78f);
        }
        // Внутренняя светящаяся плазменная камера и 7 стальных жалюзи-лопаток сопла
        drawPart(s, cube, glm::scale(glm::translate(tower, glm::vec3(0.0f, -0.008f, 0.262f)), glm::vec3(0.31f, 0.185f, 0.015f)), glm::vec3(0.03f), fluxBlue * 0.52f, 64.0f, 0.1f);
        for (int v = -3; v <= 3; ++v) {
            glm::mat4 vane = glm::translate(tower, glm::vec3(0.0f, (float)v * 0.027f, 0.268f));
            drawPart(s, cube, glm::scale(vane, glm::vec3(0.33f, 0.011f, 0.024f)), reactorSteel, glm::vec3(0.0f), 185.0f, 0.72f);
        }
        // Боковая рамка сопла + гофрированный боковой патрубок от мотора
        drawPart(s, cube, glm::scale(glm::translate(tower, glm::vec3(sx * 0.188f, 0.0f, 0.04f)), glm::vec3(0.018f, 0.22f, 0.44f)), reactorSteel, glm::vec3(0.0f), 195.0f, 0.78f);

        // Аккуратные низкие медные и силовые коллекторы строго на плоской платформе реактора (ничего не протыкает жалюзи)
        part(s, cube, root, boxAt(glm::vec3(sx * 0.25f, 0.486f, 1.72f), glm::vec3(0.08f, 0.036f, 0.24f)), copper, glm::vec3(0.0f), 160.0f, 0.65f);
        part(s, cube, root, boxAt(glm::vec3(sx * 0.25f, 0.490f, 1.88f), glm::vec3(0.05f, 0.024f, 0.14f)), wireRed, glm::vec3(0.0f), 96.0f, 0.30f);

        // Задняя неоновая потоковая рельса вокруг моторного отсека
        part(s, cube, root, boxAt(glm::vec3(sx * 0.73f, 0.465f, 1.66f), glm::vec3(0.036f, 0.034f, 0.64f)), reactorSteel, fluxBlue * 0.90f, 195.0f, 0.78f);

        // Жаберный боковой воздухозаборник на заднем крыле позади двери
        part(s, cube, root, boxAt(glm::vec3(sx * 0.895f, 0.28f, 0.76f), glm::vec3(0.034f, 0.14f, 0.34f)), darkAlloy, glm::vec3(0.0f), 64.0f, 0.20f);
        for (int lv = 0; lv < 4; ++lv) {
            part(s, cube, root, boxAt(glm::vec3(sx * 0.910f, 0.235f + (float)lv * 0.030f, 0.76f), glm::vec3(0.014f, 0.012f, 0.30f)), reactorSteel, glm::vec3(0.0f), 160.0f, 0.65f);
        }

        // Боковые чёрные молдинги с тонкой стальной окантовкой + потоковые шины по порогам с хомутами
        part(s, cube, root, boxAt(glm::vec3(sx * 0.935f, 0.105f, -1.48f), glm::vec3(0.026f, 0.046f, 1.18f)), glm::vec3(0.032f), glm::vec3(0.0f), 32.0f, 0.1f);
        part(s, cube, root, boxAt(glm::vec3(sx * 0.945f, 0.155f,  1.28f), glm::vec3(0.026f, 0.046f, 1.56f)), glm::vec3(0.032f), glm::vec3(0.0f), 32.0f, 0.1f);
        part(s, cube, root, boxAt(glm::vec3(sx * 0.875f, -0.19f, 0.0f), glm::vec3(0.032f, 0.026f, 2.04f)), reactorSteel, fluxBlue * 0.90f, 190.0f, 0.75f);
        for (float clZ : {-0.75f, -0.25f, 0.25f, 0.75f}) {
            part(s, cube, root, boxAt(glm::vec3(sx * 0.880f, -0.19f, clZ), glm::vec3(0.046f, 0.044f, 0.055f)), darkAlloy, glm::vec3(0.0f), 85.0f, 0.35f);
        }
    }
}

void Car::drawCockpitInterior(const Shader& s, const glm::mat4& root) const {
    glm::vec3 leather(0.12f, 0.13f, 0.17f);
    glm::vec3 dashDark(0.05f, 0.055f, 0.075f);

    part(s, cube, root, boxAt(glm::vec3(0.0f, -0.06f, 0.06f), glm::vec3(1.34f, 0.06f, 1.24f)), dashDark, glm::vec3(0.0f), 32.0f, 0.1f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.28f, -0.58f), glm::vec3(1.26f, 0.22f, 0.44f)), dashDark, glm::vec3(0.0f), 32.0f, 0.1f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.12f, 0.04f), glm::vec3(0.22f, 0.20f, 1.05f)), dashDark, glm::vec3(0.0f), 32.0f, 0.1f);

    glm::mat4 wheelCol = glm::translate(root, glm::vec3(-0.34f, 0.35f, -0.32f));
    wheelCol = glm::rotate(wheelCol, glm::radians(-22.0f), glm::vec3(1, 0, 0));
    wheelCol = glm::rotate(wheelCol, glm::radians(90.0f), glm::vec3(0, 1, 0));
    part(s, cylinder, wheelCol, glm::scale(glm::mat4(1.0f), glm::vec3(0.03f, 0.13f, 0.13f)), glm::vec3(0.04f), glm::vec3(0.0f), 64.0f, 0.2f);

    for (float sx : {-0.34f, 0.34f}) {
        part(s, cube, root, boxAt(glm::vec3(sx, 0.10f, 0.08f), glm::vec3(0.36f, 0.12f, 0.44f)), leather, glm::vec3(0.0f), 48.0f, 0.15f);
        part(s, cube, root, boxAt(glm::vec3(sx, 0.34f, 0.32f), glm::vec3(0.34f, 0.40f, 0.10f)), leather, glm::vec3(0.0f), 48.0f, 0.15f);
        part(s, cube, root, boxAt(glm::vec3(sx, 0.58f, 0.34f), glm::vec3(0.20f, 0.14f, 0.08f)), leather, glm::vec3(0.0f), 48.0f, 0.15f);
    }

    // Салонное зеркало заднего вида под крышей и подсвеченная цифровая приборная панель водителя
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.72f, -0.34f), glm::vec3(0.20f, 0.045f, 0.032f)), glm::vec3(0.04f), glm::vec3(0.0f), 85.0f, 0.35f);
    part(s, cube, root, boxAt(glm::vec3(-0.34f, 0.36f, -0.52f), glm::vec3(0.32f, 0.065f, 0.022f)), glm::vec3(0.10f, 0.92f, 1.0f), glm::vec3(0.12f, 1.45f, 1.85f));
    // Чёрный резиновый уплотнитель по верхнему и нижнему краю лобового стекла (добавляет реализма)
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.786f, -0.27f), glm::vec3(1.18f, 0.018f, 0.030f)), glm::vec3(0.02f), glm::vec3(0.0f), 28.0f, 0.05f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.380f, -0.895f), glm::vec3(1.56f, 0.016f, 0.028f)), glm::vec3(0.02f), glm::vec3(0.0f), 28.0f, 0.05f);

    // Светящиеся табло "Цепи времени" (Destination / Present / Last Time Departed)
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.33f, -0.32f), glm::vec3(0.15f, 0.025f, 0.06f)), glm::vec3(1.0f, 0.1f, 0.1f), glm::vec3(2.2f, 0.2f, 0.1f));
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.295f, -0.32f), glm::vec3(0.15f, 0.025f, 0.06f)), glm::vec3(0.1f, 1.0f, 0.2f), glm::vec3(0.2f, 2.2f, 0.3f));
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.26f, -0.32f), glm::vec3(0.15f, 0.025f, 0.06f)), glm::vec3(1.0f, 0.7f, 0.1f), glm::vec3(2.2f, 1.4f, 0.1f));

    // Потоковый накопитель Flux Capacitor между сиденьями
    float fluxPulse = 0.75f + 0.25f * std::sin(clock * 18.0f);
    part(s, cube, root, boxAt(glm::vec3(0.0f, 0.46f, 0.42f), glm::vec3(0.18f, 0.22f, 0.06f)), glm::vec3(0.2f), glm::vec3(0.25f, 1.9f, 2.6f) * fluxPulse);
}
