#ifndef TRACK_H
#define TRACK_H

#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

enum class EntityType { Coin, Letter, FlightOrb, MagnetOrb, Barrier, Truck, Gate };

struct Entity {
    EntityType type;
    float dist;
    float x;
    float y;
    int index;
    bool alive;
    bool hit;
    bool pulled;
};

struct TrackEvents {
    int coins = 0;
    int letterIndex = -1;
    bool flightOrb = false;
    bool magnetOrb = false;
    bool crash = false;
};

class Track {
private:
    std::mt19937 rng;
    float nextRow = 60.0f;
    float flightCursor = 0.0f;

    float rnd() { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); }
    int ri(int n) { return static_cast<int>(rnd() * (float)n) % n; }

    void add(EntityType t, float d, int lane, float y = 0.0f, int index = 0) {
        entities.push_back({t, d, laneX(lane), y, index, true, false, false});
    }

    void coinLine(float d, int lane, int n, float gap, float y) {
        for (int i = 0; i < n; ++i) add(EntityType::Coin, d + (float)i * gap, lane, y);
    }

    void coinArc(float d, int lane) {
        const float ys[5] = {0.9f, 1.55f, 2.1f, 1.55f, 0.9f};
        for (int i = 0; i < 5; ++i) add(EntityType::Coin, d - 6.0f + (float)i * 3.0f, lane, ys[i]);
    }

    bool letterAlive() const {
        for (const Entity& e : entities)
            if (e.alive && e.type == EntityType::Letter) return true;
        return false;
    }

    void spawnRow(float d, int neededLetter, bool flying) {
        int a = ri(3);
        int b = (a + 1 + ri(2)) % 3;
        int c = 3 - a - b;

        if (!flying) {
            int pat = ri(7);
            if (pat == 0) {
                add(EntityType::Barrier, d, a);
                coinArc(d, a);
                coinLine(d - 4.0f, b, 6, 2.4f, 0.9f);
            } else if (pat == 1) {
                add(EntityType::Truck, d, a, 0.0f, ri(5));
                add(EntityType::Barrier, d, b);
                coinLine(d - 5.0f, c, 7, 2.4f, 0.9f);
            } else if (pat == 2) {
                add(EntityType::Gate, d, 1);
                coinLine(d - 4.0f, b, 5, 2.4f, 0.9f);
            } else if (pat == 3) {
                coinLine(d, a, 7, 2.5f, 0.9f);
                add(EntityType::Barrier, d + 4.0f, b);
            } else if (pat == 4) {
                add(EntityType::Truck, d, a, 0.0f, ri(5));
                add(EntityType::Truck, d, c, 0.0f, ri(5));
                coinLine(d - 5.0f, b, 8, 2.4f, 0.9f);
            } else if (pat == 5) {
                add(EntityType::Barrier, d, a);
                add(EntityType::Barrier, d, b);
                coinLine(d - 4.0f, c, 7, 2.4f, 0.9f);
            } else {
                add(EntityType::Truck, d, a, 0.0f, ri(5));
                coinLine(d - 5.0f, b, 6, 2.4f, 0.9f);
            }
        }

        if (!letterAlive() && rnd() < 0.40f) {
            add(EntityType::Letter, d + 16.0f, ri(3), 1.35f, neededLetter);
        }

        if (!flying && rnd() < 0.030f) {
            add(EntityType::FlightOrb, d + 18.0f, ri(3), 1.25f);
        }

        if (rnd() < 0.018f) {
            add(EntityType::MagnetOrb, d + 20.0f, ri(3), 1.25f);
        }
    }

public:
    static constexpr int kNumLanes = 3;
    static constexpr float kLaneGap = 2.4f;

    std::vector<Entity> entities;

    Track() : rng(2423) {}

    static float laneX(int lane) {
        return ((float)lane - 1.0f) * kLaneGap;
    }

    void reset(float playerDist) {
        entities.clear();
        nextRow = playerDist + 70.0f;
        flightCursor = playerDist;
    }

    void clear() { entities.clear(); }

    void startFlight(float playerDist) {
        flightCursor = playerDist + 20.0f;
    }

    void endFlight() {
        entities.erase(std::remove_if(entities.begin(), entities.end(), [](const Entity& e) {
            return e.type == EntityType::Coin && e.y > 2.5f;
        }), entities.end());
    }

    void ensure(float playerDist, bool flying, int neededLetter, float difficulty) {
        while (nextRow < playerDist + 200.0f) {
            spawnRow(nextRow, neededLetter, flying);
            float gap = std::max(18.0f, 28.0f - difficulty * 0.5f) + rnd() * 6.0f;
            nextRow += gap;
        }

        if (flying) {
            while (flightCursor < playerDist + 140.0f) {
                float wave = std::sin(flightCursor * 0.040f);
                float x = wave * 2.4f;
                entities.push_back({EntityType::Coin, flightCursor, x, 3.2f, 0, true, false, false});
                flightCursor += 5.0f;
            }
        }
    }

    TrackEvents update(float playerDist, float px, float carBottom, bool flying, bool invulnerable, bool magnet, float dt) {
        TrackEvents ev;
        float centerY = carBottom + 0.65f;
        float carTop = carBottom + 1.2f;

        for (Entity& e : entities) {
            if (!e.alive) continue;
            float dz = e.dist - playerDist;
            float dx = std::fabs(e.x - px);

            switch (e.type) {
            case EntityType::Coin:
                if (magnet && !e.pulled && dz > -1.0f && dz < 25.0f) e.pulled = true;
                if (e.pulled) {
                    float k = std::min(1.0f, 10.0f * dt);
                    e.x += (px - e.x) * k;
                    e.y += (centerY - e.y) * k;
                    e.dist -= dz * k * 0.65f;
                    dz = e.dist - playerDist;
                    if (dz < 1.3f) { e.alive = false; ev.coins++; }
                } else if (std::fabs(dz) < 1.2f && dx < 1.2f && std::fabs(e.y - centerY) < 1.4f) {
                    e.alive = false;
                    ev.coins++;
                }
                break;
            case EntityType::Letter:
                if (std::fabs(dz) < 1.4f && dx < 1.3f && std::fabs(e.y - centerY) < 1.5f) {
                    e.alive = false;
                    ev.letterIndex = e.index;
                }
                break;
            case EntityType::FlightOrb:
                if (std::fabs(dz) < 1.4f && dx < 1.3f && std::fabs(e.y - centerY) < 1.5f) {
                    e.alive = false;
                    ev.flightOrb = true;
                }
                break;
            case EntityType::MagnetOrb:
                if (std::fabs(dz) < 1.4f && dx < 1.3f && std::fabs(e.y - centerY) < 1.5f) {
                    e.alive = false;
                    ev.magnetOrb = true;
                }
                break;
            case EntityType::Barrier:
                if (!flying && !invulnerable && !e.hit && std::fabs(dz) < 2.0f && dx < 1.5f && carBottom < 0.95f) {
                    e.hit = true;
                    ev.crash = true;
                }
                break;
            case EntityType::Truck:
                if (!flying && !invulnerable && !e.hit && std::fabs(dz) < 3.2f && dx < 1.5f && carBottom < 2.4f) {
                    e.hit = true;
                    ev.crash = true;
                }
                break;
            case EntityType::Gate:
                if (!flying && !invulnerable && !e.hit && std::fabs(dz) < 2.0f && carTop > 1.70f) {
                    e.hit = true;
                    ev.crash = true;
                }
                break;
            }
        }

        entities.erase(std::remove_if(entities.begin(), entities.end(), [&](const Entity& e) {
            return !e.alive || (e.dist - playerDist) < -15.0f;
        }), entities.end());

        return ev;
    }
};

#endif
