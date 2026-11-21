#include "Car.h"

// ============================================================================
// ОСНОВНОЙ РЕНДЕР ТРАНСПОРТА, 8 СКИНОВ И ПРОЗРАЧНЫЕ ЭФФЕКТЫ (CarRenderEffects.cpp)
// ВАЖНО ДЛЯ ЗАЩИТЫ (критерии R3, R4, B2):
//   Рендер разделён на 2 прохода для правильной работы буфера глубины (Z-buffer):
//     1) draw() — сначала рисуются все НЕПРОЗРАЧНЫЕ детали кузова и колёс;
//     2) drawTranslucentEffects() — затем включается альфа-смешивание (GL_BLEND)
//        и отключается запись в глубину (glDepthMask(GL_FALSE)) для стёкол, невидимого
//        скина, световых шлейфов фар, энергетического щита и волны EMP.
// ============================================================================

// Отрисовка машины или мотоцикла с учётом выбранного скина (paintIdx 0..7)
void Car::draw(const Shader& s, const glm::vec3& glow) const {
    if (!visible) return;

    // 8 освещённо-реактивных скинов (клавиша B, меняют и материал в car.frag, и цвет фар!):
    // 0: ORIGINAL (сталь 304 / красный Акира), 1: STEALTH INVIS (оптический камуфляж),
    // 2: CYBER MATRIX (зелёная матрица), 3: INFERNO CARBON (карбон + лава),
    // 4: HYPER CHAMELEON (хамелеон), 5: TOXIC BIOHAZARD, 6: ROYAL 24K GOLD, 7: ARCTIC FROST.
    int pi = std::clamp(paintIdx, 0, 7);
    const SkinStyle& sk = getSkinStyle(pi);
    bool isStealthInvis = (pi == 1);
    bool isHoloSkin     = (pi == 2);
    glm::vec3 bodyCol   = sk.carBody;
    glm::vec3 underCol  = sk.accent;
    float bodyRefl      = (pi == 6 || pi == 4 || pi == 7) ? 0.95f : ((pi == 5) ? 0.56f : 0.86f);
    float bodyShin      = (pi == 5) ? 140.0f : 240.0f;

    glm::vec3 darkTrim(0.050f, 0.053f, 0.066f);
    glm::vec3 tintedGlass(0.025f, 0.045f, 0.095f);

    glm::mat4 root = computeRootMatrix();

    // Передаём номер скина в шейдер car.frag
    s.setInt("skinMode", pi);

    if (!phaseShiftActive && !isStealthInvis && !isHoloSkin) {
        if (bikeMode) {
            float akiraRefl = (pi == 0) ? 0.82f : bodyRefl;
            drawAkiraBike(s, root, sk.bikeBody, underCol, darkTrim, 240.0f, akiraRefl);
        } else {
            drawPart(s, bodyMesh, root, bodyCol, glm::vec3(0.0f), bodyShin, bodyRefl);
            drawPart(s, cabinGlassMesh, root, tintedGlass, glm::vec3(0.01f, 0.02f, 0.05f), 252.0f, 0.95f);
            drawPart(s, cabinFrameMesh, root, bodyCol, glm::vec3(0.0f), bodyShin, bodyRefl);

            drawCockpitInterior(s, root);
            drawGullwingDoors(s, root, bodyCol, tintedGlass, darkTrim);
            drawLouversAndRearDeck(s, root);
            drawFrontFasciaAndHoodDetails(s, root, darkTrim, bodyCol);
            drawRearFasciaAndTaillights(s, root, darkTrim);
            drawTimeMachineConversion(s, root, underCol);
            drawWheelsAndThrusters(s, root, underCol);

            float ugPulse = 0.85f + 0.15f * std::sin(clock * 4.0f);
            part(s, cube, root, boxAt(glm::vec3(0.0f, -0.255f, 0.0f), glm::vec3(1.32f, 0.016f, 3.25f)),
                 underCol, underCol * (1.25f * ugPulse), 64.0f, 0.1f);
        }
    } else if (!phaseShiftActive && (isStealthInvis || isHoloSkin)) {
        // В невидимом (1) и матричном (2) скине оставляем яркие фары и габариты в цвет скина
        glm::vec3 lampWarm = headlights ? sk.headlight : glm::vec3(0.15f);
        glm::vec3 tailGlow = sk.taillight;
        if (bikeMode) {
            part(s, cube, root, boxAt(glm::vec3(0.0f, 0.312f, -1.56f), glm::vec3(0.17f, 0.042f, 0.02f)), glm::vec3(1.0f), lampWarm);
            for (float sd : {-0.078f, 0.078f}) {
                part(s, cube, root, boxAt(glm::vec3(sd, 0.50f, 1.15f), glm::vec3(0.085f, 0.024f, 0.02f)), tailGlow, tailGlow);
            }
        } else {
            for (float sx : {-0.62f, 0.62f}) {
                part(s, cube, root, boxAt(glm::vec3(sx, 0.07f, -2.18f), glm::vec3(0.28f, 0.08f, 0.02f)), glm::vec3(1.0f), lampWarm);
                part(s, cube, root, boxAt(glm::vec3(sx, 0.24f,  2.16f), glm::vec3(0.42f, 0.09f, 0.02f)), tailGlow, tailGlow);
            }
        }
    }
    s.setInt("skinMode", 0);

    // Плазменный импульс под днищем при Двойном прыжке в воздухе
    if (doubleJumpFlash > 0.01f) {
        float t = doubleJumpFlash / 0.34f;
        glm::vec3 burstCol(0.14f, 0.95f, 1.0f);
        if (bikeMode) {
            for (float sz : {-1.25f, 1.25f}) {
                part(s, sphere, root, boxAt(glm::vec3(0.0f, -0.45f - (1.0f - t) * 0.35f, sz), glm::vec3(0.32f * t, 0.45f * t, 0.32f * t)),
                     burstCol, burstCol * (3.0f * t), 32.0f, 0.0f);
            }
        } else {
            for (float sx : {-0.72f, 0.72f}) {
                for (float sz : {-1.10f, 1.10f}) {
                    part(s, sphere, root, boxAt(glm::vec3(sx, -0.45f - (1.0f - t) * 0.35f, sz), glm::vec3(0.28f * t, 0.42f * t, 0.28f * t)),
                         burstCol, burstCol * (3.0f * t), 32.0f, 0.0f);
                }
            }
        }
    }

    // Электрические дуги Flux Capacitor (только у DeLorean; все световые шлейфы рисуются объёмно в drawTranslucentEffects)
    if (!bikeMode && (boostGlow > 0.25f || hoverBlend > 0.25f) && !phaseShiftActive) {
        float arcPulse = 0.5f + 0.5f * std::sin(clock * 55.0f);
        float strength = std::max(boostGlow, hoverBlend) * arcPulse;
        glm::vec3 arcCyan(0.12f, 0.94f, 1.0f);
        part(s, cube, root, boxAt(glm::vec3(0.0f, 0.07f, -2.26f), glm::vec3(1.44f, 0.022f, 0.022f)),
             arcCyan, arcCyan * (2.2f * strength));
        part(s, cube, root, boxAt(glm::vec3(0.0f, 0.54f, 1.86f), glm::vec3(1.10f, 0.022f, 0.022f)),
             arcCyan, arcCyan * (2.2f * strength));
    }
}

// Все полупрозрачные и аддитивные эффекты:
// 1) Кристально чистое гнутое стекло ветровика Акиры (без сетки)
// 2) Реалистичные объёмные световые шлейфы фар (Akira Laser Trails & 88 MPH Plasma Ribbons — НИКАКИХ прямоугольников)
// 3) Щит [E], Phase Shift [Z], Morph [K], Shock-Dash [N], Coin Storm [U], Sandevistan [Q], EMP [X]
// Рисует все полупрозрачные эффекты отдельным проходом после непрозрачных деталей:
// невидимый скин, голограмму, стекло ветровика, световые шлейфы, щит и волну EMP.
void Car::drawTranslucentEffects(const Shader& s) const {
    if (!visible) return;
    int pi = std::clamp(paintIdx, 0, 7);
    bool isStealthInvis = (pi == 1);
    bool isHoloSkin     = (pi == 2);
    bool hasTrails = !phaseShiftActive && (bikeMode || boostGlow > 0.10f || hoverBlend > 0.15f);
    bool needBlend = hasTrails || (bikeMode && !phaseShiftActive) || isStealthInvis || isHoloSkin ||
                     shieldActive || (empWaveAlpha > 0.01f) ||
                     sandevistanActive || phaseShiftActive || (morphFlash > 0.01f) ||
                     nitroDashActive || magnetOverdriveActive || psychoActive;
    if (!needBlend) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    const SkinStyle& sk = getSkinStyle(pi);
    glm::mat4 baseRoot = computeRootMatrix();

    // 0) Невидимый скин (оптический камуфляж pi == 1) или Матричная голограмма (pi == 2)
    if (!phaseShiftActive && (isStealthInvis || isHoloSkin)) {
        s.setMat4("model", baseRoot);
        s.setVec3("objectColor", isHoloSkin ? sk.accent : glm::vec3(0.45f, 0.85f, 1.0f));
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shieldAlpha", isStealthInvis ? -1.10f : (0.86f + 0.12f * std::sin(clock * 11.0f)));
        if (bikeMode) {
            akiraWingLMesh.draw();
            akiraWingRMesh.draw();
            akiraNoseBridgeMesh.draw();
            akiraLowerChassisMesh.draw();
            akiraRearPodMesh.draw();
            akiraWindscreenMesh.draw();
            for (int i = 0; i < 2; ++i) {
                glm::mat4 wm = glm::translate(baseRoot, glm::vec3(0.0f, (i == 0 ? -0.10f : -0.07f), (i == 0 ? -1.30f : 1.26f)));
                s.setMat4("model", wm);
                if (i == 0) akiraTireFrontMesh.draw();
                else        akiraTireRearMesh.draw();
            }
        } else {
            bodyMesh.draw();
            cabinFrameMesh.draw();
            cabinGlassMesh.draw();
        }
    }

    // 0.25) Прозрачное гнутое стекло ветровика Акиры (для непрозрачных скинов)
    if (bikeMode && !phaseShiftActive && !isStealthInvis && !isHoloSkin) {
        s.setMat4("model", baseRoot);
        s.setVec3("objectColor", glm::vec3(0.75f, 0.88f, 0.98f));
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shieldAlpha", -0.60f);
        akiraWindscreenMesh.draw();
    }

    // 0.5) Объёмные световые шлейфы (аддитивное свечение с плавным гауссовым затуханием)
    if (hasTrails) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        auto drawRibbon = [&](const glm::vec3& origin, float width, float length, const glm::vec3& col, float intensity, float yawOff = 0.0f) {
            glm::mat4 rm = glm::translate(baseRoot, origin);
            if (yawOff != 0.0f) rm = glm::rotate(rm, glm::radians(yawOff), glm::vec3(0, 1, 0));
            rm = glm::scale(rm, glm::vec3(width, width, length));
            s.setMat4("model", rm);
            s.setVec3("objectColor", col);
            s.setVec3("emissiveColor", glm::vec3(0.0f));
            s.setFloat("shieldAlpha", -2.0f - std::clamp(intensity, 0.1f, 1.8f));
            trailRibbonMesh.draw();
        };

        if (bikeMode) {
            // двойные красные объёмные световые ленты Акиры с динамическим изгибом в повороте
            float len = 3.2f + boostGlow * 9.5f + hoverBlend * 2.8f;
            float inten = 0.62f + boostGlow * 1.05f;
            float trailSweep = (akiraSlideTimer > 0.0f) ? (-(float)akiraSlideDir * 32.0f) : std::clamp(lateralVel * 1.1f, -12.0f, 12.0f);
            glm::vec3 trailCol = sk.trail;
            drawRibbon(glm::vec3(-0.072f, 0.455f, 1.20f), 0.105f + boostGlow * 0.045f, len, trailCol, inten, trailSweep);
            drawRibbon(glm::vec3( 0.072f, 0.455f, 1.20f), 0.105f + boostGlow * 0.045f, len, trailCol, inten, trailSweep);

            // Если в полёте на Акире — мягкие реактивные плазменные потоки из-под колёс-турбин
            if (hoverBlend > 0.2f) {
                glm::vec3 jetBlue(0.18f, 0.85f, 1.0f);
                drawRibbon(glm::vec3(0.0f, -0.22f, 1.35f), 0.22f, 3.2f + boostGlow * 6.0f, jetBlue, hoverBlend * (0.65f + boostGlow * 0.7f), trailSweep * 0.6f);
            }

            // Мягкие бело-золотые шлейфы искр от шин во время AKIRA SLIDE
            if (akiraSlideTimer > 0.01f) {
                float st = std::min(1.0f, akiraSlideTimer * 2.4f);
                float sideYaw = -(float)akiraSlideDir * 36.0f;
                drawRibbon(glm::vec3(0.0f, -0.44f,  1.25f), 0.24f, 4.8f, glm::vec3(1.0f, 0.78f, 0.32f), st * 1.15f, sideYaw);
                drawRibbon(glm::vec3(0.0f, -0.44f, -1.25f), 0.20f, 3.8f, glm::vec3(0.88f, 0.92f, 1.0f), st * 0.95f, sideYaw);
            }
        } else {
            // DeLorean DMC-12: объёмные огненные шлейфы 88 MPH на асфальте И плазменные форсажи в полёте
            float trailSweep = std::clamp(lateralVel * 0.9f, -10.0f, 10.0f);
            if (hoverBlend > 0.25f) {
                float fLen = 2.8f + boostGlow * 6.8f;
                float fInt = hoverBlend * (0.45f + boostGlow * 0.95f);
                glm::vec3 hoverCol = sk.accent;
                for (float sx : {-1.02f, 1.02f}) {
                    drawRibbon(glm::vec3(sx, -0.22f, 1.45f), 0.22f, fLen, hoverCol, fInt, trailSweep);
                }
                if (boostGlow > 0.15f) {
                    for (float sx : {-0.46f, 0.46f}) {
                        drawRibbon(glm::vec3(sx, 0.54f, 2.08f), 0.24f, 3.5f + boostGlow * 7.2f, sk.trail, boostGlow * 1.15f, trailSweep);
                    }
                }
            } else if (boostGlow > 0.10f) {
                float fLen = 3.2f + boostGlow * 7.5f;
                glm::vec3 tireTrailCol = (pi == 0) ? glm::vec3(1.0f, 0.42f, 0.08f) : sk.trail;
                for (float sx : {-0.86f, 0.86f}) {
                    drawRibbon(glm::vec3(sx, -0.42f, 1.55f), 0.24f, fLen, tireTrailCol, boostGlow * 1.25f, trailSweep);
                }
                for (float sx : {-0.54f, 0.54f}) {
                    drawRibbon(glm::vec3(sx, -0.19f, 2.26f), 0.14f, 1.8f + boostGlow * 4.0f, sk.accent, boostGlow * 0.95f, trailSweep);
                }
            }
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // 1) Призрачный фазовый сдвиг [Z] (использует реальные 3D-меши Акиры или DeLorean)
    if (phaseShiftActive) {
        float pPulse = 0.85f + 0.15f * std::sin(clock * 14.0f);
        s.setMat4("model", baseRoot);
        s.setVec3("objectColor", glm::vec3(0.66f, 0.30f, 1.00f));
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shieldAlpha", 0.92f * pPulse);
        if (bikeMode) {
            akiraWingLMesh.draw();
            akiraWingRMesh.draw();
            akiraNoseBridgeMesh.draw();
            akiraLowerChassisMesh.draw();
            akiraRearPodMesh.draw();
            akiraWindscreenMesh.draw();
        } else {
            bodyMesh.draw();
            cabinFrameMesh.draw();
            cabinGlassMesh.draw();
        }
    }

    // 2) Золотое поле Супер-Магнита [U] (притягивает все монеты и бонусы со всех 3 полос)
    if (magnetOverdriveActive) {
        float mPulse = 0.86f + 0.14f * std::sin(clock * 9.0f);
        glm::mat4 magM = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY + 0.15f, 0.0f));
        s.setMat4("model", glm::scale(magM, glm::vec3(2.05f * mPulse, 0.38f, 2.85f * mPulse)));
        s.setVec3("objectColor", glm::vec3(1.00f, 0.82f, 0.18f));
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shieldAlpha", 0.72f);
        shieldSphere.draw();
    }

    // 3) Энергетический купол тарана Nitro Shock-Dash [N]
    if (nitroDashActive) {
        float dPulse = 0.85f + 0.15f * std::sin(clock * 28.0f);
        glm::mat4 ramM = glm::translate(baseRoot, glm::vec3(0.0f, 0.18f, -0.55f));
        s.setMat4("model", glm::scale(ramM, glm::vec3(bikeMode ? 0.95f : 1.55f, 1.05f, 2.85f)));
        s.setVec3("objectColor", glm::vec3(1.0f, 0.56f, 0.14f));
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shieldAlpha", 0.95f * dPulse);
        shieldSphere.draw();
    }

    // 4) Сфера трансформации Машина <-> Мотоцикл Акиры [K]
    if (morphFlash > 0.01f) {
        float t = morphFlash / 0.55f;
        float r = 1.2f + (1.0f - t) * 2.8f;
        glm::mat4 mm = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY + 0.25f, 0.0f));
        s.setMat4("model", glm::scale(mm, glm::vec3(r, r * 0.72f, r * 1.45f)));
        s.setVec3("objectColor", bikeMode ? glm::vec3(1.0f, 0.18f, 0.22f) : glm::vec3(0.42f, 0.72f, 1.00f));
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shieldAlpha", t * 0.95f);
        shieldSphere.draw();
    }

    // 5) Компактный защитный щит [E] в цвет выбранного скина (не задевает соседние вагоны)
    if (shieldActive) {
        glm::mat4 shRoot = glm::translate(baseRoot, glm::vec3(0.0f, 0.16f, 0.0f));
        float pulse = 0.88f + 0.12f * std::sin(clock * 6.5f);

        s.setMat4("model", glm::scale(shRoot, glm::vec3(bikeMode ? 0.72f : 1.24f, 0.96f, bikeMode ? 2.02f : 2.48f)));
        s.setVec3("objectColor", sk.shield);
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shininess", 128.0f);
        s.setFloat("reflectivity", 0.0f);
        s.setFloat("shieldAlpha", 0.82f * pulse);
        shieldSphere.draw();
    }

    // 6) Призрачные силуэты Sandevistan [Q] (реальная форма Акиры или DeLorean)
    if (sandevistanActive) {
        for (int ghost = 1; ghost <= 3; ++ghost) {
            float gz = (float)ghost * 1.15f;
            glm::mat4 gm = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY + suspensionOffset, gz));
            gm = glm::rotate(gm, glm::radians(tilt * 0.7f), glm::vec3(0, 0, 1));
            glm::vec3 ghostCol = (ghost == 1) ? glm::vec3(0.10f, 0.96f, 0.88f)
                               : ((ghost == 2) ? glm::vec3(0.25f, 0.75f, 1.0f) : glm::vec3(1.0f, 0.65f, 0.12f));
            s.setMat4("model", gm);
            s.setVec3("objectColor", ghostCol);
            s.setVec3("emissiveColor", glm::vec3(0.0f));
            s.setFloat("shieldAlpha", 0.55f / (float)ghost);
            if (bikeMode) {
                akiraWingLMesh.draw();
                akiraWingRMesh.draw();
                akiraLowerChassisMesh.draw();
                akiraRearPodMesh.draw();
            } else {
                bodyMesh.draw();
            }
        }
    }

    // 7) Расширяющаяся 3D-ударная волна EMP [X]
    if (empWaveAlpha > 0.01f) {
        glm::mat4 empM = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY + 0.35f, 0.0f));
        empM = glm::scale(empM, glm::vec3(empWaveRadius, empWaveRadius * 0.45f, empWaveRadius));
        s.setMat4("model", empM);
        s.setVec3("objectColor", glm::vec3(0.18f, 0.92f, 1.0f));
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shieldAlpha", empWaveAlpha * 0.85f);
        shieldSphere.draw();
    }

    // 8) Аура режима ускорения от капсулы (компактный облегающий контур)
    if (psychoActive) {
        float pp = 0.92f + 0.08f * std::sin(clock * 17.0f);
        glm::mat4 am = glm::translate(baseRoot, glm::vec3(0.0f, 0.18f, 0.0f));
        s.setMat4("model", glm::scale(am, glm::vec3(bikeMode ? 0.78f : 1.30f, 1.02f, bikeMode ? 2.12f : 2.58f) * pp));
        s.setVec3("objectColor", glm::vec3(1.0f, 0.12f, 0.18f));
        s.setVec3("emissiveColor", glm::vec3(0.0f));
        s.setFloat("shieldAlpha", 0.76f);
        shieldSphere.draw();
    }

    s.setFloat("shieldAlpha", 0.0f);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
