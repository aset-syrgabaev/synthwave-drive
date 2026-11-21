#include "Car.h"

// ============================================================================
// ФИЗИКА ТРАНСПОРТА, 8 СКИНОВ И КОРНЕВАЯ МАТРИЦА (CarPhysics.cpp) — R3, C2
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   - getSkinStyle(): палитры 8 освещённо-реактивных скинов (меняют корпус + фары + шлейф).
//   - jump(): одиночный, 2x (DeLorean) и 3x (Akira) прыжок; в Classic 2x/3x только с коином!
//   - computeRootMatrix(): строит корневую матрицу M_root = T * R_yaw * R_roll * R_pitch
//     и автоматически приподнимает кузов на lift, чтобы шины НИКОГДА не проваливались под пол!
//   - update(): гравитация (-34 м/с²), твёрдый потолок ЛРТ, пружинная подвеска и наклон.
// ============================================================================

// Возвращает полную цветовую схему выбранного скина (0..7)
const SkinStyle& Car::getSkinStyle(int idx) {
    static const SkinStyle kSkins[8] = {
        // 0: ORIGINAL (Сталь 304 / Красный Акира, тёплые фары, красные стопы)
        {glm::vec3(0.58f, 0.61f, 0.67f), glm::vec3(0.96f, 0.10f, 0.08f),
         glm::vec3(3.0f, 2.8f, 2.2f),    glm::vec3(2.9f, 0.08f, 0.12f),
         glm::vec3(1.00f, 0.08f, 0.12f), glm::vec3(0.10f, 0.88f, 1.00f),
         glm::vec3(0.12f, 0.88f, 1.00f), 0.0f},
        // 1: STEALTH INVIS (невидимый оптический камуфляж, призрачно-ледяные огни)
        {glm::vec3(0.35f, 0.82f, 0.95f), glm::vec3(0.35f, 0.82f, 0.95f),
         glm::vec3(1.6f, 2.6f, 3.2f),    glm::vec3(0.35f, 2.2f, 2.8f),
         glm::vec3(0.30f, 0.85f, 1.00f), glm::vec3(0.28f, 0.88f, 1.00f),
         glm::vec3(0.35f, 0.88f, 1.00f), 0.45f},
        // 2: CYBER MATRIX (прозрачная зелёная матричная голограмма, изумрудные фары и шлейф)
        {glm::vec3(0.12f, 0.96f, 0.44f), glm::vec3(0.12f, 0.96f, 0.44f),
         glm::vec3(0.35f, 3.2f, 0.85f),  glm::vec3(0.18f, 3.0f, 0.58f),
         glm::vec3(0.12f, 1.00f, 0.42f), glm::vec3(0.14f, 0.98f, 0.48f),
         glm::vec3(0.12f, 1.00f, 0.45f), 1.0f},
        // 3: INFERNO CARBON (чёрный карбон с лавовыми трещинами, огненные янтарные фары и шлейф)
        {glm::vec3(0.09f, 0.09f, 0.11f), glm::vec3(0.09f, 0.09f, 0.11f),
         glm::vec3(3.2f, 1.45f, 0.18f),  glm::vec3(3.2f, 0.55f, 0.04f),
         glm::vec3(1.00f, 0.42f, 0.04f), glm::vec3(1.00f, 0.36f, 0.05f),
         glm::vec3(1.00f, 0.44f, 0.06f), 1.1f},
        // 4: HYPER CHAMELEON (переливающийся фиолетово-бирюзовый хром, неоново-фиолетовые огни)
        {glm::vec3(0.38f, 0.20f, 0.78f), glm::vec3(0.38f, 0.20f, 0.78f),
         glm::vec3(2.2f, 0.85f, 3.2f),   glm::vec3(3.0f, 0.14f, 2.2f),
         glm::vec3(0.95f, 0.16f, 0.92f), glm::vec3(0.86f, 0.22f, 1.00f),
         glm::vec3(0.86f, 0.22f, 1.00f), 1.0f},
        // 5: TOXIC BIOHAZARD (матовая броня с кислотными жилами, токсично-салатовые фары и шлейф)
        {glm::vec3(0.07f, 0.09f, 0.06f), glm::vec3(0.07f, 0.09f, 0.06f),
         glm::vec3(1.6f, 3.2f, 0.18f),   glm::vec3(0.45f, 3.0f, 0.12f),
         glm::vec3(0.32f, 1.00f, 0.08f), glm::vec3(0.30f, 1.00f, 0.10f),
         glm::vec3(0.32f, 1.00f, 0.10f), 1.1f},
        // 6: ROYAL 24K GOLD (жидкое зеркальное золото, золотистые фары и шлейф)
        {glm::vec3(0.96f, 0.76f, 0.20f), glm::vec3(0.96f, 0.76f, 0.20f),
         glm::vec3(3.2f, 2.65f, 1.15f),  glm::vec3(3.1f, 1.95f, 0.18f),
         glm::vec3(1.00f, 0.76f, 0.14f), glm::vec3(1.00f, 0.74f, 0.16f),
         glm::vec3(1.00f, 0.78f, 0.16f), 0.85f},
        // 7: ARCTIC FROST (кристаллический ледяной хром, морозно-голубые фары и крио-шлейф)
        {glm::vec3(0.55f, 0.84f, 0.98f), glm::vec3(0.55f, 0.84f, 0.98f),
         glm::vec3(1.1f, 2.6f, 3.4f),    glm::vec3(0.45f, 2.4f, 3.2f),
         glm::vec3(0.35f, 0.88f, 1.00f), glm::vec3(0.38f, 0.90f, 1.00f),
         glm::vec3(0.38f, 0.86f, 1.00f), 1.0f}
    };
    return kSkins[std::clamp(idx, 0, 7)];
}

// Цвет луча фар на асфальте подстраивается под текущий скин
glm::vec3 Car::getHeadlightBeamColor() const {
    const SkinStyle& sk = getSkinStyle(paintIdx);
    return glm::clamp(sk.headlight * 0.34f, glm::vec3(0.15f), glm::vec3(1.0f));
}

Car::Car()
    : cube(Primitives::cube()),
      cylinder(Primitives::cylinder(48)),
      sphere(Primitives::sphere(20, 28)),
      shieldSphere(Primitives::sphere(28, 40)),
      bodyMesh(CarModel::body()),
      cabinGlassMesh(CarModel::cabinGlass()),
      cabinFrameMesh(CarModel::cabinFrame()),
      doorMetalL(CarModel::gullwingDoorMetal(-1.0f)),
      doorMetalR(CarModel::gullwingDoorMetal( 1.0f)),
      doorGlassL(CarModel::gullwingDoorGlass(-1.0f)),
      doorGlassR(CarModel::gullwingDoorGlass( 1.0f)),
      doorTrimL(CarModel::gullwingDoorTrim(-1.0f)),
      doorTrimR(CarModel::gullwingDoorTrim( 1.0f)),
      akiraWingLMesh(AkiraGeo::frontWingSide(-1.0f)),
      akiraWingRMesh(AkiraGeo::frontWingSide( 1.0f)),
      akiraNoseBridgeMesh(AkiraGeo::frontNoseBridge()),
      akiraLowerChassisMesh(AkiraGeo::lowerChassis()),
      akiraRearPodMesh(AkiraGeo::rearSeatCowl()),
      akiraWindscreenMesh(AkiraGeo::windscreen()),
      akiraTireFrontMesh(AkiraGeo::tire(0.42f, 0.26f, 0.32f)),
      akiraTireRearMesh(AkiraGeo::tire(0.45f, 0.27f, 0.46f)),
      akiraHubFrontMesh(AkiraGeo::hubcapFront(0.31f, 0.042f)),
      akiraHubRearMesh(AkiraGeo::hubcapRear(0.24f, 0.055f)),
      akiraDecalDisc(AkiraGeo::flatDisc()),
      akiraDecalStar(AkiraGeo::flatStar()),
      akiraDecalTri(AkiraGeo::flatTriangle()),
      akiraTextCitizen(AkiraGeo::flatText("CITIZEN")),
      akiraTextCanon(AkiraGeo::flatText("CANON")),
      akiraTextShoei(AkiraGeo::flatText("SHOEI")),
      akiraPillMesh(AkiraGeo::pillHalf(0.026f, 0.030f)),
      trailRibbonMesh(CarModel::lightTrailRibbon()),
      clock(0.0f) {}

// Прыжок с земли; двойной (DeLorean) и тройной (мотоцикл) прыжки в воздухе работают только
// если allowAirJumps == true (в Free Play всегда, а в Classic только пока активен бонус прыжка с дороги).
// Возвращает: 1..3 (номер прыжка), 0 (уже исчерпано) или -1 (воздушный прыжок закрыт в Classic).
int Car::jump(float launchBoost, bool allowAirJumps) {
    if (hover) {
        if (barrelRollTimer <= 0.0f) {
            barrelRollTimer = bikeMode ? 0.36f : 0.46f;
            return 3; // код воздушной бочки
        }
        return 0;
    }
    int maxJumps = allowAirJumps ? (bikeMode ? 3 : 2) : 1;
    if (grounded || launchBoost > 1.2f) {
        velY = (superJump ? 14.2f : 11.8f) * launchBoost;
        grounded = false;
        jumpCount = 1;
        doubleJumpUsed = !allowAirJumps;
        suspensionVel = 2.6f * launchBoost;
        return 1;
    } else if (jumpCount < maxJumps) {
        velY = superJump ? 13.4f : 11.45f; // двойной прыжок достаёт эстакаду LRT (3.85м), тройной — верхний мост (5.75м)
        jumpCount++;
        doubleJumpUsed = (jumpCount >= maxJumps);
        doubleJumpFlash = 0.34f;
        suspensionVel = 2.0f;
        return jumpCount; // 2 = DOUBLE JUMP, 3 = TRIPLE JUMP
    } else if (!allowAirJumps && jumpCount == 1) {
        return -1; // подсказка для Classic: нужен бонус-коин прыжка с дороги
    }
    return 0;
}

// Старт воздушного трюка [J]: возможен только в прыжке (не в полёте)
bool Car::startTrick() {
    if (grounded || hover || trickTimer > 0.0f) return false;
    trickTimer = 0.72f;
    return true;
}
bool Car::consumeTrickDone() {
    bool d = trickDone;
    trickDone = false;
    return d;
}

void Car::triggerEmpWave() {
    empWaveRadius = 1.2f;
    empWaveAlpha = 1.0f;
}

void Car::fastDrop() {
    if (!grounded && !hover && velY > -18.0f) {
        velY = -26.0f;
    }
}

// Открывает или закрывает двери "крыло чайки" на DeLorean (клавиша O)
void Car::toggleDoors() {
    doorsOpen = !doorsOpen;
}

// Переключает транспорт между машиной DeLorean и мотоциклом (клавиша K)
void Car::toggleVehicleMode() {
    bikeMode = !bikeMode;
    morphFlash = 0.55f;
    akiraSlideTimer = 0.0f;
    suspensionVel = 2.4f;
}

// Культовый боковой занос с искрами AKIRA SLIDE (1988)
void Car::triggerAkiraSlide(int dir) {
    if (!bikeMode || hover) return;
    akiraSlideDir = (dir >= 0) ? 1 : -1;
    akiraSlideTimer = 0.72f;
}


// Возвращает транспорт в начальную точку при старте нового заезда или рестарте
void Car::reset() {
    posX = 0.0f;
    posY = kGround;
    roadY = 0.0f;
    roadSlopePitch = 0.0f;
    velY = 0.0f;
    duckAmount = 0.0f;
    suspensionOffset = 0.0f;
    suspensionVel = 0.0f;
    yawSlip = 0.0f;
    empWaveRadius = 0.0f;
    empWaveAlpha = 0.0f;
    doubleJumpFlash = 0.0f;
    morphFlash = 0.0f;
    akiraSlideTimer = 0.0f;
    barrelRollTimer = 0.0f;
    barrelRollAngle = 0.0f;
    trickTimer = 0.0f;
    trickAngle = 0.0f;
    trickDone = false;
    psychoActive = false;
    jumpCount = 0;
    turnSignalDir = 0;
    nitroDashActive = false;
    magnetOverdriveActive = false;
    braking = false;
    sandevistanActive = false;
    phaseShiftActive = false;
    doubleJumpUsed = false;
    grounded = true;
    hover = false;
    prevHover = false;
    doorsOpen = false;
    shieldActive = false;
    superJump = false;
    onTrainRoof = false;
    onTunnelRoof = false;
    hoverBlend = 0.0f;
    doorAngle = 0.0f;
    steerAngle = 0.0f;
    tilt = 0.0f;
    pitch = 0.0f;
    boostGlow = 0.0f;
}

// Обновляет физику транспорта за один кадр: плавное перестроение, гравитацию, прыжки,
// посадку на эстакады и крыши поездов, наклон кузова, подвеску и вращение колёс.
// Собирает матрицу транспорта с автоматическим подъёмом при крене/wheelie,
// чтобы нижняя точка шин НИКОГДА не проваливалась под асфальт, крышу поезда или эстакаду LRT
glm::mat4 Car::computeRootMatrix() const {
    glm::mat4 rot(1.0f);
    rot = glm::rotate(rot, glm::radians(yawSlip), glm::vec3(0, 1, 0));
    rot = glm::rotate(rot, glm::radians(tilt + barrelRollAngle), glm::vec3(0, 0, 1));
    rot = glm::rotate(rot, glm::radians(pitch + trickAngle), glm::vec3(1, 0, 0));

    float lift = 0.0f;
    if (!hover && trickTimer <= 0.0f) {
        float minY = 0.0f;
        if (bikeMode) {
            const glm::vec3 pts[4] = {
                glm::vec3(-0.16f, -0.538f, -1.30f), glm::vec3(0.16f, -0.538f, -1.30f),
                glm::vec3(-0.23f, -0.538f,  1.26f), glm::vec3(0.23f, -0.538f,  1.26f)
            };
            for (const auto& p : pts) minY = std::min(minY, (rot * glm::vec4(p, 1.0f)).y);
        } else {
            const glm::vec3 pts[4] = {
                glm::vec3(-0.88f, -0.535f, -1.30f), glm::vec3(0.88f, -0.535f, -1.30f),
                glm::vec3(-0.88f, -0.535f,  1.30f), glm::vec3(0.88f, -0.535f,  1.30f)
            };
            for (const auto& p : pts) minY = std::min(minY, (rot * glm::vec4(p, 1.0f)).y);
        }
        if (minY < -0.538f) lift = -0.538f - minY;
    }
    // Когда колёса стоят на опоре, отрицательный прогиб подвески не уводит шины ниже пола
    float safeSusp = grounded ? std::max(0.0f, suspensionOffset) : suspensionOffset;
    glm::mat4 root = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY + safeSusp + lift, 0.0f));
    return root * rot;
}

void Car::update(float targetX, float speed, bool duckInput, float worldFloorY, bool onRamp, bool tunnelRoofFlag,
                 float currentRoadY, float nextRoadY, float dt, float realDt,
                 float worldCeilingY, bool hasLateralClamp, float clampedPosX) {
    clock += dt;
    roadY = currentRoadY;
    onTunnelRoof = tunnelRoofFlag;
    onTrainRoof = (!tunnelRoofFlag && (worldFloorY - currentRoadY) > 1.4f);

    if (empWaveAlpha > 0.0f) {
        empWaveRadius += 58.0f * realDt;
        empWaveAlpha = std::max(0.0f, empWaveAlpha - 1.55f * realDt);
    }
    if (doubleJumpFlash > 0.0f) {
        doubleJumpFlash = std::max(0.0f, doubleJumpFlash - realDt);
    }
    if (morphFlash > 0.0f) {
        morphFlash = std::max(0.0f, morphFlash - realDt);
    }
    if (akiraSlideTimer > 0.0f) {
        akiraSlideTimer = std::max(0.0f, akiraSlideTimer - realDt);
    }
    if (barrelRollTimer > 0.0f) {
        float maxRoll = bikeMode ? 0.36f : 0.46f;
        barrelRollTimer = std::max(0.0f, barrelRollTimer - realDt);
        float u = 1.0f - (barrelRollTimer / maxRoll);
        barrelRollAngle = u * 360.0f;
    } else {
        barrelRollAngle = 0.0f;
    }
    // Воздушный трюк: полный оборот назад; если приземлились раньше — трюк отменяется без очков
    if (trickTimer > 0.0f) {
        if (grounded || hover) {
            trickTimer = 0.0f;
            trickAngle = 0.0f;
        } else {
            trickTimer = std::max(0.0f, trickTimer - realDt);
            float tu = 1.0f - trickTimer / 0.72f;
            trickAngle = -360.0f * (tu * tu * (3.0f - 2.0f * tu));
            if (trickTimer <= 0.0f) {
                trickAngle = 0.0f;
                trickDone = true;
            }
        }
    }
    braking = duckInput;

    if (hover && isInsideTunnel()) {
        hover = false;
    }

    float baseSlope = onRamp ? 15.5f : (tunnelRoofFlag ? 0.0f : glm::degrees(std::atan2(nextRoadY - currentRoadY, 2.5f)));
    roadSlopePitch += (baseSlope - roadSlopePitch) * std::min(1.0f, 12.0f * realDt);

    float prevX = posX;
    // Привилегия мотоцикла Акиры: на 40% более резкое и отзывчивое перестроение
    float stiffness = bikeMode ? 24.0f : (hover ? 14.0f : 18.0f);
    posX += (targetX - posX) * (1.0f - std::exp(-stiffness * realDt));
    if (hasLateralClamp) {
        posX = clampedPosX;
    }
    lateralVel = realDt > 0.0f ? (posX - prevX) / realDt : 0.0f;

    // Мотоцикл Акиры глубоко закладывается в поворотах, а при AKIRA SLIDE плавно разворачивается боком ±46°
    float maxTilt = bikeMode ? 30.0f : 13.0f;
    float tiltGain = bikeMode ? 2.55f : 1.40f;
    float targetTilt = (akiraSlideTimer > 0.0f) ? (-akiraSlideDir * 28.0f) : std::clamp(-lateralVel * tiltGain, -maxTilt, maxTilt);
    float targetYaw  = (akiraSlideTimer > 0.0f) ? ( akiraSlideDir * 46.0f) : std::clamp(-lateralVel * 1.05f, -9.0f, 9.0f);
    tilt += (targetTilt - tilt) * std::min(1.0f, 12.0f * realDt);
    yawSlip += (targetYaw - yawSlip) * std::min(1.0f, 13.0f * realDt);
    steerAngle += (std::clamp(lateralVel * 3.4f, -28.0f, 28.0f) - steerAngle) * std::min(1.0f, 16.0f * realDt);

    if (duckInput && !grounded && !hover) {
        fastDrop();
    }

    float targetDuck = (duckInput && grounded && !hover) ? 1.0f : 0.0f;
    duckAmount += (targetDuck - duckAmount) * std::min(1.0f, 16.0f * realDt);

    if (!hover && prevHover) {
        grounded = false;
        velY = std::min(velY, -1.5f);
    }
    prevHover = hover;

    hoverBlend += ((hover ? 1.0f : 0.0f) - hoverBlend) * (1.0f - std::exp(-5.0f * realDt));

    // Колёса всегда остаются на поверхности дороги (приседание больше не топит шины в асфальт)
    float baseClearance = kGround;

    if (hover) {
        // В полёте клавиша S позволяет пикировать ниже (нырнуть под дрона / пролететь над поездами)
        float bob = 0.16f * std::sin(clock * 3.6f);
        float diveOffset = duckInput ? -1.25f : 0.0f;
        float targetFlightY = kGround + 3.85f + bob + diveOffset;
        posY += (targetFlightY - posY) * (1.0f - std::exp(-6.0f * realDt));
        velY = 0.0f;
        grounded = false;
    } else {
        float floorTargetY = worldFloorY + baseClearance;
        if (grounded) {
            if (floorTargetY >= posY - 0.55f) {
                posY = floorTargetY;
                velY = 0.0f;
            } else {
                grounded = false;
                velY = 0.0f;
            }
        }

        if (!grounded) {
            velY -= 34.0f * dt;
            posY += velY * dt;

            // Твёрдый потолок туннеля и твёрдая нижняя плита эстакады LRT / высотного моста:
            // пролететь снизу сквозь бетонное перекрытие невозможно
            float vehH = duckInput ? 0.72f : (bikeMode ? 1.15f : 1.24f);
            float maxCeilPosY = worldCeilingY - vehH + kGround;
            if (posY > maxCeilPosY) {
                posY = maxCeilPosY;
                velY = std::min(velY, -2.0f);
            }
            if (currentRoadY < -3.0f && !tunnelRoofFlag && posY > -1.85f) {
                posY = -1.85f;
                velY = std::min(velY, 0.0f);
            }

            if (posY <= floorTargetY) {
                float impact = std::clamp(-velY * 0.20f, 0.0f, 3.4f);
                suspensionVel -= impact;
                posY = floorTargetY;
                velY = 0.0f;
                grounded = true;
                jumpCount = 0;
                doubleJumpUsed = false;
            }
        }
    }

    float roadMicroBounce = grounded ? (std::sin(clock * speed * 1.3f) * 0.004f) : 0.0f;
    float springForce = -140.0f * (suspensionOffset - roadMicroBounce) - 14.5f * suspensionVel;
    suspensionVel += springForce * realDt;
    suspensionOffset += suspensionVel * realDt;
    suspensionOffset = std::clamp(suspensionOffset, -0.18f, 0.14f);

    // Мотоцикл встаёт на дыбы (Wheelie +13.5°) при ускорении SHIFT на асфальте
    float boostSquat = (bikeMode && grounded ? boostGlow * 13.5f : boostGlow * 2.4f) * (1.0f - hoverBlend);
    float targetPitch = hover ? 1.0f : (roadSlopePitch + boostSquat + duckAmount * -3.6f + std::clamp(velY * 0.85f, -10.0f, 10.0f));
    pitch += (targetPitch - pitch) * std::min(1.0f, 11.0f * realDt);

    float doorGoal = (doorsOpen || hover) ? 56.0f : 0.0f;
    doorAngle += (doorGoal - doorAngle) * std::min(1.0f, 7.5f * realDt);

    wheelRot += speed * 65.0f * dt;
}
