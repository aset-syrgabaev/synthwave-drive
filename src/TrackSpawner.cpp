#include "Track.h"

// ============================================================================
// ПРОЦЕДУРНЫЙ ГЕНЕРАТОР ТРАССЫ (TrackSpawner.cpp) — КРИТЕРИЙ R2 И B2
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   - По мере движения вперёд (scrollDistance) динамически создаёт новые секции трассы
//     на 180 метров вперёд и удаляет проеханные объекты позади (нет утечки памяти!).
//   - Функция isSpaceFree() проверяет 3D-интервалы (dist, lane, Y), гарантируя,
//     что два твёрдых объекта НИКОГДА не появятся друг внутри друга (критерий B2).
//   - В режиме CLASSIC бонусы (Nitro, Jump, Shield, Flight, Q/X/Z/N/U) спавнятся
//     редкими монетами на дороге и на 2-м этаже ЛРТ.
// ============================================================================

// Проверяет, свободна ли полоса в диапазоне дистанций [dMin, dMax] и высот [yMin, yMax],
// чтобы два твёрдых объекта никогда не спавнились друг внутри друга
bool Track::isSpaceFree(float dMin, float dMax, int lane, float yMin, float yMax) const {
    for (const Entity& e : entities) {
        if (!e.alive) continue;
        if (e.type == EntityType::Tunnel || e.type == EntityType::Coin) continue;
        if (e.lane != lane && e.type != EntityType::MovingBarrier) continue;
        float eMinD = e.dist - e.length * 0.5f - 1.2f;
        float eMaxD = e.dist + e.length * 0.5f + 1.2f;
        if (dMax < eMinD || dMin > eMaxD) continue;

        float eMinY = e.y;
        float eMaxY = e.y + 1.6f;
        if (e.type == EntityType::Truck || e.type == EntityType::Ramp) { eMinY = 0.0f; eMaxY = kTrainHeight + 0.3f; }
        else if (e.type == EntityType::LrtPlatform) { eMinY = kLrtHeight - kLrtThickness; eMaxY = kLrtHeight + 0.3f; }
        else if (e.type == EntityType::LrtTrain) { eMinY = kLrtHeight; eMaxY = kLrtHeight + kLrtTrainHeight + 0.2f; }
        else if (e.type == EntityType::SkyCatwalk) { eMinY = kSkyCatwalkHeight - kCatwalkThickness; eMaxY = kSkyCatwalkHeight + 0.3f; }
        else if (e.type == EntityType::Gate) { eMinY = 0.0f; eMaxY = 2.4f; }

        if (!(yMax < eMinY || yMin > eMaxY)) return false;
    }
    return true;
}

// Безопасное добавление монеты (не ставит монету внутрь поезда или плиты LRT)
void Track::addSafeCoin(float d, int lane, float y) {
    if (isSpaceFree(d - 0.5f, d + 0.5f, lane, y - 0.35f, y + 0.35f)) {
        add(EntityType::Coin, d, lane, y);
    }
}

void Track::coinLine(float d, int lane, int n, float gap, float y) {
    for (int i = 0; i < n; ++i) addSafeCoin(d + (float)i * gap, lane, y);
}

void Track::coinArc(float d, int lane, float baseY, float peakY) {
    const float ys[5] = {
        baseY,
        baseY + (peakY - baseY) * 0.65f,
        peakY,
        baseY + (peakY - baseY) * 0.65f,
        baseY
    };
    for (int i = 0; i < 5; ++i) addSafeCoin(d - 4.0f + (float)i * 2.0f, lane, ys[i]);
}

// Спавнит состав из 1-2 вагонов с наклонной рампой заезда (с проверкой свободного места)
void Track::addTrainWithRamp(float d, int lane, int colorIdx, bool withCoins, int numCars) {
    float trainLen = 12.0f;
    float rampLen  = 7.5f;
    float rampCenter = d - trainLen * 0.5f - rampLen * 0.5f + 0.2f;
    float totalMin = rampCenter - rampLen * 0.5f;
    float totalMax = d + (float)(numCars - 1) * (trainLen + 0.5f) + trainLen * 0.5f;
    if (!isSpaceFree(totalMin, totalMax, lane, 0.0f, kTrainHeight + 0.5f)) return;

    add(EntityType::Ramp, rampCenter, lane, 0.0f, colorIdx, rampLen, 0.0f);
    for (int c = 0; c < numCars; ++c) {
        float carCenter = d + (float)c * (trainLen + 0.5f);
        add(EntityType::Truck, carCenter, lane, 0.0f, (colorIdx + c) % 5, trainLen, 0.0f);
        if (withCoins) {
            coinLine(carCenter - trainLen * 0.36f, lane, 6, 1.75f, kTrainHeight + 0.70f);
        }
    }
}

void Track::addStationaryTrain(float d, int lane, int colorIdx, bool roofCoins) {
    float trainLen = 11.4f;
    if (!isSpaceFree(d - trainLen * 0.5f, d + trainLen * 0.5f, lane, 0.0f, kTrainHeight + 0.5f)) return;
    add(EntityType::Truck, d, lane, 0.0f, colorIdx, trainLen, 0.0f);
    if (roofCoins) {
        coinLine(d - trainLen * 0.34f, lane, 6, 1.75f, kTrainHeight + 0.70f);
    }
}

void Track::addMovingTrain(float d, int lane, int colorIdx) {
    float trainLen = 11.4f;
    // Движущемуся поезду нужен чистый коридор впереди и сзади, чтобы он не проходил сквозь стоящие объекты
    if (!isSpaceFree(d - 22.0f, d + 12.0f, lane, 0.0f, kTrainHeight + 0.5f)) return;
    add(EntityType::Truck, d, lane, 0.0f, colorIdx, trainLen, 10.5f);
}

// Спавнит полноценную надземную эстакаду LRT на высоте kLrtHeight (3.85м):
// - перед передним краем в открытом воздухе ставим трамплин (или можно запрыгнуть двойным прыжком до моста/сбоку),
// - снизу плита твёрдая (пробить потолок снизу невозможно),
// - наверху едет надземный поезд LRT и лежат редкие токены способностей для режима Classic
void Track::addLrtViaduct(float d, int lane, float len, bool withEntryJumpPad, bool withLrtTrain, bool withUpperCatwalk, bool classicMode) {
    float halfLen = len * 0.5f;
    if (!isSpaceFree(d - halfLen - 1.0f, d + halfLen + 1.0f, lane, kLrtHeight - kLrtThickness, kLrtHeight + 2.5f)) return;

    add(EntityType::LrtPlatform, d, lane, kLrtHeight, 0, len, 0.0f);

    // Трамплин заезда стоит СТРОГО ПЕРЕД эстакадой в открытом пространстве (не под потолком)
    if (withEntryJumpPad) {
        float jpDist = d - halfLen - 7.5f;
        if (isSpaceFree(jpDist - 2.0f, jpDist + 2.0f, lane, 0.0f, 1.8f)) {
            add(EntityType::JumpPad, jpDist, lane, 0.0f);
        }
    }

    // Монеты вдоль рельсов LRT наверху
    coinLine(d - halfLen + 3.0f, lane, 6, 2.4f, kLrtHeight + 0.85f);

    // Надземный монорельсовый вагон LRT наверху эстакады (можно перепрыгнуть или запрыгнуть на его крышу)
    if (withLrtTrain && len >= 24.0f) {
        add(EntityType::LrtTrain, d + 3.5f, lane, kLrtHeight, ri(3), 10.5f, 0.0f);
        coinLine(d + 0.0f, lane, 4, 2.0f, kLrtHeight + kLrtTrainHeight + 0.72f);
    } else if (rnd() < 0.50f) {
        add(EntityType::BoostPad, d - 2.0f, lane, kLrtHeight, 0, 1.0f, 0.0f);
    }

    // Награда за паркур на LRT: редкий бонус-коин (токен способности, нитро, двойной прыжок, щит или полёт)
    float rewardY = withLrtTrain && len >= 24.0f ? (kLrtHeight + kLrtTrainHeight + 0.95f) : (kLrtHeight + 1.05f);
    float rewardD = d + halfLen - 4.0f;
    float roll = rnd();
    if (roll < 0.34f) {
        add(EntityType::TechToken, rewardD, lane, rewardY, ri(5));
    } else if (roll < 0.58f) {
        add(EntityType::NitroOrb, rewardD, lane, rewardY);
    } else if (roll < 0.76f) {
        add(EntityType::SneakersOrb, rewardD, lane, rewardY);
    } else if (roll < 0.90f) {
        add(EntityType::ShieldOrb, rewardD, lane, rewardY);
    } else {
        add(EntityType::FlightOrb, rewardD, lane, rewardY);
    }

    // Второй ярус: высотный мост SkyCatwalk (5.75м) на соседней полосе — под прыжок с LRT или тройной прыжок Акиры
    if (withUpperCatwalk) {
        int nextLane = (lane + 1 + ri(2)) % 3;
        float cwDist = d + halfLen + 12.5f;
        float cwLen = 18.0f;
        if (isSpaceFree(cwDist - cwLen * 0.5f, cwDist + cwLen * 0.5f, nextLane, kSkyCatwalkHeight - kCatwalkThickness, kSkyCatwalkHeight + 2.0f)) {
            add(EntityType::SkyCatwalk, cwDist, nextLane, kSkyCatwalkHeight, 0, cwLen, 0.0f);
            coinLine(cwDist - cwLen * 0.32f, nextLane, 5, 2.2f, kSkyCatwalkHeight + 0.85f);
            add(EntityType::TechToken, cwDist + cwLen * 0.30f, nextLane, kSkyCatwalkHeight + 1.05f, ri(5));
        }
    }
    (void)classicMode;
}

bool Track::letterAlive() const {
    for (const Entity& e : entities)
        if (e.alive && e.type == EntityType::Letter) return true;
    return false;
}

// Спавнит один тактический сектор трассы (в Classic бонусы на земле редкие, ценные токены на LRT и в сложных полосах)
void Track::spawnRow(float d, char neededChar, bool flying, bool classicMode) {
    int a = ri(3);
    int b = (a + 1 + ri(2)) % 3;
    int c = 3 - a - b;
    rowCounter++;
    if (classicPickupCooldown > 0) classicPickupCooldown--;

    float cyc = std::fmod(d, kTunnelCycle);
    bool onTunnelSlope = (cyc > 140.0f && cyc < 188.0f) || (cyc > 380.0f && cyc < 428.0f);
    bool inDeepTunnel  = (cyc >= 188.0f && cyc <= 380.0f);

    if (onTunnelSlope) {
        if (isSpaceFree(d - 2.0f, d + 2.0f, a, 0.0f, 1.5f)) add(EntityType::Barrier, d, a);
        coinArc(d, a);
        coinLine(d - 3.0f, b, 5, 1.8f, 0.78f);
    } else if (inDeepTunnel) {
        int tPat = ri(5);
        if (tPat == 0) {
            addStationaryTrain(d, a, ri(5), false);
            if (isSpaceFree(d - 2.0f, d + 2.0f, b, 0.0f, 1.5f)) add(EntityType::Barrier, d, b);
            coinLine(d - 3.5f, c, 6, 1.75f, 0.78f);
        } else if (tPat == 1) {
            add(EntityType::MovingBarrier, d, 1);
            coinArc(d, b);
        } else if (tPat == 2) {
            add(EntityType::Gate, d, a);
            add(EntityType::Spinner, d, b);
            coinLine(d - 3.5f, c, 5, 1.75f, 0.78f);
        } else if (tPat == 3) {
            addMovingTrain(d + 6.0f, a, ri(5));
            coinLine(d - 3.5f, b, 6, 1.75f, 0.78f);
        } else {
            addTrainWithRamp(d + 2.0f, a, ri(5), true, 1);
            add(EntityType::Gate, d, b);
            coinLine(d - 3.0f, c, 5, 1.75f, 0.78f);
        }
    } else {
        // 18 разнообразных паттернов с настоящими эстакадами LRT наверху, поездами и лазерами
        int pat = ri(18);
        if (pat == 0) {
            addTrainWithRamp(d + 4.0f, a, ri(5), true, 2);
            add(EntityType::Barrier, d, b);
            add(EntityType::LaserSweep, d + 11.0f, c, 0.0f, 1);
            coinArc(d, b);
            nextRow += 12.0f;
        } else if (pat == 1) {
            // Эстакада LRT (3.85м) с надземным поездом LRT наверху и трамплином перед заездом
            addLrtViaduct(d + 16.0f, b, 26.0f, true, true, true, classicMode);
            addStationaryTrain(d + 4.0f, a, ri(5), true);
            add(EntityType::Barrier, d + 8.0f, c);
            nextRow += 26.0f;
        } else if (pat == 2) {
            add(EntityType::LaserSweep, d, a, 0.0f, 0);
            add(EntityType::LaserSweep, d, b, 0.0f, 1);
            add(EntityType::Spinner, d, c);
            coinLine(d - 3.5f, b, 5, 1.7f, 0.65f);
        } else if (pat == 3) {
            addMovingTrain(d + 10.0f, a, ri(5));
            addMovingTrain(d + 20.0f, c, ri(5));
            coinLine(d - 3.5f, b, 6, 1.75f, 0.78f);
            add(EntityType::Barrier, d + 3.0f, b);
        } else if (pat == 4) {
            // Центральная линия LRT сверху над шоссе (запрыгиваем двойным прыжком до моста или с крыши соседнего поезда)
            addLrtViaduct(d + 14.0f, 1, 24.0f, false, false, false, classicMode);
            addTrainWithRamp(d + 2.0f, 0, ri(5), true, 1);
            addStationaryTrain(d + 6.0f, 2, ri(5), false);
            add(EntityType::LaserSweep, d + 14.0f, 1, 0.0f, 1);
            nextRow += 16.0f;
        } else if (pat == 5) {
            add(EntityType::Barrier, d, a);
            add(EntityType::Gate, d, b);
            add(EntityType::Spinner, d, c);
            coinArc(d, a);
        } else if (pat == 6) {
            addTrainWithRamp(d + 3.0f, 1, ri(5), true, 2);
            addMovingTrain(d + 24.0f, a == 1 ? 0 : a, ri(5));
            addLrtViaduct(d + 24.0f, 1, 22.0f, false, false, true, classicMode);
            nextRow += 22.0f;
        } else if (pat == 7) {
            add(EntityType::MovingBarrier, d, 1);
            coinArc(d, b);
            add(EntityType::LaserSweep, d + 9.0f, c, 0.0f, 0);
            add(EntityType::Gate, d + 9.0f, a);
        } else if (pat == 8) {
            // Высотная связка LRT -> SkyCatwalk с трамплином в открытом воздухе перед заездом
            addLrtViaduct(d + 14.0f, b, 24.0f, true, false, true, classicMode);
            add(EntityType::Spinner, d + 6.0f, a);
            add(EntityType::Barrier, d + 10.5f, c);
            nextRow += 22.0f;
        } else if (pat == 9) {
            addTrainWithRamp(d + 2.0f, a, ri(5), true, 1);
            addTrainWithRamp(d + 2.0f, c, ri(5), true, 1);
            add(EntityType::LaserSweep, d + 2.0f, b, 0.0f, 1);
        } else if (pat == 10) {
            add(EntityType::Spinner, d, a);
            add(EntityType::MovingBarrier, d + 6.5f, b);
            add(EntityType::LaserSweep, d + 6.5f, c, 0.0f, 0);
            coinLine(d - 3.0f, c, 6, 1.75f, 0.78f);
        } else if (pat == 11) {
            add(EntityType::Gate, d, a);
            add(EntityType::Gate, d, b);
            add(EntityType::LaserSweep, d, c, 0.0f, 0);
            coinLine(d - 3.5f, c, 5, 1.75f, 0.78f);
        } else if (pat == 12) {
            addTrainWithRamp(d + 3.0f, a, ri(5), true, 2);
            addLrtViaduct(d + 18.0f, b, 24.0f, false, true, false, classicMode);
            coinLine(d - 3.0f, c, 6, 1.75f, 0.78f);
            nextRow += 18.0f;
        } else if (pat == 13) {
            // Две последовательные эстакады LRT на разных полосах (перепрыгиваем с одной линии на другую)
            addLrtViaduct(d + 12.0f, a, 20.0f, true, false, false, classicMode);
            addLrtViaduct(d + 30.0f, b, 24.0f, false, true, true, classicMode);
            add(EntityType::Barrier, d + 12.0f, c);
            nextRow += 32.0f;
        } else if (pat == 14) {
            add(EntityType::LaserSweep, d, a, 0.0f, 0);
            add(EntityType::LaserSweep, d + 9.0f, a, 0.0f, 1);
            add(EntityType::MovingBarrier, d + 4.5f, b);
            addStationaryTrain(d + 4.5f, c, ri(5), true);
            nextRow += 9.0f;
        } else if (pat == 15) {
            // Эстакада LRT справа/слева + поезд на соседней полосе (не пересекаются)
            addStationaryTrain(d + 2.0f, a, ri(5), true);
            addLrtViaduct(d + 14.0f, b, 24.0f, true, false, false, classicMode);
            add(EntityType::Spinner, d + 6.0f, c);
            nextRow += 16.0f;
        } else if (pat == 16) {
            addMovingTrain(d + 16.0f, a, ri(5));
            add(EntityType::Barrier, d + 2.0f, b);
            add(EntityType::LaserSweep, d + 9.0f, b, 0.0f, 1);
            add(EntityType::Spinner, d + 4.0f, c);
        } else {
            addTrainWithRamp(d + 2.0f, a, ri(5), true, 1);
            coinLine(d - 3.5f, b, 5, 1.75f, 0.78f);
            add(EntityType::LaserSweep, d + 2.0f, c, 0.0f, 0);
        }
    }

    // Буква текущего слова появляется на свободной полосе
    if (!letterAlive() && rnd() < 0.65f) {
        int lLane = ri(3);
        if (isSpaceFree(d + 9.0f, d + 11.5f, lLane, 0.8f, 2.0f)) {
            add(EntityType::Letter, d + 10.2f, lLane, 1.30f, 0, 1.0f, 0.0f, neededChar);
        }
    }

    // Редкий спавн бонус-коинов на земле:
    // в Classic все ништяки (нитро, двойной/тройной прыжок, щит, полёт, способности)
    // выдаются только через эти редкие бонус-коины на трассе (не чаще раза в 3-4 секции)
    float bonusChance = classicMode ? 0.28f : 0.42f;
    if (classicPickupCooldown == 0 && rnd() < bonusChance) {
        int pLane = ri(3);
        float pd = d + 12.5f;
        if (isSpaceFree(pd - 1.5f, pd + 1.5f, pLane, 0.6f, 2.0f)) {
            float pRoll = rnd();
            if (pRoll < 0.26f) {
                // Нитро-коин (даёт запас ускорения SHIFT в Classic)
                add(EntityType::NitroOrb, pd, pLane, 1.24f);
            } else if (pRoll < 0.46f) {
                // Бонус-коин прыжка (открывает двойной / тройной прыжок на 18с в Classic)
                add(EntityType::SneakersOrb, pd, pLane, 1.24f);
            } else if (pRoll < 0.70f) {
                // Спец-токен способностей (заряжает Q / X / Z / N / U)
                add(EntityType::TechToken, pd, pLane, 1.24f, ri(5));
            } else if (pRoll < 0.82f) {
                // Энергоблок щита [E]
                add(EntityType::ShieldOrb, pd, pLane, 1.24f);
            } else if (!flying && !inDeepTunnel && !onTunnelSlope && pRoll < 0.91f) {
                // Энергоблок полёта [F]
                add(EntityType::FlightOrb, pd, pLane, 1.24f);
            } else if (pRoll < 0.96f) {
                add(EntityType::MultiplierOrb, pd, pLane, 1.24f);
            } else {
                add(EntityType::AkiraCapsule, pd, pLane, 1.24f);
            }
            classicPickupCooldown = classicMode ? 3 : 1;
        }
    }

    // Полоса ускорения (BoostPad) ставится только на гарантированно свободный участок дороги
    float boostChance = classicMode ? 0.14f : 0.28f;
    if (!flying && rnd() < boostChance) {
        int bpLane = ri(3);
        float bpd = d - 6.5f;
        if (isSpaceFree(bpd - 2.2f, bpd + 2.2f, bpLane, 0.0f, 1.6f)) {
            add(EntityType::BoostPad, bpd, bpLane, 0.0f);
        }
    }
}


// Подгружает новые сегменты туннеля, секции трассы и воздушные объекты впереди игрока
void Track::ensure(float playerDist, bool flying, float flightWorldY, char neededChar, float difficulty, bool classicMode) {
    while ((float)nextTunnelBlock * kTunnelCycle + 185.0f < playerDist + 260.0f) {
        float baseZ = (float)nextTunnelBlock * kTunnelCycle;
        for (int seg = 0; seg < 5; ++seg) {
            add(EntityType::Tunnel, baseZ + 205.0f + (float)seg * 40.0f, 1, 0.0f, seg, 40.2f, 0.0f);
        }
        nextTunnelBlock++;
    }

    while (nextRow < playerDist + 215.0f) {
        spawnRow(nextRow, neededChar, flying, classicMode);
        float gap = std::max(18.5f, 25.5f - difficulty * 0.35f) + rnd() * 5.0f;
        nextRow += gap;
    }

    if (flying) {
        static int skyStep = 0;
        while (flightCursor < playerDist + 165.0f) {
            int lane = static_cast<int>(std::round(std::sin(flightCursor * 0.065f) + 1.0f));
            lane = std::clamp(lane, 0, 2);
            float relY = flightWorldY - tunnelDepthAt(flightCursor);
            // Помечаем воздушные монеты полёта индексом 99
            entities.push_back({EntityType::Coin, flightCursor, laneX(lane), relY, 1.0f, 0.0f, lane, 99, 'C', true, false, false});

            // В Classic количество воздушных колец ускорения строго ограничено (flightRingsLeft)
            if (skyStep % 9 == 4 && flightRingsLeft > 0) {
                entities.push_back({EntityType::SkyRing, flightCursor, laneX(lane), relY, 1.0f, 0.0f, lane, skyStep % 5, 'R', true, false, false});
                if (classicMode) flightRingsLeft--;
            } else if (skyStep % 8 == 7) {
                int droneLane = (lane + 1 + (skyStep % 2)) % 3;
                entities.push_back({EntityType::SkyDrone, flightCursor, laneX(droneLane), relY, 1.4f, 0.0f, droneLane, 0, 'D', true, false, false});
            }
            skyStep++;
            flightCursor += 3.8f;
        }
    }
}

