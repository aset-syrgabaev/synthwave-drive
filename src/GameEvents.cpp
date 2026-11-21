#include "GameApp.h"

// ============================================================================
// ИГРОВОЙ ЦИКЛ, ШЕЙДЕРЫ И ЭКОНОМИКА РЕЖИМОВ (GameEvents.cpp) — КРИТЕРИИ R1, C2
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   - Конструктор GameApp компилирует все 9 пар GLSL-шейдеров один раз при запуске.
//   - resetGame() сбрасывает заезд: в режиме CLASSIC все заряды (NOS:0%, Q/X/Z/N/U/E/F:x0)
//     обнуляются, чтобы игрок зарабатывал их на трассе; в FREE PLAY всё доступно сразу.
//   - updatePlayingTrackAndEvents() связывает физику машины (CarPhysics), физику трассы
//     (TrackPhysics), спавн объектов (TrackSpawner), подбор монет и мини-босса Hunter.
// ============================================================================

// Конструктор GameApp вызывается один раз при запуске программы.
// Тут загружаются все 8 пар 3D-шейдеров, создаётся фреймбуфер пост-обработки и перемешиваются слова.
GameApp::GameApp(unsigned int w, unsigned int h)
    : scrWidth(w),
      scrHeight(h),
      camera(glm::vec3(0.0f, 3.1f, 7.4f)),
      skyShader("shaders/sky.vert", "shaders/sky.frag"),
      terrainShader("shaders/terrain.vert", "shaders/terrain.frag"),
      mountainShader("shaders/mountain.vert", "shaders/mountain.frag"),
      carShader("shaders/car.vert", "shaders/car.frag"),
      instancedShader("shaders/instanced.vert", "shaders/instanced.frag"),
      buildingShader("shaders/building.vert", "shaders/building.frag"),
      particleShader("shaders/particles.vert", "shaders/particles.frag"),
      postprocessShader("shaders/postprocess.vert", "shaders/postprocess.frag"),
      speedParticles(640),
      postProcess(w, h),
      themes(makeThemes()),
      wordRng(std::random_device{}())
{
    std::shuffle(wordList.begin(), wordList.end(), wordRng);
    camera.SetMode(ORBIT_CAM);
    delorean.doorsOpen = true;
}

// Добавляет очки стиля за трюки (ранги B -> A -> S -> AKIRA, дают +20% к очкам)
void GameApp::addStyle(int sp) {
    stylePoints = std::min(120, stylePoints + sp);
    styleDecayTimer = 5.0f;
}

// Начисляет очки с учётом множителя 2x, бонуса мотоцикла (+25%) и высокого ранга стиля (+20%)
int GameApp::awardPoints(int basePts) {
    int effMult = (scoreMultiplier > 1 || psychoTimer > 0.0f) ? 2 : 1;
    int pts = basePts * effMult;
    if (delorean.bikeMode) pts = std::max(1, (pts * 5 + 2) / 4);
    if (stylePoints >= 90) pts = std::max(1, (pts * 6 + 2) / 5);
    score += pts;
    return pts;
}

// Сбивает дрона-охотника (мини-босса), если игрок нажал EMP [X], Dash [N] или приземлил сальто [J]
bool GameApp::destroyHunter() {
    if (hunterTimer <= 0.0f) return false;
    hunterTimer = 0.0f;
    hunterPhase = 0.0f;
    int hPts = awardPoints(250);
    addStyle(28);
    shakeTimer = 0.25f;
    flashColor = glm::vec3(1.0f, 0.45f, 0.12f);
    flashTimer = 0.18f;
    bannerText = "HUNTER DOWN +" + std::to_string(hPts);
    bannerTimer = 1.6f;
    return true;
}

// Меняет один из 10 пунктов в интерактивной панели настроек [G] (вызывается стрелками или кликом мыши)
void GameApp::applySettingChange(int idx, int dir) {
    if (idx == 0)      gameMode = static_cast<GameMode>(((int)gameMode + dir + 2) % 2);
    else if (idx == 1) themeIdx = (themeIdx + dir + (int)themes.size()) % (int)themes.size();
    else if (idx == 2) delorean.toggleVehicleMode();
    else if (idx == 3) delorean.paintIdx = (delorean.paintIdx + dir + 8) % 8;
    else if (idx == 4) camera.SetMode(static_cast<CameraMode>(((int)camera.Mode + dir + 5) % 5));
    else if (idx == 5) delorean.headlights = !delorean.headlights;
    else if (idx == 6) postProcess.enabled = !postProcess.enabled;
    else if (idx == 7) { bloomLevel = (bloomLevel + dir + 3) % 3; postProcess.bloomStrength = bloomValues[bloomLevel]; }
    else if (idx == 8) speedLevel = (speedLevel + dir + 3) % 3;
    else if (idx == 9) delorean.toggleDoors();
}

// Полностью обнуляет заезд перед стартом или рестартом: очки, монеты, таймеры, заряды Classic и саму трассу
void GameApp::resetGame() {
    currentLane = 1; prevLane = 1; laneSwitchTimer = 0.0f;
    scrollDistance = 0.0f; baseSpeed = 14.5f;
    currentSpeed = baseSpeed * speedMultipliers[speedLevel];
    score = 0; scoreDistAccum = 0.0f; coins = 0;
    scoreMultiplier = 1; comboStreak = 0; nextFeverMark = 25;
    comboDecayTimer = 0.0f; timeWarpCharge = 0.0f;
    currentWordIdx = 0;
    std::shuffle(wordList.begin(), wordList.end(), wordRng);
    currentLetterProgress = 0;
    hoverTimer = 0.0f; hoverMaxTimer = delorean.bikeMode ? 12.0f : 18.0f; hoverCD = 0.0f;
    ringBoostTimer = magnetTimer = sneakersTimer = multiplierTimer = 0.0f;
    shieldTimer = 0.0f; shieldMaxTimer = delorean.bikeMode ? 14.0f : 24.0f; shieldCD = 0.0f;
    hunterTimer = hunterPhase = 0.0f; hunterLane = 1; nextHunterDist = 320.0f;
    hunterHitDone = false; onLrtDeck = false;
    qCharges = xCharges = zCharges = nCharges = uCharges = eCharges = fCharges = 0;
    nitroFuel = 0.0f; keyShiftWarned = false; flightRingsCollected = 0;
    sandevistanTimer = sandevistanCD = empCD = phaseTimer = phaseCD = 0.0f;
    dashTimer = dashCD = magOverdriveTimer = magOverdriveCD = psychoTimer = 0.0f;
    stuntScoreCD = 0.0f; stylePoints = 0; styleDecayTimer = 0.0f; wheelieAccum = 0.0f;
    stumbleAlertTimer = invulnerableTimer = shakeTimer = flashTimer = 0.0f;
    bannerTimer = 1.6f; bannerText = wordList[currentWordIdx];
    delorean.reset(); track.reset(0.0f); camera.SetMode(CHASE_CAM);
}

// Обработка таймеров способностей, скорости, физики трассы, бонус-коинов, букв и столкновений
void GameApp::updatePlayingTrackAndEvents(bool classicMode, bool duckInput, bool boostInput, bool allowAirJumps) {
    if (sandevistanTimer > 0.0f) sandevistanTimer = std::max(0.0f, sandevistanTimer - deltaTime);
    if (sandevistanCD > 0.0f)    sandevistanCD    = std::max(0.0f, sandevistanCD - deltaTime);
    if (empCD > 0.0f)            empCD            = std::max(0.0f, empCD - deltaTime);
    if (phaseTimer > 0.0f)       phaseTimer       = std::max(0.0f, phaseTimer - deltaTime);
    if (phaseCD > 0.0f)          phaseCD          = std::max(0.0f, phaseCD - deltaTime);
    if (ringBoostTimer > 0.0f)   ringBoostTimer   = std::max(0.0f, ringBoostTimer - deltaTime);
    if (dashTimer > 0.0f) {
        dashTimer = std::max(0.0f, dashTimer - deltaTime);
        int smashes = track.empPulse(scrollDistance + 4.0f, 12.0f);
        if (smashes > 0) awardPoints(smashes * 25);
    }
    if (dashCD > 0.0f)            dashCD            = std::max(0.0f, dashCD - deltaTime);
    if (magOverdriveTimer > 0.0f) magOverdriveTimer = std::max(0.0f, magOverdriveTimer - deltaTime);
    if (magOverdriveCD > 0.0f)    magOverdriveCD    = std::max(0.0f, magOverdriveCD - deltaTime);
    if (psychoTimer > 0.0f)       psychoTimer       = std::max(0.0f, psychoTimer - deltaTime);

    delorean.sandevistanActive     = (sandevistanTimer > 0.0f);
    delorean.phaseShiftActive      = (phaseTimer > 0.0f);
    delorean.nitroDashActive       = (dashTimer > 0.0f);
    delorean.magnetOverdriveActive = (magOverdriveTimer > 0.0f);
    delorean.psychoActive          = (psychoTimer > 0.0f);

    // Замедление времени Sandevistan [Q]: замедляем игровой шаг simDt до 38%, чтобы легче уворачиваться
    float timeScale = (sandevistanTimer > 0.0f) ? 0.38f : 1.0f;
    simDt = deltaTime * timeScale;
    sceneClock += simDt;

    // Если игрок удачно закончил сальто [J] и приземлился на колёса — даём очки стиля и откатываем кулдауны
    if (delorean.consumeTrickDone()) {
        addStyle(22);
        int flPts = awardPoints(55);
        // Бесплатное ускорение после сальто работает только в тестовом режиме Free Play
        if (!classicMode) ringBoostTimer = std::max(ringBoostTimer, 1.15f);
        sandevistanCD  = std::max(0.0f, sandevistanCD - 1.5f);
        empCD          = std::max(0.0f, empCD - 1.5f);
        phaseCD        = std::max(0.0f, phaseCD - 1.5f);
        dashCD         = std::max(0.0f, dashCD - 1.5f);
        if (!destroyHunter()) {
            flashColor  = glm::vec3(1.0f, 0.72f, 0.18f);
            flashTimer  = 0.14f;
            bannerText  = "STUNT LANDED +" + std::to_string(flPts);
            bannerTimer = 1.2f;
        }
    }

    // Отсчёт времени полёта [F]: если залетели в подземный туннель, полёт сразу выключается
    if (hoverTimer > 0.0f) {
        if (delorean.isInsideTunnel()) {
            hoverTimer = 0.0f;
            delorean.hover = false;
            hoverCD = (gameMode == MODE_FREE_PLAY) ? 2.0f : 20.0f;
        } else {
            hoverTimer -= deltaTime;
            delorean.hover = true;
            if (hoverTimer <= 0.0f) {
                delorean.hover = false;
                hoverCD = (gameMode == MODE_FREE_PLAY) ? 2.0f : 20.0f;
                track.endFlight();
            }
        }
    } else if (hoverCD > 0.0f) {
        hoverCD = std::max(0.0f, hoverCD - deltaTime);
    }

    // Таймеры пассивных бонусов с дороги: магнит, прыжковый бонус-коин (2x/3x), щит и множитель очков 2x
    if (magnetTimer > 0.0f) magnetTimer -= deltaTime;
    if (sneakersTimer > 0.0f) { sneakersTimer -= deltaTime; delorean.superJump = true; }
    else delorean.superJump = false;

    if (shieldTimer > 0.0f) {
        shieldTimer -= deltaTime;
        delorean.shieldActive = true;
        if (shieldTimer <= 0.0f) {
            shieldTimer = 0.0f;
            delorean.shieldActive = false;
            shieldCD = (gameMode == MODE_FREE_PLAY) ? 2.0f : 18.0f;
        }
    } else {
        delorean.shieldActive = false;
        if (shieldCD > 0.0f) shieldCD = std::max(0.0f, shieldCD - deltaTime);
    }

    if (multiplierTimer > 0.0f) { multiplierTimer -= deltaTime; scoreMultiplier = 2; }
    else scoreMultiplier = 1;

    // Мигание кузова во время кратковременной неуязвимости после срабатывания щита или зацепа
    if (invulnerableTimer > 0.0f) {
        invulnerableTimer -= deltaTime;
        delorean.visible = (static_cast<int>(invulnerableTimer * 18.0f) % 2 == 0);
    } else delorean.visible = true;

    if (bannerTimer > 0.0f) bannerTimer -= deltaTime;

    // Считаем итоговую скорость: растёт плавно с пройденной дистанцией + бонусы от полёта, нитро и рывка
    float difficulty = std::min(15.0f, scrollDistance * 0.00075f);
    float progressiveBase = (baseSpeed + difficulty * 0.72f) * speedMultipliers[speedLevel];
    float vehicleSpeedMult = delorean.bikeMode ? 1.12f : 1.00f;
    float targetSpeed = progressiveBase * vehicleSpeedMult;
    if (delorean.hover) targetSpeed *= (delorean.bikeMode ? 1.30f : 1.20f);
    if (psychoTimer > 0.0f) targetSpeed *= 1.28f;
    if (boostInput) targetSpeed *= 1.44f;
    if (ringBoostTimer > 0.0f) targetSpeed *= 1.26f;
    if (dashTimer > 0.0f) targetSpeed = progressiveBase * 1.98f;

    currentSpeed += (targetSpeed - currentSpeed) * 5.8f * deltaTime;
    float targetBoost = (boostInput || dashTimer > 0.0f || ringBoostTimer > 0.0f || psychoTimer > 0.0f) ? 1.0f : (delorean.hover ? 0.35f : 0.0f);
    boostFactor += (targetBoost - boostFactor) * 5.2f * deltaTime;
    delorean.boostGlow = boostFactor;

    // На мотоцикле езда на заднем колесе с зажатым нитро постепенно капает очки
    if (delorean.bikeMode && boostInput && delorean.grounded && !delorean.hover) {
        wheelieAccum += deltaTime;
        if (wheelieAccum >= 0.65f) { wheelieAccum -= 0.65f; awardPoints(5); }
    } else wheelieAccum = 0.0f;

    // Бонус 88 MPH Overdrive (в Classic набирается только при активном нитро или бусте)
    int mph = static_cast<int>(currentSpeed * 2.85f);
    bool hasBoostForWarp = (!classicMode) || boostInput || (ringBoostTimer > 0.0f) || (dashTimer > 0.0f) || (psychoTimer > 0.0f);
    if (mph >= 88 && hasBoostForWarp) {
        timeWarpCharge += deltaTime * 0.18f;
        if (timeWarpCharge >= 1.0f) {
            timeWarpCharge = 0.0f;
            int twPts = awardPoints(120);
            track.clearNearbyObstacles(scrollDistance, 20.0f);
            bannerText = "88 MPH +" + std::to_string(twPts);
            bannerTimer = 1.4f;
        }
    } else {
        timeWarpCharge = std::max(0.0f, timeWarpCharge - deltaTime * 0.20f);
    }

    // Берём следующую нужную букву слова и просим Track::ensure догенерировать трассу впереди игрока
    const std::string& activeWord = wordList[currentWordIdx];
    char neededChar = activeWord[std::clamp(currentLetterProgress, 0, (int)activeWord.size() - 1)];

    track.ensure(scrollDistance, delorean.hover, 4.53f, neededChar, difficulty, classicMode);

    // Проверяем все столкновения и опоры (дорога, поезда, потолок LRT, монеты, бонусы) в Track::update
    bool magnetActive = (magnetTimer > 0.0f) || (magOverdriveTimer > 0.0f) || (psychoTimer > 0.0f);
    bool invuln = (invulnerableTimer > 0.0f) || (dashTimer > 0.0f) || (psychoTimer > 0.0f);
    bool isSwitching = (laneSwitchTimer > 0.0f);
    bool smashActive = (psychoTimer > 0.0f) ||
                       (!classicMode && delorean.bikeMode && delorean.akiraSlideTimer > 0.0f) ||
                       (delorean.bikeMode && delorean.akiraSlideTimer > 0.0f && boostInput) ||
                       (!delorean.bikeMode && mph >= 88 && boostInput);
    bool barrelRolling = (delorean.barrelRollTimer > 0.0f);
    TrackEvents ev = track.update(scrollDistance, delorean.posX, delorean.posY, delorean.bottomWorldY(),
                                  duckInput, delorean.hover, invuln, magnetActive, isSwitching,
                                  sceneClock, simDt, delorean.phaseShiftActive, delorean.bikeMode,
                                  smashActive, barrelRolling);
    onLrtDeck = ev.onLrt;

    // Мини-босс дрон-охотник: прилетает каждые ~380м и стреляет по полосе
    if (scrollDistance >= nextHunterDist && hunterTimer <= 0.0f && !delorean.isInsideTunnel()) {
        hunterTimer = 12.0f; hunterPhase = 0.0f; hunterLane = currentLane; hunterHitDone = false;
        nextHunterDist = scrollDistance + 380.0f;
        bannerText = "HUNTER DRONE AMBUSH!"; bannerTimer = 1.6f;
    }
    if (hunterTimer > 0.0f) {
        if (delorean.isInsideTunnel()) {
            hunterTimer = 0.0f;
        } else {
            hunterTimer = std::max(0.0f, hunterTimer - deltaTime);
            hunterPhase += simDt;
            if (hunterPhase >= 1.35f && hunterPhase < 1.90f) {
                if (!hunterHitDone && std::fabs(delorean.posX - Track::laneX(hunterLane)) < 1.05f &&
                    delorean.bottomWorldY() < 3.3f && !delorean.hover && !invuln && !delorean.phaseShiftActive) {
                    hunterHitDone = true;
                    ev.crash = true;
                }
            } else if (hunterPhase >= 2.05f) {
                hunterPhase = 0.0f; hunterLane = currentLane; hunterHitDone = false;
            }
        }
    }

    float targetX = Track::laneX(currentLane);
    float roadNow  = Track::tunnelDepthAt(scrollDistance);
    float roadNext = Track::tunnelDepthAt(scrollDistance + 2.5f);
    delorean.update(targetX, currentSpeed, duckInput, ev.worldFloorY, ev.onRamp, ev.onTunnelRoof,
                    roadNow, roadNext, simDt, deltaTime, ev.worldCeilingY, ev.hasLateralClamp, ev.clampedPosX);
    scrollDistance += currentSpeed * simDt;

    scoreDistAccum += currentSpeed * simDt;
    if (scoreDistAccum >= 3.2f) { awardPoints(1); scoreDistAccum -= 3.2f; }

    if (ev.jumpPad) {
        delorean.jump(1.36f, allowAirJumps);
        addStyle(12);
        bannerText = "JUMP PAD +" + std::to_string(awardPoints(25)); bannerTimer = 1.1f;
    }
    if (ev.boostPad) {
        ringBoostTimer = 1.40f;
        nitroFuel = std::min(100.0f, nitroFuel + 25.0f);
        addStyle(12);
        bannerText = "NITRO PAD +" + std::to_string(awardPoints(25)); bannerTimer = 1.1f;
    }
    if (ev.akiraCapsule) {
        psychoTimer = 5.5f; addStyle(24);
        int akPts = awardPoints(80);
        flashColor = glm::vec3(1.0f, 0.16f, 0.24f); flashTimer = 0.16f; shakeTimer = 0.15f;
        bannerText = "AKIRA PSYCHO +" + std::to_string(akPts); bannerTimer = 1.4f;
    }
    if (ev.skyRing) {
        addStyle(16);
        int srPts = awardPoints(45);
        flightRingsCollected++;
        hoverTimer = std::min(hoverMaxTimer, hoverTimer + (classicMode ? 0.85f : 1.5f));
        ringBoostTimer = 1.20f;
        flashColor = glm::vec3(0.22f, 0.92f, 1.0f); flashTimer = 0.14f;
        bannerText = classicMode ? ("RING " + std::to_string(flightRingsCollected) + "/4 +" + std::to_string(srPts))
                                 : ("SKY RING +" + std::to_string(srPts));
        bannerTimer = 1.2f;
    }
    if (ev.smashedCount > 0) {
        addStyle(ev.smashedCount * 14);
        int smPts = awardPoints(ev.smashedCount * 40);
        shakeTimer = 0.15f;
        bannerText = (psychoTimer > 0.0f) ? ("PSYCHO SMASH +" + std::to_string(smPts))
                   : (delorean.hover ? ("DRONE SMASH +" + std::to_string(smPts))
                   : (delorean.bikeMode ? ("SLIDE SLASH +" + std::to_string(smPts))
                                        : ("88 MPH RAM +" + std::to_string(smPts))));
        bannerTimer = 1.1f;
    }
    if (ev.phasedCount > 0) {
        bannerText = "PHASE +" + std::to_string(awardPoints(ev.phasedCount * 25)); bannerTimer = 1.0f;
    }
    if (ev.coins > 0) {
        coins += ev.coins; comboStreak += ev.coins; comboDecayTimer = 3.5f;
        int comboBonus = std::min(3, 1 + comboStreak / 15);
        int stormMult = (magOverdriveTimer > 0.0f) ? 2 : 1;
        awardPoints(ev.coins * 5 * comboBonus * stormMult);
        if (comboStreak >= nextFeverMark) {
            nextFeverMark += 30;
            magnetTimer = std::max(magnetTimer, delorean.bikeMode ? 5.0f : 7.5f);
            bannerText = "COIN FEVER +" + std::to_string(awardPoints(80)); bannerTimer = 1.3f;
        }
    }
    if (ev.nearMiss) { addStyle(15); bannerText = "NEAR MISS +" + std::to_string(awardPoints(35)); bannerTimer = 1.0f; }

    if (ev.mysteryBox) {
        int prize = rand() % 4;
        if (prize == 0) { coins += 15; bannerText = "CASE +" + std::to_string(awardPoints(120)); }
        else if (prize == 1) { shieldMaxTimer = delorean.bikeMode ? 14.0f : 24.0f; shieldTimer = shieldMaxTimer; bannerText = "CASE: SHIELD"; }
        else if (prize == 2) { magnetTimer = delorean.bikeMode ? 12.0f : 18.0f; sneakersTimer = 15.0f; bannerText = "CASE: MAGNET"; }
        else if (delorean.isInsideTunnel()) { shieldMaxTimer = delorean.bikeMode ? 14.0f : 24.0f; shieldTimer = shieldMaxTimer; awardPoints(100); bannerText = "CASE: SHIELD"; }
        else if (classicMode) { fCharges = std::min(2, fCharges + 1); bannerText = "CASE: +1 FLIGHT [F]"; }
        else {
            delorean.hover = true; hoverMaxTimer = delorean.bikeMode ? 12.0f : 16.0f; hoverTimer = hoverMaxTimer;
            flightRingsCollected = 0; track.startFlight(scrollDistance, classicMode); bannerText = "CASE: FLIGHT";
        }
        bannerTimer = 1.5f;
    }

    if (ev.collectedLetter != 0 && ev.collectedLetter == neededChar) {
        currentLetterProgress++; awardPoints(35);
        bannerText = std::string("+ ") + ev.collectedLetter; bannerTimer = 1.0f;
        if (currentLetterProgress >= (int)activeWord.size()) {
            int wPts = awardPoints(250);
            if (classicMode) { fCharges = std::min(2, fCharges + 1); eCharges = std::min(3, eCharges + 1); }
            else if (!delorean.isInsideTunnel()) {
                delorean.hover = true; hoverMaxTimer = delorean.bikeMode ? 12.0f : 16.0f; hoverTimer = hoverMaxTimer;
                flightRingsCollected = 0; track.startFlight(scrollDistance, classicMode);
            } else { shieldMaxTimer = delorean.bikeMode ? 14.0f : 20.0f; shieldTimer = shieldMaxTimer; }
            bannerText = activeWord + " +" + std::to_string(wPts); bannerTimer = 2.0f;
            currentWordIdx = (currentWordIdx + 1) % (int)wordList.size();
            if (currentWordIdx == 0) std::shuffle(wordList.begin(), wordList.end(), wordRng);
            currentLetterProgress = 0;
        }
    }

    // Разбираем все собранные за кадр бонусы и выдаём заряды / таймеры
    if (ev.shieldOrb) {
        if (classicMode) { eCharges = std::min(3, eCharges + 1); bannerText = "+1 SHIELD [E]"; }
        else { shieldMaxTimer = delorean.bikeMode ? 14.0f : 20.0f; shieldTimer = shieldMaxTimer; bannerText = "SHIELD ON"; }
        bannerTimer = 1.3f;
    }
    if (ev.techTokenType >= 0) {
        if (ev.techTokenType == 0)      { qCharges = std::min(3, qCharges + 1); bannerText = "+1 SLOW-MO [Q]"; }
        else if (ev.techTokenType == 1) { xCharges = std::min(3, xCharges + 1); bannerText = "+1 EMP [X]"; }
        else if (ev.techTokenType == 2) { zCharges = std::min(3, zCharges + 1); bannerText = "+1 PHASE [Z]"; }
        else if (ev.techTokenType == 3) { nCharges = std::min(3, nCharges + 1); bannerText = "+1 DASH [N]"; }
        else                            { uCharges = std::min(3, uCharges + 1); bannerText = "+1 MAGNET [U]"; }
        flashColor = glm::vec3(0.24f, 0.92f, 1.0f); flashTimer = 0.12f; bannerTimer = 1.3f;
    }
    if (ev.flightOrb) {
        if (classicMode) { fCharges = std::min(2, fCharges + 1); bannerText = "+1 FLIGHT [F]"; }
        else if (!delorean.isInsideTunnel()) {
            delorean.hover = true; hoverMaxTimer = delorean.bikeMode ? 12.0f : 16.0f; hoverTimer = hoverMaxTimer;
            flightRingsCollected = 0; track.startFlight(scrollDistance, classicMode);
            bannerText = delorean.bikeMode ? "AKIRA JET 12S" : "CRUISER FLIGHT";
        } else { bannerText = "+" + std::to_string(awardPoints(75)) + " BONUS"; }
        bannerTimer = 1.3f;
    }
    // Нитро-коин даёт +50% топлива для ускорения SHIFT в режиме Classic
    if (ev.nitroOrb) {
        nitroFuel = std::min(100.0f, nitroFuel + 50.0f);
        flashColor = glm::vec3(1.0f, 0.48f, 0.10f); flashTimer = 0.12f;
        bannerText = "+50% NITRO [SHIFT]"; bannerTimer = 1.3f;
    }
    if (ev.magnetOrb)     { magnetTimer = delorean.bikeMode ? 12.0f : 17.0f; bannerText = "MAGNET"; bannerTimer = 1.2f; }
    // Зелёный коин прыжка открывает двойной/тройной прыжок на 18 секунд в режиме Classic
    if (ev.sneakersOrb)   { sneakersTimer = 18.0f; bannerText = delorean.bikeMode ? "3X JUMP 18S" : "2X JUMP 18S"; bannerTimer = 1.3f; }
    if (ev.multiplierOrb) { multiplierTimer = 15.0f; bannerText = "2X SCORE"; bannerTimer = 1.2f; }

    // Боковой зацеп (stumble): в первый раз отбрасывает на прошлую полосу и прилетает полиция; второй зацеп — авария
    if (ev.stumble) {
        if (gameMode == MODE_FREE_PLAY) shakeTimer = 0.12f;
        else if (stumbleAlertTimer > 0.0f) ev.crash = true;
        else {
            currentLane = prevLane; stumbleAlertTimer = 4.5f; shakeTimer = 0.22f; invulnerableTimer = 0.55f;
            bannerText = "POLICE BEHIND"; bannerTimer = 1.3f;
        }
    }
    // Лобовая авария: в Free Play просто расчищаем путь, в Classic спасает активный щит, иначе Game Over
    if (ev.crash) {
        if (gameMode == MODE_FREE_PLAY) { track.clearNearbyObstacles(scrollDistance, 22.0f); shakeTimer = 0.14f; }
        else if (shieldTimer > 0.0f) {
            shieldTimer = 0.0f; shieldCD = 18.0f; delorean.shieldActive = false; stumbleAlertTimer = 0.0f;
            track.clearNearbyObstacles(scrollDistance, 30.0f); shakeTimer = 0.28f; invulnerableTimer = 1.5f;
            bannerText = "SHIELD SAVED"; bannerTimer = 1.5f;
        } else {
            shakeTimer = 0.0f; flashColor = glm::vec3(0.95f, 0.12f, 0.20f); flashTimer = 0.20f;
            sandevistanTimer = phaseTimer = dashTimer = magOverdriveTimer = psychoTimer = hunterTimer = 0.0f;
            delorean.sandevistanActive = delorean.phaseShiftActive = delorean.nitroDashActive = false;
            delorean.magnetOverdriveActive = delorean.psychoActive = false;
            state = STATE_GAMEOVER; delorean.visible = true;
            if (score > highScore) highScore = score;
        }
    }
}
