#include "GameApp.h"

// Этот файл обрабатывает все нажатия клавиш на клавиатуре.
// Везде есть флаги вроде keyTPressed — это защита от дребезга, чтобы за одно нажатие
// клавиша сработала ровно один раз, а не 60 раз в секунду.

void GameApp::handleGlobalKeys(GLFWwindow* window) {
    // G — открывает или закрывает меню настроек (если открыли настройки, справку H прячем)
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (!keyGPressed) {
            showSettings = !showSettings;
            if (showSettings) showHelpOverlay = false;
            keyGPressed = true;
        }
    } else keyGPressed = false;

    // H или F1 — показывает подсказку по всем кнопкам управления прямо на экране
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
        if (!keyHPressed) {
            showHelpOverlay = !showHelpOverlay;
            if (showHelpOverlay) showSettings = false;
            keyHPressed = true;
        }
    } else keyHPressed = false;

    // ESC — по очереди закрывает менюшки, ставит паузу, возвращает в главное меню или закрывает окно
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (!keyEscPressed) {
            if (showSettings) showSettings = false;
            else if (showHelpOverlay) showHelpOverlay = false;
            else if (state == STATE_PLAYING) state = STATE_PAUSED;
            else if (state == STATE_PAUSED) state = STATE_PLAYING;
            else if (state == STATE_GAMEOVER) {
                state = STATE_MENU;
                camera.SetMode(ORBIT_CAM);
                delorean.doorsOpen = true;
            } else {
                glfwSetWindowShouldClose(window, true);
            }
            keyEscPressed = true;
        }
    } else keyEscPressed = false;

    // Пока открыта панель настроек G, стрелки листают 10 пунктов и меняют значения влево/вправо
    if (showSettings) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            if (!keyUpPressed) { settingsCursor = (settingsCursor + 9) % 10; keyUpPressed = true; }
        } else keyUpPressed = false;

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (!keyDownPressed) { settingsCursor = (settingsCursor + 1) % 10; keyDownPressed = true; }
        } else keyDownPressed = false;

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            if (!keyLeftPressed) { applySettingChange(settingsCursor, -1); keyLeftPressed = true; }
        } else keyLeftPressed = false;

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!keyRightPressed) { applySettingChange(settingsCursor, +1); keyRightPressed = true; }
        } else keyRightPressed = false;
        return;
    }

    // TAB — быстро переключает режим игры между CLASSIC и FREE PLAY
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!keyTabPressed) {
            gameMode = static_cast<GameMode>(((int)gameMode + 1) % 2);
            bannerText = modeShort[(int)gameMode];
            bannerTimer = 1.4f;
            keyTabPressed = true;
        }
    } else keyTabPressed = false;

    // C — включает и выключает ретро-фильтр старого монитора (CRT сканлайны + искривление)
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!keyCPressed) { postProcess.enabled = !postProcess.enabled; keyCPressed = true; }
    } else keyCPressed = false;

    // T — листает по кругу все 8 локаций (Night City, Las Vegas, Neo-Tokyo, Arctic, Space, Sahara, Jungle, Mars)
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (!keyTPressed) {
            themeIdx = (themeIdx + 1) % (int)themes.size();
            bannerText = themes[themeIdx].name;
            bannerTimer = 1.6f;
            keyTPressed = true;
        }
    } else keyTPressed = false;

    // K — превращает машину DeLorean в мотоцикл Акиры и обратно
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (!keyKPressed) {
            delorean.toggleVehicleMode();
            bannerText = delorean.bikeMode
                       ? ((gameMode == MODE_CLASSIC) ? "AKIRA BIKE" : "AKIRA: 3X JUMP / SLIDE")
                       : ((gameMode == MODE_CLASSIC) ? "DELOREAN DMC-12" : "DELOREAN: 88 RAM / FLY");
            bannerTimer = 1.6f;
            keyKPressed = true;
        }
    } else keyKPressed = false;

    // B — переключает 8 скинов (меняется не только кузов, но и цвет фар, шлейфов и щита)
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (!keyBPressed) {
            delorean.paintIdx = (delorean.paintIdx + 1) % 8;
            bannerText = paintNames[delorean.paintIdx];
            bannerTimer = 1.3f;
            keyBPressed = true;
        }
    } else keyBPressed = false;

    // L — включает и выключает фары (это динамический свет для критерия R4)
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!keyLPressed) { delorean.headlights = !delorean.headlights; keyLPressed = true; }
    } else keyLPressed = false;

    // O — открывает и закрывает двери крыло чайки у DeLorean (иерархия parent-child, R3)
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        if (!keyOPressed) { delorean.toggleDoors(); keyOPressed = true; }
    } else keyOPressed = false;

    // В главном меню цифры 1 и 2 выбирают режим игры, а во время езды 1..5 выбирают камеру
    if (state == STATE_MENU) {
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) gameMode = MODE_CLASSIC;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) gameMode = MODE_FREE_PLAY;
    } else {
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) camera.SetMode(CHASE_CAM);
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) camera.SetMode(ACTION_CAM);
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) camera.SetMode(HOOD_CAM);
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) camera.SetMode(ORBIT_CAM);
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) camera.SetMode(AKIRA_CAM);
    }

    // V — просто листает 5 камер по очереди
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        if (!keyVPressed) {
            int nextMode = (camera.Mode + 1) % 5;
            camera.SetMode(static_cast<CameraMode>(nextMode));
            bannerText = camNames[nextMode];
            bannerTimer = 1.1f;
            keyVPressed = true;
        }
    } else keyVPressed = false;
}

// Логика главного меню: ждём Space/Enter для старта и плавно крутим орбитальную камеру вокруг машины
void GameApp::updateMenu(GLFWwindow* window) {
    bool overlayOpen = showHelpOverlay || showSettings;
    if (!overlayOpen && (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)) {
        if (!keyEnterPressed && !keyJumpPressed) {
            resetGame();
            state = STATE_PLAYING;
            keyEnterPressed = true;
            keyJumpPressed = true;
        }
    } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        keyEnterPressed = false;
        keyJumpPressed = false;
    }

    if (!overlayOpen) {
        sceneClock += deltaTime;
        scrollDistance += 8.0f * deltaTime;
        camera.OrbitYaw += 14.0f * deltaTime;
        if (shotMode) { camera.OrbitYaw = shotYaw; delorean.morphFlash = 0.0f; }
        delorean.update(0.0f, 8.0f, false, 0.0f, false, false, 0.0f, 0.0f, deltaTime, deltaTime);
    }
}

void GameApp::updatePlaying(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!keyPPressed) { state = STATE_PAUSED; keyPPressed = true; }
    } else keyPPressed = false;

    if (laneSwitchTimer > 0.0f) {
        laneSwitchTimer = std::max(0.0f, laneSwitchTimer - deltaTime);
        if (laneSwitchTimer <= 0.0f) delorean.turnSignalDir = 0;
    }
    if (stumbleAlertTimer > 0.0f) stumbleAlertTimer -= deltaTime;
    if (comboDecayTimer > 0.0f) {
        comboDecayTimer -= deltaTime;
        if (comboDecayTimer <= 0.0f) { comboStreak = 0; nextFeverMark = 25; }
    }
    if (stuntScoreCD > 0.0f) stuntScoreCD = std::max(0.0f, stuntScoreCD - deltaTime);
    if (styleDecayTimer > 0.0f) {
        styleDecayTimer -= deltaTime;
        if (styleDecayTimer <= 0.0f) stylePoints = std::max(0, stylePoints - 25);
    }

    bool classicMode = (gameMode == MODE_CLASSIC);

    // [Q] Sandevistan — в Classic требует собранный спец-токен (qCharges > 0)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (!keyQPressed) {
            if (classicMode && qCharges <= 0) { bannerText = "FIND [Q] TOKEN"; bannerTimer = 0.9f; }
            else if (sandevistanCD <= 0.0f && sandevistanTimer <= 0.0f) {
                if (classicMode) qCharges--;
                sandevistanTimer = 5.0f;
                sandevistanCD = classicMode ? 4.0f : 2.0f;
                bannerText = "SANDEVISTAN"; bannerTimer = 1.3f;
            }
            keyQPressed = true;
        }
    } else keyQPressed = false;

    // [X] EMP-волна
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (!keyXPressed) {
            if (classicMode && xCharges <= 0) { bannerText = "FIND [X] TOKEN"; bannerTimer = 0.9f; }
            else if (empCD <= 0.0f) {
                if (classicMode) xCharges--;
                empCD = classicMode ? 4.0f : 2.0f;
                delorean.triggerEmpWave();
                int hits = track.empPulse(scrollDistance, delorean.bikeMode ? 28.0f : 48.0f);
                stumbleAlertTimer = 0.0f;
                int bonus = (hits > 0) ? awardPoints(hits * 30) : 0;
                if (!destroyHunter()) {
                    flashColor = glm::vec3(0.14f, 0.85f, 1.0f); flashTimer = 0.16f; shakeTimer = 0.18f;
                    bannerText = (bonus > 0) ? ("EMP +" + std::to_string(bonus)) : "EMP WAVE";
                    bannerTimer = 1.3f;
                }
            }
            keyXPressed = true;
        }
    } else keyXPressed = false;

    // [Z] Phase Shift
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        if (!keyZPressed) {
            if (classicMode && zCharges <= 0) { bannerText = "FIND [Z] TOKEN"; bannerTimer = 0.9f; }
            else if (phaseCD <= 0.0f && phaseTimer <= 0.0f) {
                if (classicMode) zCharges--;
                phaseTimer = delorean.bikeMode ? 5.0f : 4.2f;
                phaseCD = classicMode ? 4.0f : 2.0f;
                bannerText = "PHASE SHIFT"; bannerTimer = 1.3f;
            }
            keyZPressed = true;
        }
    } else keyZPressed = false;

    // [N] Nitro Shock-Dash
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        if (!keyNPressed) {
            if (classicMode && nCharges <= 0) { bannerText = "FIND [N] TOKEN"; bannerTimer = 0.9f; }
            else if (dashCD <= 0.0f && dashTimer <= 0.0f) {
                if (classicMode) nCharges--;
                dashTimer = delorean.bikeMode ? 1.8f : 2.2f;
                dashCD = classicMode ? 4.0f : 2.0f;
                if (!destroyHunter()) { shakeTimer = 0.16f; bannerText = "SHOCK DASH"; bannerTimer = 1.3f; }
            }
            keyNPressed = true;
        }
    } else keyNPressed = false;

    // [U] Супер-Магнит
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        if (!keyUPressed) {
            if (classicMode && uCharges <= 0) { bannerText = "FIND [U] TOKEN"; bannerTimer = 0.9f; }
            else if (magOverdriveCD <= 0.0f && magOverdriveTimer <= 0.0f) {
                if (classicMode) uCharges--;
                magOverdriveTimer = delorean.bikeMode ? 7.0f : 10.0f;
                magOverdriveCD = classicMode ? 4.0f : 2.0f;
                bannerText = "COIN STORM"; bannerTimer = 1.4f;
            }
            keyUPressed = true;
        }
    } else keyUPressed = false;

    // Перестроение по 3 полосам (0 = левая, 1 = центр, 2 = правая) и включение поворотника
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (!keyAPressed) {
            if (currentLane > 0) {
                prevLane = currentLane; currentLane--; laneSwitchTimer = 0.42f;
                delorean.turnSignalDir = -1; lastTurnDir = -1;
            }
            keyAPressed = true;
        }
    } else keyAPressed = false;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (!keyDPressed) {
            if (currentLane < 2) {
                prevLane = currentLane; currentLane++; laneSwitchTimer = 0.42f;
                delorean.turnSignalDir = +1; lastTurnDir = +1;
            }
            keyDPressed = true;
        }
    } else keyDPressed = false;

    // Прыжок (SPACE / W / Вверх).
    // Важно для режима Classic: двойной и тройной прыжок работают только если подобран зелёный коин прыжка (sneakersTimer > 0)
    bool jumpKey = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                    glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                    glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
    bool allowAirJumps = (!classicMode) || (sneakersTimer > 0.0f);
    if (jumpKey) {
        if (!keyJumpPressed) {
            int jRes = delorean.jump(1.0f, allowAirJumps);
            if (jRes == -1) { bannerText = "FIND JUMP COIN"; bannerTimer = 0.9f; }
            else if (delorean.hover && jRes == 3) {
                addStyle(12);
                if (stuntScoreCD <= 0.0f) { stuntScoreCD = 2.4f; bannerText = "BARREL ROLL +" + std::to_string(awardPoints(15)); }
                else bannerText = "BARREL ROLL";
                bannerTimer = 0.9f;
            } else if (jRes == 2) { addStyle(8); bannerText = "DOUBLE JUMP"; bannerTimer = 0.9f; }
            else if (jRes == 3) {
                addStyle(14);
                int tjPts = (stuntScoreCD <= 0.0f) ? awardPoints(15) : 0;
                if (tjPts > 0) stuntScoreCD = 2.0f;
                bannerText = (tjPts > 0) ? ("AKIRA TRIPLE +" + std::to_string(tjPts)) : "AKIRA TRIPLE";
                bannerTimer = 0.9f;
            }
            keyJumpPressed = true;
        }
    } else keyJumpPressed = false;

    // J — сальто 360 градусов назад, пока транспорт в воздухе
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        if (!keyJPressed) {
            if (delorean.startTrick()) { addStyle(12); bannerText = "360 BACKFLIP..."; bannerTimer = 0.8f; }
            keyJPressed = true;
        }
    } else keyJPressed = false;

    // S / Вниз — присесть под лазерные ворота (а на мотоцикле ещё и сделать боковой дрифт Akira Slide)
    bool duckInput = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
    if (duckInput) {
        if (!keySPressed) {
            if (delorean.bikeMode && delorean.grounded && !delorean.hover && delorean.akiraSlideTimer <= 0.0f) {
                delorean.triggerAkiraSlide(lastTurnDir);
                addStyle(12);
                if (stuntScoreCD <= 0.0f) { stuntScoreCD = 2.4f; bannerText = "AKIRA SLIDE +" + std::to_string(awardPoints(20)); }
                else bannerText = "AKIRA SLIDE";
                bannerTimer = 1.1f;
            }
            keySPressed = true;
        }
    } else keySPressed = false;

    // SHIFT — нитро-ускорение. В Free Play оно бесконечное, а в Classic тратит nitroFuel (пополняется оранжевым коином)
    bool shiftHeld = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    bool boostInput = shiftHeld && (!classicMode || nitroFuel > 0.0f);
    if (classicMode && boostInput) {
        nitroFuel = std::max(0.0f, nitroFuel - 28.0f * deltaTime);
    } else if (classicMode && shiftHeld && nitroFuel <= 0.0f) {
        if (!keyShiftWarned) { bannerText = "FIND NITRO COIN"; bannerTimer = 0.9f; keyShiftWarned = true; }
    } else if (!shiftHeld) {
        keyShiftWarned = false;
    }

    // E — включает защитный щит (в Classic нужен синий энергоблок eCharges > 0)
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!keyEPressed) {
            if (classicMode && shieldTimer <= 0.0f && eCharges <= 0) { bannerText = "FIND [E] SHIELD"; bannerTimer = 0.9f; }
            else if (shieldTimer <= 0.0f && shieldCD <= 0.0f) {
                if (classicMode) eCharges--;
                shieldMaxTimer = delorean.bikeMode ? 14.0f : 20.0f;
                shieldTimer = shieldMaxTimer;
                bannerText = delorean.bikeMode ? "SHIELD 14S" : "SHIELD 20S"; bannerTimer = 1.3f;
            } else if (shieldTimer <= 0.0f && shieldCD > 0.0f) {
                bannerText = "SHIELD CD " + std::to_string((int)shieldCD + 1) + "S"; bannerTimer = 0.9f;
            }
            keyEPressed = true;
        }
    } else keyEPressed = false;

    // F — включает режим полёта (колёса складываются на 90 градусов, в Classic нужен заряд fCharges > 0)
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!keyFPressed) {
            if (!delorean.hover && delorean.isInsideTunnel()) { bannerText = "NO FLIGHT IN TUNNEL"; bannerTimer = 1.3f; }
            else if (!delorean.hover && classicMode && fCharges <= 0) { bannerText = "FIND [F] FLIGHT"; bannerTimer = 0.9f; }
            else if (!delorean.hover && hoverCD > 0.0f) { bannerText = "FLIGHT CD " + std::to_string((int)hoverCD + 1) + "S"; bannerTimer = 0.9f; }
            else {
                delorean.hover = !delorean.hover;
                if (delorean.hover) {
                    if (classicMode) fCharges--;
                    hoverMaxTimer = delorean.bikeMode ? 12.0f : 16.0f;
                    hoverTimer = hoverMaxTimer;
                    flightRingsCollected = 0;
                    track.startFlight(scrollDistance, classicMode);
                    bannerText = classicMode ? "FLIGHT (4 RINGS)" : (delorean.bikeMode ? "AKIRA JET 12S" : "CRUISER FLIGHT");
                    bannerTimer = 1.3f;
                } else {
                    hoverTimer = 0.0f; hoverCD = classicMode ? 4.0f : 2.0f; track.endFlight();
                }
            }
            keyFPressed = true;
        }
    } else keyFPressed = false;

    // Передаём собранные нажатия в GameEvents.cpp, где обновляется физика, трасса и столкновения
    updatePlayingTrackAndEvents(classicMode, duckInput, boostInput, allowAirJumps);
}

// Обработка клавиш на экране паузы (P, R, M) и экране проигрыша Game Over (Space/R повтор, E продолжить за щит)
void GameApp::updatePausedAndGameOver(GLFWwindow* window) {
    if (state == STATE_PAUSED) {
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            if (!keyPPressed) { state = STATE_PLAYING; keyPPressed = true; }
        } else keyPPressed = false;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) { resetGame(); state = STATE_PLAYING; }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            state = STATE_MENU; camera.SetMode(ORBIT_CAM); delorean.doorsOpen = true;
        }
    } else if (state == STATE_GAMEOVER) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            if (!keyRPressed && !keyJumpPressed && !keyEnterPressed) {
                resetGame(); state = STATE_PLAYING;
                keyRPressed = keyJumpPressed = keyEnterPressed = true;
            }
        } else {
            keyRPressed = keyJumpPressed = keyEnterPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            if (!keyEPressed) {
                if (gameMode == MODE_FREE_PLAY || eCharges > 0) {
                    if (gameMode == MODE_CLASSIC) eCharges--;
                    track.clearNearbyObstacles(scrollDistance, 32.0f);
                    invulnerableTimer = 2.2f; shieldTimer = 10.0f; stumbleAlertTimer = 0.0f;
                    state = STATE_PLAYING; bannerText = "SAVED"; bannerTimer = 1.4f;
                }
                keyEPressed = true;
            }
        } else keyEPressed = false;

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            state = STATE_MENU; camera.SetMode(ORBIT_CAM); delorean.doorsOpen = true;
        }
    }
}
