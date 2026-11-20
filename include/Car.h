#ifndef CAR_H
#define CAR_H

#include <cmath>
#include <algorithm>
#include "GLHeaders.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Primitives.h"
#include "CarModel.h"
#include "AkiraBike.h"

// Набор уникальных параметров для каждого из 8 скинов:
// меняется не только кузов, но и цвет фар, стоп-сигналов, дисков, шлейфов, неона и щита.
struct SkinStyle {
    glm::vec3 carBody;
    glm::vec3 bikeBody;
    glm::vec3 headlight;
    glm::vec3 taillight;
    glm::vec3 trail;
    glm::vec3 accent;
    glm::vec3 shield;
    float rimGlow;
};

// Основной класс транспорта игрока: DeLorean DMC-12 и мотоцикл Канэды "Акира".
// Реализует иерархическую систему трансформаций (корпус -> колёса, двери, шлейфы) и физику движения.
class Car {
public:
    static const SkinStyle& getSkinStyle(int idx);
    glm::vec3 getHeadlightBeamColor() const;

private:
    // Базовые примитивы и заранее сгенерированные 3D-меши кузова, дверей и кабины DeLorean
    Mesh cube;
    Mesh cylinder;
    Mesh sphere;
    Mesh shieldSphere;
    Mesh bodyMesh;
    Mesh cabinGlassMesh;
    Mesh cabinFrameMesh;
    Mesh doorMetalL;
    Mesh doorMetalR;
    Mesh doorGlassL;
    Mesh doorGlassR;
    Mesh doorTrimL;
    Mesh doorTrimR;

    // 3D-детали мотоцикла Канэды из Акиры (обтекатели, шасси, ветровик, шины, колпаки и надписи)
    Mesh akiraWingLMesh;
    Mesh akiraWingRMesh;
    Mesh akiraNoseBridgeMesh;
    Mesh akiraLowerChassisMesh;
    Mesh akiraRearPodMesh;
    Mesh akiraWindscreenMesh;
    Mesh akiraTireFrontMesh;
    Mesh akiraTireRearMesh;
    Mesh akiraHubFrontMesh;
    Mesh akiraHubRearMesh;
    Mesh akiraDecalDisc;
    Mesh akiraDecalStar;
    Mesh akiraDecalTri;
    Mesh akiraTextCitizen;
    Mesh akiraTextCanon;
    Mesh akiraTextShoei;
    Mesh akiraPillMesh;
    Mesh trailRibbonMesh; // лента для светящихся шлейфов за фарами и колёсами
    float clock;

    // Вспомогательные функции отрисовки отдельных узлов DeLorean (реализация в CarRenderDeLorean.cpp)
    void part(const Shader& s, const Mesh& m, const glm::mat4& root, const glm::mat4& local, const glm::vec3& color,
              const glm::vec3& emissive = glm::vec3(0.0f), float shininess = 96.0f, float refl = 0.0f) const;
    static glm::mat4 boxAt(const glm::vec3& p, const glm::vec3& size);

    void drawGullwingDoors(const Shader& s, const glm::mat4& root, const glm::vec3& steel, const glm::vec3& glass, const glm::vec3& darkTrim) const;
    void drawWheelsAndThrusters(const Shader& s, const glm::mat4& root, const glm::vec3& glow) const;
    void drawLouversAndRearDeck(const Shader& s, const glm::mat4& root) const;
    void drawFrontFasciaAndHoodDetails(const Shader& s, const glm::mat4& root, const glm::vec3& darkTrim, const glm::vec3& steel) const;
    void drawRearFasciaAndTaillights(const Shader& s, const glm::mat4& root, const glm::vec3& darkTrim) const;
    void drawTimeMachineConversion(const Shader& s, const glm::mat4& root, const glm::vec3& glow) const;
    void drawCockpitInterior(const Shader& s, const glm::mat4& root) const;

    // Вспомогательные функции отрисовки мотоцикла Акиры (реализация в CarRenderAkira.cpp)
    void tube(const Shader& s, const glm::mat4& root, const glm::vec3& a, const glm::vec3& b, float r,
              const glm::vec3& col, const glm::vec3& em = glm::vec3(0.0f), float shin = 64.0f, float refl = 0.2f) const;
    void ball(const Shader& s, const glm::mat4& root, const glm::vec3& p, const glm::vec3& radii,
              const glm::vec3& col, const glm::vec3& em = glm::vec3(0.0f), float shin = 64.0f, float refl = 0.2f) const;
    void drawAkiraBike(const Shader& s, const glm::mat4& root, const glm::vec3& bikePaint, const glm::vec3& neonCol,
                       const glm::vec3& darkTrim, float bodyShin, float bodyRefl) const;

public:
    static constexpr float kGround = 0.54f; // высота центра кузова над поверхностью земли (чтобы шины ровно касались пола)

    // Физические координаты, скорости, флаги трюков и активных способностей (обновляются в CarPhysics.cpp)
    float posX = 0.0f;
    float posY = kGround;
    float roadY = 0.0f;
    float roadSlopePitch = 0.0f;
    float velY = 0.0f;
    float duckAmount = 0.0f;
    float suspensionOffset = 0.0f;
    float suspensionVel = 0.0f;
    float yawSlip = 0.0f;
    float empWaveRadius = 0.0f;
    float empWaveAlpha = 0.0f;
    float doubleJumpFlash = 0.0f;
    float morphFlash = 0.0f;
    float akiraSlideTimer = 0.0f;
    float barrelRollTimer = 0.0f;
    float barrelRollAngle = 0.0f;
    float trickTimer = 0.0f;
    float trickAngle = 0.0f;
    bool trickDone = false;
    bool psychoActive = false;
    int akiraSlideDir = 1;
    int jumpCount = 0;           // сколько прыжков уже сделано в воздухе
    int paintIdx = 0;            // текущий скин (0..7, меняется клавишей B)
    int turnSignalDir = 0;       // -1 левый поворотник, 0 выкл, +1 правый
    bool bikeMode = false;       // false = DeLorean DMC-12, true = мотоцикл Акиры (клавиша K)
    bool nitroDashActive = false;
    bool magnetOverdriveActive = false;
    bool braking = false;
    bool sandevistanActive = false;
    bool phaseShiftActive = false;
    bool doubleJumpUsed = false;
    bool grounded = true;        // стоит ли транспорт сейчас на твёрдой опоре
    bool hover = false;          // включён ли режим полёта (клавиша F)
    bool prevHover = false;
    bool headlights = true;      // включены ли фары (клавиша L)
    bool visible = true;
    bool doorsOpen = false;      // открыты ли двери крыло чайки (клавиша O)
    bool shieldActive = false;
    bool superJump = false;
    bool onTrainRoof = false;
    bool onTunnelRoof = false;

    // Углы для иерархической анимации (складывание колёс, двери, руль, крен, тангаж и вращение шин)
    float hoverBlend = 0.0f;
    float doorAngle = 0.0f;
    float steerAngle = 0.0f;
    float tilt = 0.0f;
    float pitch = 0.0f;
    float wheelRot = 0.0f;
    float lateralVel = 0.0f;
    float boostGlow = 0.0f;

    Car();

    // Действия игрока (вызываются из GameInput.cpp)
    int jump(float launchBoost = 1.0f, bool allowAirJumps = true);
    bool startTrick();
    bool consumeTrickDone();
    void triggerEmpWave();
    void fastDrop();
    void toggleDoors();
    void toggleVehicleMode();
    void triggerAkiraSlide(int dir);

    // Нижняя точка колёс в мировых координатах и проверка, едем ли сейчас внутри подземного туннеля
    float bottomWorldY() const { return posY - kGround; }
    bool isInsideTunnel() const { return (roadY < -1.0f && !onTunnelRoof && bottomWorldY() < -0.5f); }

    void reset();
    // Строит корневую матрицу кузова и поднимает её так, чтобы колёса не проваливались в пол при наклоне
    glm::mat4 computeRootMatrix() const;
    // Главный шаг физики транспорта: гравитация, пол, твёрдый потолок ЛРТ, выталкивание от стен поезда
    void update(float targetX, float speed, bool duckInput, float worldFloorY, bool onRamp, bool tunnelRoofFlag,
                float currentRoadY, float nextRoadY, float dt, float realDt,
                float worldCeilingY = 999.0f, bool hasLateralClamp = false, float clampedPosX = 0.0f);

    // Отрисовка: сначала непрозрачный кузов (draw), потом прозрачные эффекты (drawTranslucentEffects)
    void draw(const Shader& s, const glm::vec3& glow) const;
    void drawTranslucentEffects(const Shader& s) const;
};

#endif
