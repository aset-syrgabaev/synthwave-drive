#include "GameApp.h"

// Этот файл рисует весь плоский 2D-интерфейс поверх 3D-картинки (меню, счёт, монеты, кнопки,
// панель настроек G, подсказку H и зелёный счётчик FPS в правом верхнем углу).
// Для 2D мы временно выключаем проверку глубины GL_DEPTH_TEST и включаем прозрачность GL_BLEND.
void GameApp::renderHud() {
    bool overlayOpen = showHelpOverlay || showSettings;

    // Отключаем Z-буфер, чтобы 2D-плашки всегда рисовались ровно поверх 3D-мира
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Очищаем батч прямоугольников/букв перед наполнением на этот кадр
    textRenderer.begin(scrWidth, scrHeight);

    float sw = static_cast<float>(scrWidth);
    float sh = static_cast<float>(scrHeight);

    // Мягкая цветовая вспышка по всему экрану при подборе бонуса, EMP или аварии
    if (flashTimer > 0.0f) {
        float alpha = (flashTimer / 0.25f) * 0.14f;
        textRenderer.rect(0.0f, 0.0f, sw, sh, glm::vec4(flashColor, alpha));
    }

    // Неоновая рамка по краю экрана, когда активно замедление времени [Q] или призрак [Z]
    if (sandevistanTimer > 0.0f && state == STATE_PLAYING) {
        textRenderer.frame(3.0f, 3.0f, sw - 6.0f, sh - 6.0f, 2.5f, glm::vec4(0.18f, 0.88f, 0.84f, 0.58f));
    } else if (phaseTimer > 0.0f && state == STATE_PLAYING) {
        textRenderer.frame(3.0f, 3.0f, sw - 6.0f, sh - 6.0f, 2.5f, glm::vec4(0.68f, 0.42f, 1.0f, 0.62f));
    }

    // Главный экран меню: заголовок, 2 кликабельные кнопки режима (CLASSIC / FREE PLAY) и кнопка старта
    if (state == STATE_MENU && !overlayOpen) {
        float cardW = 460.0f;
        float cardX = sw * 0.5f - cardW * 0.5f;
        textRenderer.rect(cardX, 20.0f, cardW, 110.0f, glm::vec4(0.02f, 0.03f, 0.06f, 0.84f));
        textRenderer.frame(cardX, 20.0f, cardW, 110.0f, 1.5f, glm::vec4(0.28f, 0.78f, 0.94f, 0.75f));

        textRenderer.textCentered("SYNTHWAVE SURFERS", sw * 0.5f, 32.0f, 3.0f,
                                  glm::vec4(0.94f, 0.97f, 1.0f, 1.0f), glm::vec4(0.08f, 0.55f, 0.78f, 0.6f));
        textRenderer.textCentered("DELOREAN DMC-12  |  SE-2423", sw * 0.5f, 64.0f, 1.45f,
                                  glm::vec4(0.68f, 0.82f, 0.94f, 0.92f), glm::vec4(0.0f));

        float tabW = 190.0f, tabGap = 18.0f;
        float tabStartX = sw * 0.5f - (2.0f * tabW + tabGap) * 0.5f;
        float tabY = 88.0f;
        for (int m = 0; m < 2; ++m) {
            float tx = tabStartX + (float)m * (tabW + tabGap);
            bool active = ((int)gameMode == m);
            if (clickNow && mouseX >= tx && mouseX <= tx + tabW && mouseY >= tabY && mouseY <= tabY + 32.0f) {
                gameMode = static_cast<GameMode>(m);
                active = true;
            }
            textRenderer.rect(tx, tabY, tabW, 32.0f, active ? glm::vec4(0.10f, 0.34f, 0.48f, 0.92f) : glm::vec4(0.05f, 0.06f, 0.12f, 0.84f));
            textRenderer.frame(tx, tabY, tabW, 32.0f, active ? 2.0f : 1.0f, active ? glm::vec4(0.32f, 0.92f, 1.0f, 0.95f) : glm::vec4(0.32f, 0.40f, 0.52f, 0.60f));
            textRenderer.textCentered(std::to_string(m + 1) + "  " + modeShort[m], tx + tabW * 0.5f, tabY + 10.0f, 1.75f,
                                      active ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.72f, 0.78f, 0.88f, 0.88f), glm::vec4(0.0f));
        }

        float botY = sh - 86.0f;
        float playW = 280.0f;
        float playX = sw * 0.5f - playW * 0.5f;
        if (clickNow && mouseX >= playX && mouseX <= playX + playW && mouseY >= botY && mouseY <= botY + 40.0f) {
            resetGame();
            state = STATE_PLAYING;
        }
        textRenderer.rect(playX, botY, playW, 40.0f, glm::vec4(0.05f, 0.22f, 0.30f, 0.90f));
        textRenderer.frame(playX, botY, playW, 40.0f, 2.0f, glm::vec4(0.30f, 0.92f, 1.0f, 0.92f));
        textRenderer.textCentered("SPACE : PLAY", sw * 0.5f, botY + 12.0f, 2.25f,
                                  glm::vec4(0.98f, 0.99f, 1.0f, 1.0f), glm::vec4(0.0f));

        textRenderer.textCentered("K : BIKE / CAR   |   T : 8 WORLDS   |   B : 8 SKINS   |   G : SET   |   H : KEYS",
                                  sw * 0.5f, botY + 52.0f, 1.45f, glm::vec4(0.76f, 0.86f, 0.96f, 0.92f), glm::vec4(0.0f));
    }
    else if (state == STATE_PLAYING || state == STATE_PAUSED) {
        // 1) Верхний левый угол: счёт и скорость в милях/ч
        textRenderer.rect(18.0f, 16.0f, 196.0f, 58.0f, glm::vec4(0.02f, 0.03f, 0.06f, 0.80f));
        textRenderer.frame(18.0f, 16.0f, 196.0f, 58.0f, 1.5f, glm::vec4(0.24f, 0.72f, 0.88f, 0.65f));
        textRenderer.text(std::to_string(score), 30.0f, 23.0f, 2.5f, glm::vec4(0.98f, 0.99f, 1.0f, 1.0f));

        int mph = static_cast<int>(currentSpeed * 2.85f);
        std::string rankStr = (stylePoints >= 95) ? "AKIRA!"
                            : (stylePoints >= 65) ? "RANK:S"
                            : (stylePoints >= 40) ? "RANK:A"
                            : (stylePoints >= 20) ? "RANK:B" : "";
        std::string subLeft = std::to_string(mph) + " MPH";
        if (!rankStr.empty()) subLeft += "  " + rankStr;
        else if (delorean.bikeMode) subLeft += "  1.25X";
        else if (mph >= 88 && boostFactor > 0.6f) subLeft += "  RAM!";
        else if (comboStreak >= 5) subLeft += "  x" + std::to_string(comboStreak);
        textRenderer.text(subLeft, 30.0f, 51.0f, 1.50f,
                          (stylePoints >= 65 || delorean.bikeMode) ? glm::vec4(1.0f, 0.42f, 0.44f, 0.96f) : glm::vec4(0.55f, 0.88f, 1.0f, 0.95f));

        // 2) Центр сверху: прогресс букв текущего слова
        const std::string& activeWord = wordList[currentWordIdx];
        int wLen = (int)activeWord.size();
        float badgeStep = 36.0f;
        float totalBadgesW = (float)wLen * badgeStep - 6.0f;
        float cardW = totalBadgesW + 24.0f;
        float cardX = sw * 0.5f - cardW * 0.5f;
        textRenderer.rect(cardX, 16.0f, cardW, 46.0f, glm::vec4(0.02f, 0.03f, 0.06f, 0.82f));
        textRenderer.frame(cardX, 16.0f, cardW, 46.0f, 1.5f, glm::vec4(0.32f, 0.82f, 0.96f, 0.70f));

        float badgeStartX = sw * 0.5f - totalBadgesW * 0.5f;
        for (int k = 0; k < wLen; ++k) {
            float bx = badgeStartX + (float)k * badgeStep;
            bool got = (k < currentLetterProgress);
            textRenderer.rect(bx, 24.0f, 30.0f, 30.0f, got ? glm::vec4(0.92f, 0.78f, 0.24f, 0.96f) : glm::vec4(0.08f, 0.11f, 0.18f, 0.90f));
            textRenderer.frame(bx, 24.0f, 30.0f, 30.0f, 1.5f, got ? glm::vec4(1.0f, 0.96f, 0.65f, 1.0f) : glm::vec4(0.40f, 0.52f, 0.68f, 0.72f));
            std::string lStr(1, activeWord[k]);
            textRenderer.textCentered(lStr, bx + 15.0f, 31.0f, 2.2f,
                                      got ? glm::vec4(0.04f, 0.05f, 0.08f, 1.0f) : glm::vec4(0.90f, 0.94f, 1.0f, 0.95f), glm::vec4(0.0f));
        }

        // 3) Верхний правый угол: монеты и краткий статус
        float rightW = 196.0f;
        float rightX = sw - rightW - 18.0f;
        textRenderer.rect(rightX, 16.0f, rightW, 58.0f, glm::vec4(0.02f, 0.03f, 0.06f, 0.80f));
        textRenderer.frame(rightX, 16.0f, rightW, 58.0f, 1.5f, glm::vec4(0.24f, 0.72f, 0.88f, 0.65f));
        textRenderer.text("COINS " + std::to_string(coins), rightX + 14.0f, 23.0f, 2.0f, glm::vec4(0.98f, 0.86f, 0.34f, 1.0f));

        bool inDarkTunnel = (delorean.roadY < -2.5f && !delorean.onTunnelRoof && !delorean.hover);
        if (inDarkTunnel && !delorean.headlights)
            textRenderer.text("L : LIGHTS!", rightX + 14.0f, 51.0f, 1.55f, glm::vec4(1.0f, 0.42f, 0.32f, 1.0f));
        else if (delorean.hover)
            textRenderer.text("SPC:ROLL S:DIVE", rightX + 14.0f, 51.0f, 1.40f, glm::vec4(0.32f, 0.94f, 1.0f, 1.0f));
        else if (onLrtDeck && delorean.bottomWorldY() > 5.0f)
            textRenderer.text("SKY WALK", rightX + 14.0f, 51.0f, 1.50f, glm::vec4(1.0f, 0.42f, 0.86f, 1.0f));
        else if (onLrtDeck)
            textRenderer.text("LRT DECK", rightX + 14.0f, 51.0f, 1.50f, glm::vec4(0.28f, 0.94f, 1.0f, 1.0f));
        else if (hunterTimer > 0.0f)
            textRenderer.text("HUNTER LOCK!", rightX + 14.0f, 51.0f, 1.45f, glm::vec4(1.0f, 0.28f, 0.30f, 1.0f));
        else if (delorean.onTunnelRoof)
            textRenderer.text("TUNNEL ROOF", rightX + 14.0f, 51.0f, 1.50f, glm::vec4(0.35f, 0.96f, 0.72f, 1.0f));
        else
            textRenderer.text("G:SET  H:KEYS", rightX + 14.0f, 51.0f, 1.50f, glm::vec4(0.72f, 0.82f, 0.94f, 0.92f));

        if (inDarkTunnel && !delorean.headlights) {
            float ww = 290.0f;
            textRenderer.rect(sw * 0.5f - ww * 0.5f, 70.0f, ww, 32.0f, glm::vec4(0.14f, 0.04f, 0.05f, 0.88f));
            textRenderer.frame(sw * 0.5f - ww * 0.5f, 70.0f, ww, 32.0f, 1.5f, glm::vec4(0.95f, 0.40f, 0.28f, 0.90f));
            textRenderer.textCentered("PRESS [L] LIGHTS", sw * 0.5f, 79.0f, 1.85f, glm::vec4(1.0f, 0.92f, 0.55f, 1.0f), glm::vec4(0.0f));
        }

        // Полоса активного бонуса внизу по центру
        float activeTimer = 0.0f, maxTimer = 14.0f;
        std::string timerLabel = "";
        glm::vec4 timerColor(0.14f, 0.78f, 0.94f, 0.88f);

        if (psychoTimer > 0.0f) {
            activeTimer = psychoTimer; maxTimer = 5.5f;
            timerLabel = "AKIRA PSYCHO " + std::to_string((int)psychoTimer + 1) + "S";
            timerColor = glm::vec4(0.96f, 0.16f, 0.24f, 0.90f);
        } else if (phaseTimer > 0.0f) {
            activeTimer = phaseTimer; maxTimer = 4.5f;
            timerLabel = "PHASE " + std::to_string((int)phaseTimer + 1) + "S";
            timerColor = glm::vec4(0.58f, 0.34f, 0.95f, 0.88f);
        } else if (sandevistanTimer > 0.0f) {
            activeTimer = sandevistanTimer; maxTimer = 5.0f;
            timerLabel = "SLOW-MO " + std::to_string((int)sandevistanTimer + 1) + "S";
            timerColor = glm::vec4(0.12f, 0.82f, 0.78f, 0.88f);
        } else if (hoverTimer > 0.0f) {
            activeTimer = hoverTimer; maxTimer = hoverMaxTimer;
            timerLabel = "HOVER " + std::to_string((int)hoverTimer + 1) + "S";
            timerColor = glm::vec4(0.12f, 0.72f, 0.94f, 0.86f);
        } else if (shieldTimer > 0.0f) {
            activeTimer = shieldTimer; maxTimer = shieldMaxTimer;
            timerLabel = "SHIELD " + std::to_string((int)shieldTimer + 1) + "S";
            timerColor = glm::vec4(0.16f, 0.76f, 0.96f, 0.86f);
        } else if (magnetTimer > 0.0f) {
            activeTimer = magnetTimer; maxTimer = 14.0f;
            timerLabel = "MAGNET " + std::to_string((int)magnetTimer + 1) + "S";
            timerColor = glm::vec4(0.90f, 0.64f, 0.16f, 0.86f);
        } else if (sneakersTimer > 0.0f) {
            activeTimer = sneakersTimer; maxTimer = 18.0f;
            timerLabel = "AIR JUMP " + std::to_string((int)sneakersTimer + 1) + "S";
            timerColor = glm::vec4(0.18f, 0.80f, 0.44f, 0.86f);
        }

        if (activeTimer > 0.0f) {
            float barW = 260.0f, barX = sw * 0.5f - barW * 0.5f, barY = sh - 44.0f;
            textRenderer.rect(barX, barY, barW, 26.0f, glm::vec4(0.02f, 0.03f, 0.06f, 0.86f));
            textRenderer.frame(barX, barY, barW, 26.0f, 1.5f, glm::vec4(0.28f, 0.84f, 0.96f, 0.85f));
            float fill = std::clamp(activeTimer / maxTimer, 0.0f, 1.0f);
            textRenderer.rect(barX + 3.0f, barY + 3.0f, (barW - 6.0f) * fill, 20.0f, timerColor);
            textRenderer.textCentered(timerLabel, sw * 0.5f, barY + 7.0f, 1.65f, glm::vec4(1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.85f));
        }

        // Индикаторы способностей: в Classic отображаются собранные заряды (x0/x1..), в Free Play открыто всё
        bool isClassicHud = (gameMode == MODE_CLASSIC);
        float pillY = sh - 42.0f;
        bool qReady = (sandevistanCD <= 0.0f && sandevistanTimer <= 0.0f) && (!isClassicHud || qCharges > 0);
        bool xReady = (empCD <= 0.0f) && (!isClassicHud || xCharges > 0);
        bool zReady = (phaseCD <= 0.0f && phaseTimer <= 0.0f) && (!isClassicHud || zCharges > 0);
        bool nReady = (dashCD <= 0.0f && dashTimer <= 0.0f) && (!isClassicHud || nCharges > 0);
        bool uReady = (magOverdriveCD <= 0.0f && magOverdriveTimer <= 0.0f) && (!isClassicHud || uCharges > 0);

        auto fmtPill = [&](const char* key, const char* name, float activeT, float cdT, int chg) -> std::string {
            if (activeT > 0.0f) return std::string(key) + ":ON";
            if (isClassicHud) return std::string(key) + ":x" + std::to_string(chg);
            if (cdT > 0.0f) return std::string(key) + ":" + std::to_string((int)cdT + 1) + "S";
            return std::string(key) + ":" + name;
        };
        auto drawPill = [&](float px, float pw, const std::string& txt, bool ready, const glm::vec4& borderCol) {
            textRenderer.rect(px, pillY, pw, 26.0f, ready ? glm::vec4(0.04f, 0.14f, 0.22f, 0.84f) : glm::vec4(0.02f, 0.03f, 0.06f, 0.78f));
            textRenderer.frame(px, pillY, pw, 26.0f, 1.5f, ready ? borderCol : glm::vec4(0.32f, 0.38f, 0.48f, 0.55f));
            textRenderer.textCentered(txt, px + pw * 0.5f, pillY + 7.0f, 1.40f,
                                      ready ? glm::vec4(0.94f, 0.98f, 1.0f, 1.0f) : glm::vec4(0.58f, 0.64f, 0.74f, 0.85f), glm::vec4(0.0f));
        };

        drawPill(16.0f,  64.0f, fmtPill("Q", "SLOW", sandevistanTimer, sandevistanCD, qCharges), qReady || sandevistanTimer > 0.0f, glm::vec4(0.18f, 0.90f, 0.84f, 0.90f));
        drawPill(84.0f,  60.0f, fmtPill("X", "EMP",  0.0f,             empCD,         xCharges), xReady,                             glm::vec4(0.24f, 0.84f, 1.00f, 0.90f));
        drawPill(148.0f, 68.0f, fmtPill("Z", "GHST", phaseTimer,       phaseCD,       zCharges), zReady || phaseTimer > 0.0f,        glm::vec4(0.72f, 0.48f, 1.00f, 0.90f));
        drawPill(220.0f, 66.0f, fmtPill("N", "DASH", dashTimer,        dashCD,        nCharges), nReady || dashTimer > 0.0f,         glm::vec4(1.00f, 0.60f, 0.22f, 0.90f));
        drawPill(290.0f, 62.0f, fmtPill("U", "MAG",  magOverdriveTimer,magOverdriveCD,uCharges), uReady || magOverdriveTimer > 0.0f, glm::vec4(1.00f, 0.84f, 0.24f, 0.90f));
        drawPill(356.0f, 72.0f, delorean.bikeMode ? "K:CAR" : "K:AKIRA", true,                   glm::vec4(1.00f, 0.28f, 0.32f, 0.92f));
        drawPill(432.0f, 62.0f, "J:FLIP", !delorean.grounded && !delorean.hover,                 glm::vec4(1.00f, 0.52f, 0.88f, 0.90f));

        bool nosReady = (!isClassicHud || nitroFuel > 0.5f);
        std::string nosTxt = isClassicHud ? ("NOS:" + std::to_string((int)std::ceil(nitroFuel)) + "%") : "NOS:INF";
        bool eReady = (shieldTimer <= 0.0f && shieldCD <= 0.0f) && (!isClassicHud || eCharges > 0);
        bool fReady = (!delorean.hover && hoverCD <= 0.0f) && (!isClassicHud || fCharges > 0);
        drawPill(sw - 266.0f, 82.0f, nosTxt, nosReady,                  glm::vec4(1.00f, 0.54f, 0.14f, 0.92f));
        drawPill(sw - 178.0f, 78.0f, fmtPill("E", "SHLD", shieldTimer, shieldCD, eCharges), eReady || shieldTimer > 0.0f, glm::vec4(0.20f, 0.82f, 1.00f, 0.90f));
        drawPill(sw -  94.0f, 78.0f, delorean.hover ? "F:FLY" : fmtPill("F", "FLY", 0.0f, hoverCD, fCharges), fReady || delorean.hover, glm::vec4(0.32f, 0.96f, 0.72f, 0.90f));

        if (bannerTimer > 0.0f && !bannerText.empty() && !(inDarkTunnel && !delorean.headlights)) {
            float bw = textRenderer.width(bannerText, 1.85f) + 32.0f;
            float bx = sw * 0.5f - bw * 0.5f, by = 70.0f;
            textRenderer.rect(bx, by, bw, 32.0f, glm::vec4(0.02f, 0.03f, 0.07f, 0.86f));
            textRenderer.frame(bx, by, bw, 32.0f, 1.5f, glm::vec4(0.35f, 0.86f, 1.0f, 0.85f));
            textRenderer.textCentered(bannerText, sw * 0.5f, by + 9.0f, 1.85f, glm::vec4(0.96f, 0.98f, 1.0f, 1.0f), glm::vec4(0.0f));
        }

        if (state == STATE_PAUSED && !overlayOpen) {
            textRenderer.rect(0.0f, 0.0f, sw, sh, glm::vec4(0.02f, 0.02f, 0.05f, 0.72f));
            textRenderer.textCentered("PAUSED", sw * 0.5f, sh * 0.38f, 3.8f, glm::vec4(0.96f, 0.98f, 1.0f, 1.0f), glm::vec4(0.0f));
            textRenderer.textCentered("P : RESUME   |   G : SET   |   R : RESTART   |   M : MENU", sw * 0.5f, sh * 0.50f, 1.75f,
                                      glm::vec4(0.80f, 0.90f, 1.0f, 1.0f), glm::vec4(0.0f));
        }
    }
    else if (state == STATE_GAMEOVER && !overlayOpen) {
        textRenderer.rect(0.0f, 0.0f, sw, sh, glm::vec4(0.02f, 0.02f, 0.05f, 0.74f));
        float cardW = 440.0f, cardH = 310.0f, cardX = sw * 0.5f - cardW * 0.5f, cardY = sh * 0.5f - cardH * 0.5f;
        textRenderer.rect(cardX, cardY, cardW, cardH, glm::vec4(0.03f, 0.03f, 0.07f, 0.94f));
        textRenderer.frame(cardX, cardY, cardW, cardH, 2.5f, glm::vec4(0.92f, 0.28f, 0.36f, 0.92f));
        textRenderer.textCentered("RUN OVER", sw * 0.5f, cardY + 24.0f, 3.4f, glm::vec4(0.98f, 0.40f, 0.46f, 1.0f), glm::vec4(0.0f));
        textRenderer.text("SCORE : " + std::to_string(score), cardX + 48.0f, cardY + 90.0f, 2.1f, glm::vec4(0.98f, 0.99f, 1.0f, 1.0f));
        textRenderer.text("COINS : " + std::to_string(coins), cardX + 48.0f, cardY + 124.0f, 1.95f, glm::vec4(0.98f, 0.86f, 0.34f, 1.0f));
        textRenderer.text("BEST  : " + std::to_string(highScore), cardX + 48.0f, cardY + 158.0f, 1.95f, glm::vec4(0.42f, 0.90f, 1.0f, 1.0f));
        std::string contTxt = (gameMode == MODE_CLASSIC && eCharges <= 0) ? "NO SHIELD CHARGE" : "E : CONTINUE";
        glm::vec4 contCol = (gameMode == MODE_CLASSIC && eCharges <= 0) ? glm::vec4(0.55f, 0.58f, 0.65f, 0.85f) : glm::vec4(0.42f, 0.94f, 1.0f, 1.0f);
        textRenderer.textCentered(contTxt, sw * 0.5f, cardY + 212.0f, 2.0f, contCol, glm::vec4(0.0f));
        textRenderer.textCentered("SPACE : RETRY   |   M : MENU", sw * 0.5f, cardY + 256.0f, 1.75f, glm::vec4(0.92f, 0.95f, 1.0f, 1.0f), glm::vec4(0.0f));
    }

    // Интерактивная панель настроек (G)
    if (showSettings) {
        textRenderer.rect(0.0f, 0.0f, sw, sh, glm::vec4(0.01f, 0.02f, 0.04f, 0.78f));
        float pw = 560.0f, ph = 524.0f, px = sw * 0.5f - pw * 0.5f, py = sh * 0.5f - ph * 0.5f;
        textRenderer.rect(px, py, pw, ph, glm::vec4(0.03f, 0.035f, 0.08f, 0.96f));
        textRenderer.frame(px, py, pw, ph, 2.0f, glm::vec4(0.26f, 0.84f, 0.96f, 0.92f));
        textRenderer.textCentered("SETTINGS", sw * 0.5f, py + 16.0f, 2.6f, glm::vec4(0.96f, 0.98f, 1.0f, 1.0f), glm::vec4(0.0f));

        const std::string labels[10] = {
            "MODE", "WORLD (T)", "VEHICLE (K)", "PAINT (B)", "CAMERA",
            "HEADLIGHTS", "CRT FILTER", "BLOOM", "SPEED", "DOORS"
        };
        const std::string values[10] = {
            modeShort[(int)gameMode], themes[themeIdx].name,
            delorean.bikeMode ? "AKIRA BIKE" : "DELOREAN",
            paintNames[delorean.paintIdx], camNames[(int)camera.Mode],
            delorean.headlights ? "ON" : "OFF", postProcess.enabled ? "ON" : "OFF",
            levelNames[bloomLevel], speedNames[speedLevel],
            delorean.doorsOpen ? "OPEN" : "CLOSED"
        };

        for (int i = 0; i < 10; ++i) {
            float ry = py + 56.0f + (float)i * 42.0f;
            bool hovered = (mouseX >= px + 24.0f && mouseX <= px + pw - 24.0f && mouseY >= ry && mouseY <= ry + 34.0f);
            if (hovered) settingsCursor = i;
            if (clickNow && hovered) applySettingChange(i, +1);
            bool sel = (settingsCursor == i);
            textRenderer.rect(px + 24.0f, ry, pw - 48.0f, 34.0f, sel ? glm::vec4(0.10f, 0.28f, 0.42f, 0.90f) : glm::vec4(0.05f, 0.06f, 0.12f, 0.78f));
            if (sel) textRenderer.frame(px + 24.0f, ry, pw - 48.0f, 34.0f, 1.5f, glm::vec4(0.32f, 0.90f, 1.0f, 0.92f));
            textRenderer.text(labels[i], px + 40.0f, ry + 10.0f, 1.75f, sel ? glm::vec4(1.0f) : glm::vec4(0.78f, 0.84f, 0.92f, 0.95f));
            std::string valStr = "< " + values[i] + " >";
            float vw = textRenderer.width(valStr, 1.75f);
            textRenderer.text(valStr, px + pw - 40.0f - vw, ry + 10.0f, 1.75f,
                              sel ? glm::vec4(0.98f, 0.88f, 0.36f, 1.0f) : glm::vec4(0.42f, 0.88f, 1.0f, 0.95f));
        }
        textRenderer.textCentered("ARROWS / CLICK : CHANGE   |   G / ESC : CLOSE", sw * 0.5f, py + ph - 28.0f, 1.45f,
                                  glm::vec4(0.74f, 0.84f, 0.94f, 0.92f), glm::vec4(0.0f));
    }

    // Карточка клавиш (H)
    if (showHelpOverlay) {
        textRenderer.rect(0.0f, 0.0f, sw, sh, glm::vec4(0.01f, 0.02f, 0.04f, 0.78f));
        float hw = 590.0f, hh = 514.0f, hx = sw * 0.5f - hw * 0.5f, hy = sh * 0.5f - hh * 0.5f;
        textRenderer.rect(hx, hy, hw, hh, glm::vec4(0.03f, 0.035f, 0.08f, 0.96f));
        textRenderer.frame(hx, hy, hw, hh, 2.0f, glm::vec4(0.26f, 0.84f, 0.96f, 0.92f));
        textRenderer.textCentered("KEYS", sw * 0.5f, hy + 16.0f, 2.6f, glm::vec4(0.96f, 0.98f, 1.0f, 1.0f), glm::vec4(0.0f));

        const char* keys[11] = {"A / D", "SPACE / J", "S", "SHIFT", "K", "F / E", "Q / X", "Z / N", "U", "T / B", "1-5 / L"};
        const char* acts[11] = {
            "LANE + BLINKERS (BIKE +40%)",
            "JUMP (2X/3X = JUMP COIN) / FLIP",
            "DUCK / AKIRA SLIDE / FLY DIVE",
            "NITRO (NITRO COIN IN CLASSIC)",
            "AKIRA BIKE <-> DELOREAN DMC-12",
            "FLY / SHIELD (COIN IN CLASSIC)",
            "SLOW-MO / EMP (TOKEN IN CLASSIC)",
            "PHASE / DASH (TOKEN IN CLASSIC)",
            "MAGNET STORM (TOKEN IN CLASSIC)",
            "8 WORLDS / 8 SKINS (1=INVIS)",
            "5 CAMS (5=AKIRA) / LIGHTS"
        };
        for (int i = 0; i < 11; ++i) {
            float ry = hy + 54.0f + (float)i * 38.0f;
            textRenderer.rect(hx + 28.0f, ry, 134.0f, 28.0f, glm::vec4(0.08f, 0.20f, 0.30f, 0.90f));
            textRenderer.frame(hx + 28.0f, ry, 134.0f, 28.0f, 1.5f, glm::vec4(0.28f, 0.86f, 0.98f, 0.85f));
            textRenderer.textCentered(keys[i], hx + 95.0f, ry + 8.0f, 1.55f, glm::vec4(0.98f, 0.92f, 0.42f, 1.0f), glm::vec4(0.0f));
            textRenderer.text(acts[i], hx + 178.0f, ry + 8.0f, 1.60f, glm::vec4(0.92f, 0.96f, 1.0f, 1.0f));
        }
        textRenderer.textCentered("H / ESC : CLOSE", sw * 0.5f, hy + hh - 26.0f, 1.50f, glm::vec4(0.40f, 0.92f, 0.82f, 1.0f), glm::vec4(0.0f));
    }

    // Постоянный счётчик FPS на экране (требование R7 и критерий G3 для демо-видео — виден на всех экранах)
    float fpsW = 104.0f;
    float fpsX = sw - fpsW - 18.0f;
    float fpsY = 82.0f;
    textRenderer.rect(fpsX, fpsY, fpsW, 24.0f, glm::vec4(0.02f, 0.03f, 0.06f, 0.82f));
    textRenderer.frame(fpsX, fpsY, fpsW, 24.0f, 1.2f, glm::vec4(0.24f, 0.88f, 0.62f, 0.82f));
    textRenderer.textCentered("FPS: " + std::to_string(currentFps), fpsX + fpsW * 0.5f, fpsY + 6.0f, 1.45f,
                              glm::vec4(0.45f, 0.98f, 0.72f, 1.0f), glm::vec4(0.0f));

    // Отправляем все накопленные 2D-прямоугольники и буквы на видеокарту за один draw call
    textRenderer.flush();

    // Возвращаем 3D-режим OpenGL перед следующим кадром
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
