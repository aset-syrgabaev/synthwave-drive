#include "Track.h"

// Физика твёрдых опор, потолков LRT, бокового выталкивания и AABB-коллизий

// Повторяет формулу из terrain.vert: спуск 150..185м, туннель 185..385м, подъём 385..420м
float Track::tunnelDepthAt(float trackDist) {
    float c = std::fmod(trackDist, kTunnelCycle);
    if (c < 0.0f) c += kTunnelCycle;
    float down = smoothStep(150.0f, 185.0f, c);
    float up   = smoothStep(385.0f, 420.0f, c);
    return -6.0f * (down - up);
}

// Есть ли на данной дистанции верхняя плита перекрытия туннеля (на уровне y = 0.0)
bool Track::hasTunnelRoofAt(float trackDist) {
    float c = std::fmod(trackDist, kTunnelCycle);
    if (c < 0.0f) c += kTunnelCycle;
    return (c >= 184.0f && c <= 386.0f);
}

// Сброс всех объектов трассы перед новым заездом
void Track::reset(float playerDist) {
    entities.clear();
    nextRow = playerDist + 26.0f;
    flightCursor = playerDist;
    nextTunnelBlock = 0;
    rowCounter = 0;
    flightRingsLeft = 0;
    classicPickupCooldown = 1;
}

// Разрушает препятствия впереди и притягивает монеты при нажатии EMP [X]
int Track::empPulse(float playerDist, float radius) {
    int destroyed = 0;
    for (Entity& e : entities) {
        if (!e.alive) continue;
        float dz = e.dist - playerDist;
        if (dz > -6.0f && dz < radius) {
            if (e.type == EntityType::Barrier || e.type == EntityType::MovingBarrier ||
                e.type == EntityType::Gate || e.type == EntityType::Spinner ||
                e.type == EntityType::LaserSweep || e.type == EntityType::SkyDrone) {
                e.alive = false;
                destroyed++;
            } else if (e.type == EntityType::Coin && dz < 36.0f) {
                e.pulled = true;
            }
        }
    }
    return destroyed;
}

// Расчищает ближайшие препятствия после срабатывания щита или продолжения игры, чтобы не врезаться сразу снова
void Track::clearNearbyObstacles(float playerDist, float radius) {
    for (Entity& e : entities) {
        if (!e.alive) continue;
        float dz = e.dist - playerDist;
        if (dz > -8.0f && dz < radius) {
            if (e.type == EntityType::Barrier || e.type == EntityType::MovingBarrier ||
                e.type == EntityType::Truck || e.type == EntityType::Gate ||
                e.type == EntityType::Spinner || e.type == EntityType::LaserSweep ||
                e.type == EntityType::SkyDrone) {
                e.alive = false;
            }
        }
    }
}

// Старт полёта [F]: в Classic выделяем строго 4 кольца ускорения за полёт, в Free Play — без лимита
void Track::startFlight(float playerDist, bool classicMode) {
    flightCursor = std::max(flightCursor, playerDist + 14.0f);
    flightRingsLeft = classicMode ? 4 : 999;
}

// Окончание полёта: перестаём спавнить воздушные кольца
void Track::endFlight() {
    flightRingsLeft = 0;
}

// Главный физический шаг трассы (вызывается каждый кадр):
// 1) двигает встречные поезда и движущиеся барьеры;
// 2) ищет твёрдую опору под колёсами (worldFloorY: дорога, рампа, поезд, ЛРТ 3.85м, мост 5.75м);
// 3) ищет твёрдый потолок сверху (worldCeilingY = 3.23м под бетонной балкой ЛРТ, чтобы не пробить её снизу);
// 4) проверяет AABB-столкновения с препятствиями и подбор всех монет/бонусов.
TrackEvents Track::update(float playerDist, float px, float carWorldY, float carBottomWorldY, bool ducking, bool flying,
                          bool invulnerable, bool magnet, bool switchingLane, float time, float dt,
                          bool phaseShift, bool bikeMode, bool smashActive, bool barrelRolling) {
    TrackEvents ev;
    ev.roadElevation = tunnelDepthAt(playerDist);
    ev.worldFloorY = ev.roadElevation;
    ev.worldCeilingY = 999.0f;

    // Если игрок едет наверху над подземным туннелем (y = 0.0), держим его на крыше эстакады
    bool roofHere = hasTunnelRoofAt(playerDist);
    if (roofHere && carBottomWorldY >= -0.60f) {
        ev.worldFloorY = 0.0f;
        ev.onTunnelRoof = true;
    }

    // Реальные габариты транспорта (в приседе высота уменьшается до 0.72м, мотоцикл уже машины)
    float vehicleHeight = ducking ? 0.72f : (bikeMode ? 1.15f : 1.24f);
    float vehicleHalfW  = bikeMode ? 0.44f : 0.88f;
    float carWorldCenterY = carWorldY + 0.16f;
    float carTopWorldY = carBottomWorldY + vehicleHeight;

    // Двигаем встречные поезда, скользящие барьеры и поднимаем бонусы на уровень полёта
    for (Entity& e : entities) {
        if (!e.alive) continue;
        if (e.speed > 0.0f && (e.dist - playerDist) < 150.0f) {
            e.dist -= e.speed * dt;
        }
        if (e.type == EntityType::MovingBarrier) {
            e.x = std::sin(time * 2.4f + e.dist * 0.1f) * 2.35f;
        }

        // В полёте предметы поднимаются на высоту полёта, после посадки — опускаются обратно только монеты полёта (index 99)
        float dz = e.dist - playerDist;
        if (flying && dz > -2.5f && dz < 95.0f) {
            if (e.type == EntityType::Coin || e.type == EntityType::Letter || e.type == EntityType::MagnetOrb ||
                e.type == EntityType::MultiplierOrb || e.type == EntityType::MysteryBox ||
                e.type == EntityType::AkiraCapsule || e.type == EntityType::ShieldOrb ||
                e.type == EntityType::TechToken || e.type == EntityType::NitroOrb ||
                e.type == EntityType::SneakersOrb) {
                float targetRelY = carWorldCenterY - tunnelDepthAt(e.dist);
                e.y += (targetRelY - e.y) * std::min(1.0f, 9.5f * dt);
            }
        } else if (!flying && dz > -2.5f && dz < 160.0f) {
            if (e.type == EntityType::Coin && e.index == 99 && e.y > 3.1f) {
                e.y += (0.78f - e.y) * std::min(1.0f, 8.0f * dt);
            } else if (e.type == EntityType::Letter && e.y > 2.2f && e.y < 3.6f) {
                e.y += (1.30f - e.y) * std::min(1.0f, 8.0f * dt);
            }
        }
    }

    // 1) Заезд на наклонную рампу
    bool onRampInLane = false;
    for (const Entity& e : entities) {
        if (!e.alive || flying || ev.onTunnelRoof) continue;
        if (e.type == EntityType::Ramp && std::fabs(e.x - px) < 1.18f) {
            float dz = e.dist - playerDist;
            float halfLen = e.length * 0.5f;
            if (dz >= -halfLen - 1.9f && dz <= halfLen + 1.4f) {
                float t = std::clamp((halfLen + 1.0f - dz) / (e.length + 1.0f), 0.0f, 1.0f);
                float rampTopWorldY = ev.roadElevation + t * kTrainHeight;
                if (rampTopWorldY > ev.worldFloorY) {
                    ev.worldFloorY = rampTopWorldY;
                    ev.onRamp = true;
                    onRampInLane = true;
                }
            }
        }
    }

    // 2) Твёрдые поверхности и твёрдые потолки: поезда метро, эстакады LRT, поезда LRT и высотные мосты
    for (const Entity& e : entities) {
        if (!e.alive || flying || ev.onTunnelRoof) continue;
        float dz = e.dist - playerDist;
        float halfLen = e.length * 0.5f;
        float dx = std::fabs(e.x - px);

        if (e.type == EntityType::Truck && dx < (1.02f + vehicleHalfW * 0.45f)) {
            if (dz >= -halfLen - 1.25f && dz <= halfLen + 1.25f) {
                float trainTopWorldY = ev.roadElevation + kTrainHeight;
                if (carBottomWorldY >= trainTopWorldY - 0.55f || ev.worldFloorY >= trainTopWorldY - 0.55f || onRampInLane) {
                    ev.worldFloorY = std::max(ev.worldFloorY, trainTopWorldY);
                }
            }
        } else if (e.type == EntityType::LrtPlatform || e.type == EntityType::SkyCatwalk) {
            float platHalfW = (e.type == EntityType::LrtPlatform) ? 1.16f : 1.08f;
            float slabThick = (e.type == EntityType::LrtPlatform) ? kLrtThickness : kCatwalkThickness;
            if (dx < platHalfW + vehicleHalfW * 0.45f && dz >= -halfLen - 1.25f && dz <= halfLen + 1.25f) {
                float platTopWorldY = ev.roadElevation + e.y;
                float platBotWorldY = platTopWorldY - slabThick;

                // Сверху плита держит транспорт, если колёса уже выше её верхней кромки (с малым допуском на посадку)
                if (carBottomWorldY >= platTopWorldY - 0.34f) {
                    if (platTopWorldY > ev.worldFloorY) {
                        ev.worldFloorY = platTopWorldY;
                        ev.onLrt = true;
                    }
                } else if (carBottomWorldY < platBotWorldY + 0.10f) {
                    // Снизу это ТВЁРДЫЙ БЕТОННЫЙ ПОТОЛОК: при прыжке под эстакадой транспорт упирается в него
                    // и НИКОГДА не проходит вверх сквозь перекрытие
                    ev.worldCeilingY = std::min(ev.worldCeilingY, platBotWorldY);
                }
            }
        } else if (e.type == EntityType::LrtTrain && dx < (0.98f + vehicleHalfW * 0.45f)) {
            if (dz >= -halfLen - 1.2f && dz <= halfLen + 1.2f) {
                float topY = ev.roadElevation + e.y + kLrtTrainHeight;
                if (carBottomWorldY >= topY - 0.55f) {
                    ev.worldFloorY = std::max(ev.worldFloorY, topY);
                    ev.onLrt = true;
                }
            }
        }
    }

    // 3) Сбор предметов и точные столкновения без проваливания моделей друг в друга
    for (Entity& e : entities) {
        if (!e.alive) continue;
        float dz = e.dist - playerDist;
        float dx = std::fabs(e.x - px);
        float eRoadY = tunnelDepthAt(e.dist);
        float entityWorldY = eRoadY + e.y;

        switch (e.type) {
        case EntityType::Coin: {
            if (!e.pulled && dz > -1.0f && ((magnet && dz < 26.0f) || (flying && dx < 1.55f && dz < 14.0f))) {
                e.pulled = true;
            }
            if (e.pulled) {
                float k = std::min(1.0f, 14.0f * dt);
                e.x += (px - e.x) * k;
                float targetRelY = carWorldCenterY - eRoadY;
                e.y += (targetRelY - e.y) * k;
                e.dist -= dz * k * 0.78f;
                dz = e.dist - playerDist;
                if (dz < 1.5f) { e.alive = false; ev.coins++; }
            } else if (std::fabs(dz) < 1.45f && dx < 1.25f && std::fabs(entityWorldY - carWorldCenterY) < 1.55f) {
                e.alive = false;
                ev.coins++;
            }
            break;
        }
        case EntityType::Letter:
            if (std::fabs(dz) < 1.55f && dx < 1.30f && std::fabs(entityWorldY - carWorldCenterY) < 1.70f) {
                e.alive = false;
                ev.collectedLetter = e.letterChar;
            }
            break;
        case EntityType::FlightOrb:
            if (std::fabs(dz) < 1.55f && dx < 1.30f && std::fabs(entityWorldY - carWorldCenterY) < 1.65f) {
                e.alive = false;
                ev.flightOrb = true;
            }
            break;
        case EntityType::ShieldOrb:
            if (std::fabs(dz) < 1.55f && dx < 1.30f && std::fabs(entityWorldY - carWorldCenterY) < 1.65f) {
                e.alive = false;
                ev.shieldOrb = true;
            }
            break;
        case EntityType::NitroOrb:
            if (std::fabs(dz) < 1.55f && dx < 1.30f && std::fabs(entityWorldY - carWorldCenterY) < 1.65f) {
                e.alive = false;
                ev.nitroOrb = true;
            }
            break;
        case EntityType::TechToken:
            if (std::fabs(dz) < 1.55f && dx < 1.30f && std::fabs(entityWorldY - carWorldCenterY) < 1.65f) {
                e.alive = false;
                ev.techTokenType = e.index;
            }
            break;
        case EntityType::MagnetOrb:
            if (std::fabs(dz) < 1.55f && dx < 1.30f && std::fabs(entityWorldY - carWorldCenterY) < 1.65f) {
                e.alive = false;
                ev.magnetOrb = true;
            }
            break;
        case EntityType::SneakersOrb:
            if (std::fabs(dz) < 1.55f && dx < 1.30f && std::fabs(entityWorldY - carWorldCenterY) < 1.65f) {
                e.alive = false;
                ev.sneakersOrb = true;
            }
            break;
        case EntityType::MultiplierOrb:
            if (std::fabs(dz) < 1.55f && dx < 1.30f && std::fabs(entityWorldY - carWorldCenterY) < 1.65f) {
                e.alive = false;
                ev.multiplierOrb = true;
            }
            break;
        case EntityType::MysteryBox:
            if (std::fabs(dz) < 1.60f && dx < 1.32f && std::fabs(entityWorldY - carWorldCenterY) < 1.70f) {
                e.alive = false;
                ev.mysteryBox = true;
            }
            break;
        case EntityType::JumpPad:
            if (!flying && !e.hit && std::fabs(dz) < 1.50f && dx < 1.18f && std::fabs(carBottomWorldY - eRoadY) < 0.85f) {
                e.hit = true;
                ev.jumpPad = true;
            }
            break;
        case EntityType::BoostPad:
            if (!flying && !e.hit && std::fabs(dz) < 1.65f && dx < 1.22f && std::fabs(carBottomWorldY - entityWorldY) < 0.90f) {
                e.hit = true;
                ev.boostPad = true;
            }
            break;
        case EntityType::LaserSweep: {
            bool lowBeam = (e.index == 0);
            float beamBot = entityWorldY + (lowBeam ? 0.12f : 1.04f);
            float beamTop = entityWorldY + (lowBeam ? 0.88f : 1.85f);
            float hitW = bikeMode ? 0.86f : 1.10f;
            if (!flying && !e.hit && std::fabs(dz) < 1.15f && dx < hitW && carBottomWorldY < beamTop && carTopWorldY > beamBot) {
                if (smashActive) {
                    e.hit = true;
                    e.alive = false;
                    ev.smashedCount++;
                } else if (phaseShift) {
                    e.hit = true;
                    ev.phasedCount++;
                } else if (!invulnerable) {
                    e.hit = true;
                    ev.crash = true;
                }
            }
            break;
        }
        case EntityType::LrtPlatform:
        case EntityType::SkyCatwalk: {
            // Проверяем лобовой удар в торец бетонной плиты LRT (если игрок прыгнул недостаточно высоко)
            float halfLen = e.length * 0.5f;
            float platTop = eRoadY + e.y;
            float platBot = platTop - ((e.type == EntityType::LrtPlatform) ? kLrtThickness : kCatwalkThickness);
            float platHalfW = (e.type == EntityType::LrtPlatform) ? 1.16f : 1.08f;
            if (!flying && !phaseShift && !invulnerable && !e.hit &&
                dx < platHalfW + vehicleHalfW * 0.35f &&
                dz > halfLen - 1.6f && dz < halfLen + 1.1f &&
                carBottomWorldY < platTop - 0.34f && carTopWorldY > platBot + 0.18f) {
                e.hit = true;
                ev.stumble = true;
            }
            break;
        }
        case EntityType::LrtTrain: {
            // Надземный вагон LRT наверху эстакады: можно перепрыгнуть или удариться, если едешь прямо в него на LRT
            float halfLen = e.length * 0.5f;
            float trainBot = eRoadY + e.y;
            float trainTop = trainBot + kLrtTrainHeight;
            float solidW = 0.96f + vehicleHalfW;
            bool safeAbove = (carBottomWorldY >= trainTop - 0.55f);
            if (!flying && !e.hit && std::fabs(dz) < halfLen + 1.15f && dx < solidW &&
                carBottomWorldY >= trainBot - 0.25f && !safeAbove && carTopWorldY > trainBot + 0.1f) {
                if (phaseShift) {
                    e.hit = true;
                    ev.phasedCount++;
                } else if (!invulnerable) {
                    e.hit = true;
                    ev.crash = true;
                }
            }
            break;
        }
        case EntityType::AkiraCapsule:
            if (std::fabs(dz) < 1.60f && dx < 1.35f && std::fabs(entityWorldY - carWorldCenterY) < 1.75f) {
                e.alive = false;
                ev.akiraCapsule = true;
            }
            break;
        case EntityType::SkyRing:
            if (flying && !e.hit && std::fabs(dz) < 1.95f && dx < 1.65f && std::fabs(entityWorldY - carWorldCenterY) < 1.95f) {
                e.hit = true;
                e.alive = false;
                ev.skyRing = true;
            }
            break;
        case EntityType::SkyDrone:
            if (flying && !e.hit && std::fabs(dz) < 1.55f && dx < (bikeMode ? 0.92f : 1.12f) && std::fabs(entityWorldY - carWorldCenterY) < 1.35f) {
                if (barrelRolling || phaseShift || smashActive) {
                    e.hit = true;
                    e.alive = false;
                    ev.smashedCount++;
                } else if (!invulnerable) {
                    e.hit = true;
                    e.alive = false;
                    ev.stumble = true;
                }
            }
            break;
        case EntityType::Barrier:
        case EntityType::MovingBarrier:
        case EntityType::Spinner: {
            float obsTopY = eRoadY + 0.92f;
            float hitW = 0.92f + vehicleHalfW * 0.45f;
            if (!flying && !e.hit && std::fabs(dz) < 1.35f && dx < hitW && carBottomWorldY < obsTopY && carTopWorldY > eRoadY) {
                if (smashActive) {
                    e.hit = true;
                    e.alive = false;
                    ev.smashedCount++;
                } else if (phaseShift) {
                    e.hit = true;
                    ev.phasedCount++;
                } else if (!invulnerable) {
                    e.hit = true;
                    if (switchingLane) ev.stumble = true;
                    else ev.crash = true;
                }
            }
            break;
        }
        case EntityType::Truck: {
            // Твёрдотельная физика вагона: учитываем реальную полуширину поезда (1.04м) + полуширину транспорта,
            // чтобы мотоцикл и машина НИКОГДА не заезжали внутрь стенки вагона
            float halfLen = e.length * 0.5f;
            float trainTopWorldY = eRoadY + kTrainHeight;
            float solidHalfW = 1.04f + vehicleHalfW;
            bool safeAbove = (carBottomWorldY >= trainTopWorldY - 0.55f) || (ev.worldFloorY >= trainTopWorldY - 0.45f) || onRampInLane || ev.onTunnelRoof;
            if (!flying && !phaseShift && std::fabs(dz) < halfLen + 1.15f && dx < solidHalfW && !safeAbove && carTopWorldY > eRoadY + 0.05f) {
                // Если врезались сбоку (находясь вдоль корпуса вагона) — физически выталкиваем наружу от стенки
                bool sideBump = (dz < halfLen - 0.4f && dz > -halfLen - 0.8f && dx > 0.72f);
                if (sideBump) {
                    float pushSign = (px >= e.x) ? 1.0f : -1.0f;
                    ev.hasLateralClamp = true;
                    ev.clampedPosX = e.x + pushSign * (solidHalfW + 0.04f);
                }
                if (!e.hit && !invulnerable) {
                    e.hit = true;
                    if (sideBump || switchingLane) ev.stumble = true;
                    else ev.crash = true;
                }
            } else if (!flying && !e.hit && phaseShift && std::fabs(dz) < halfLen + 0.65f && dx < solidHalfW && !safeAbove) {
                e.hit = true;
                ev.phasedCount++;
            } else if (!flying && !e.hit && e.speed > 0.0f && !ev.onTunnelRoof && std::fabs(dz) < 1.2f && dx >= solidHalfW && dx < solidHalfW + 1.35f) {
                e.hit = true;
                ev.nearMiss = true;
            }
            break;
        }
        case EntityType::Gate: {
            float gateBottomY = eRoadY + 1.05f;
            float gateTopY    = eRoadY + 2.35f;
            if (!flying && !e.hit && std::fabs(dz) < 1.35f && dx < (0.98f + vehicleHalfW * 0.40f) && carTopWorldY > gateBottomY && carBottomWorldY < gateTopY) {
                if (phaseShift) {
                    e.hit = true;
                    ev.phasedCount++;
                } else if (!invulnerable) {
                    e.hit = true;
                    ev.crash = true;
                }
            }
            break;
        }
        case EntityType::Ramp:
        case EntityType::Tunnel:
            break;
        }
    }

    // Удаляем объекты, которые остались далеко позади игрока
    entities.erase(std::remove_if(entities.begin(), entities.end(), [&](const Entity& e) {
        return !e.alive || (e.dist + e.length * 0.5f - playerDist) < -38.0f;
    }), entities.end());

    return ev;
}
