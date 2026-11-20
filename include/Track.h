#ifndef TRACK_H
#define TRACK_H

#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

// Все типы объектов, которые могут появляться на 3-полосной трассе
enum class EntityType {
    Coin,
    Letter,
    FlightOrb,     // заряд полёта [F]
    ShieldOrb,     // редкий синий энергоблок: заряд щита [E]
    TechToken,     // редкий кибер-токен: заряд способностей [Q]/[X]/[Z]/[N]/[U] (индекс 0..4)
    NitroOrb,      // редкий нитро-коин: пополняет запас ускорения [SHIFT] в режиме Classic
    MagnetOrb,     // временный магнит для монет
    SneakersOrb,   // редкий бонус прыжка: открывает двойной / тройной прыжок в Classic
    MultiplierOrb, // временный множитель 2x очков
    MysteryBox,    // случайный контейнер с наградой
    JumpPad,       // трамплин (подбрасывает на крыши поездов и на эстакаду LRT)
    BoostPad,      // полоса ускорения на дороге или на эстакаде LRT
    AkiraCapsule,  // редкая капсула (кратковременный режим ускорения и тарана)
    SkyRing,       // воздушное кольцо ускорения в полёте (в Classic максимум 4 за полёт)
    SkyDrone,      // патрульный воздушный дрон в полёте
    LrtPlatform,   // надземная бетонная эстакада LRT (высота 3.85м, твёрдая сверху и снизу)
    LrtTrain,      // надземный поезд LRT, который едет наверху по рельсам эстакады
    SkyCatwalk,    // высотный технический мост (высота 5.75м, под тройной прыжок или прыжок с LRT)
    LaserSweep,    // лазерный барьер (index=0 нижний — перепрыгнуть, index=1 верхний — присесть)
    Barrier,
    MovingBarrier,
    Truck,         // поезд метро на дорожном уровне
    Ramp,          // наклонная рампа для заезда на крышу поезда
    Gate,          // высокий шлагбаум (проезжается в приседе S)
    Tunnel,        // секция подземного туннеля
    Spinner        // вращающаяся лазерная турель
};

// Структура одного объекта на трассе: хранит тип, координаты, длину и состояние
struct Entity {
    EntityType type;
    float dist;       // координата вдоль трассы в метрах от старта
    float x;          // поперечная координата полосы (-2.4, 0, +2.4)
    float y;          // высота над дорожным полотном
    float length;     // длина объекта вдоль оси Z
    float speed;      // скорость навстречу игроку (> 0 для движущихся поездов)
    int lane;         // номер полосы (0, 1, 2)
    int index;        // подтип объекта (например, цвет поезда или тип способности 0..4)
    char letterChar;  // буква для EntityType::Letter
    bool alive;
    bool hit;
    bool pulled;      // притягивается ли монета магнитом
};

// Итоги физики и столкновений за один кадр (передаются из Track::update в main.cpp и Car::update)
struct TrackEvents {
    int coins = 0;
    char collectedLetter = 0;
    bool flightOrb = false;
    bool shieldOrb = false;
    bool nitroOrb = false;
    int techTokenType = -1;    // 0:Q(Slow), 1:X(EMP), 2:Z(Phase), 3:N(Dash), 4:U(Magnet), -1 = не подобран
    bool magnetOrb = false;
    bool sneakersOrb = false;
    bool multiplierOrb = false;
    bool mysteryBox = false;
    bool jumpPad = false;
    bool boostPad = false;
    bool akiraCapsule = false;
    bool skyRing = false;
    int smashedCount = 0;
    int phasedCount = 0;
    bool crash = false;
    bool stumble = false;
    bool nearMiss = false;
    float worldFloorY = 0.0f;      // твёрдая поверхность под колёсами (дорога, рампа, поезд, LRT, мост, крыша туннеля)
    float worldCeilingY = 999.0f;  // твёрдый потолок сверху (нижняя плита LRT / моста, нельзя пробить снизу)
    bool hasLateralClamp = false;  // если игрок упёрся сбоку в поезд, выталкиваем его, чтобы кузов не входил в вагон
    float clampedPosX = 0.0f;
    bool onRamp = false;
    bool onLrt = false;
    bool onTunnelRoof = false;
    float roadElevation = 0.0f;
};

// Генератор бесконечной трассы и физическая проверка столкновений (AABB).
// Следит за тем, чтобы препятствия не накладывались друг на друга, а эстакады LRT имели твёрдый потолок снизу.
class Track {
private:
    std::mt19937 rng;
    float nextRow = 26.0f;
    float flightCursor = 0.0f;
    int nextTunnelBlock = 0;
    int rowCounter = 0;
    int flightRingsLeft = 0;       // счётчик оставшихся колец ускорения за текущий полёт
    int classicPickupCooldown = 0; // ограничивает частоту редких бонусов в режиме Classic

    float rnd() { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); }
    int ri(int n) { return static_cast<int>(rnd() * (float)n) % n; }

    void add(EntityType t, float d, int lane, float y = 0.0f, int index = 0, float len = 1.0f, float spd = 0.0f, char ch = 'S') {
        entities.push_back({t, d, laneX(lane), y, len, spd, lane, index, ch, true, false, false});
    }


    bool isSpaceFree(float dMin, float dMax, int lane, float yMin, float yMax) const;
    void addSafeCoin(float d, int lane, float y);
    void coinLine(float d, int lane, int n, float gap, float y);
    void coinArc(float d, int lane, float baseY = 0.78f, float peakY = 2.15f);
    void addTrainWithRamp(float d, int lane, int colorIdx, bool withCoins = true, int numCars = 1);
    void addStationaryTrain(float d, int lane, int colorIdx, bool roofCoins = false);
    void addMovingTrain(float d, int lane, int colorIdx);
    void addLrtViaduct(float d, int lane, float len, bool withEntryJumpPad, bool withLrtTrain, bool withUpperCatwalk, bool classicMode);
    bool letterAlive() const;
    void spawnRow(float d, char neededChar, bool flying, bool classicMode);

public:
    static constexpr int kNumLanes = 3;
    static constexpr float kLaneGap = 2.4f;
    static constexpr float kTrainHeight = 2.15f;
    static constexpr float kLrtHeight = 3.85f;
    static constexpr float kLrtThickness = 0.62f;
    static constexpr float kLrtTrainHeight = 1.85f;
    static constexpr float kSkyCatwalkHeight = 5.75f;
    static constexpr float kCatwalkThickness = 0.42f;
    static constexpr float kTunnelCycle = 460.0f;

    std::vector<Entity> entities;

    Track() : rng(2423) {}

    static float laneX(int lane) { return ((float)lane - 1.0f) * kLaneGap; }
    static float smoothStep(float e0, float e1, float x) {
        float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
    static float tunnelDepthAt(float trackDist);
    static bool hasTunnelRoofAt(float trackDist);

    void reset(float playerDist);
    void clear() { entities.clear(); }

    int empPulse(float playerDist, float radius = 44.0f);
    void clearNearbyObstacles(float playerDist, float radius = 28.0f);
    void startFlight(float playerDist, bool classicMode = true);
    void endFlight();

    void ensure(float playerDist, bool flying, float flightWorldY, char neededChar, float difficulty, bool classicMode = true);
    TrackEvents update(float playerDist, float px, float carWorldY, float carBottomWorldY, bool ducking, bool flying,
                       bool invulnerable, bool magnet, bool switchingLane, float time, float dt,
                       bool phaseShift = false, bool bikeMode = false, bool smashActive = false, bool barrelRolling = false);
};

#endif
