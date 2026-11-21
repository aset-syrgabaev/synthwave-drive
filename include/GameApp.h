#ifndef GAME_APP_H
#define GAME_APP_H

#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include "GLHeaders.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Car.h"
#include "Terrain.h"
#include "Mountain.h"
#include "PalmTree.h"
#include "VegasCity.h"
#include "Track.h"
#include "TrackRenderer.h"
#include "TextRenderer.h"
#include "Sky.h"
#include "SpeedParticles.h"
#include "PostProcess.h"
#include "Theme.h"

// ============================================================================
// ЦЕНТРАЛЬНЫЙ КЛАСС ПРИЛОЖЕНИЯ (GameApp.h) — КРИТЕРИЙ D2 (МОДУЛЬНАЯ АРХИТЕКТУРА)
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   Объединяет все 9 пар шейдеров, камеру, транспорт, дорогу, горы, пальмы, город,
//   трассу, пост-обработку (FBO Bloom/CRT) и 2D HUD.
//   Чтобы ни один файл не превышал 500 строк (критерий D2), методы GameApp
//   чисто разнесены по 4 отдельным .cpp файлам:
//     - GameInput.cpp  (клавиатура и мышь)
//     - GameEvents.cpp (логика, таймеры, коллизии, режимы Classic / Free Play)
//     - GameRender.cpp (10-этапный 3D-рендер кадра)
//     - GameHud.cpp    (2D-меню, счётчик FPS, настройки G и справка H)
// ============================================================================

// Основные состояния игры: главное меню, сам заезд, пауза и экран проигрыша
enum GameState {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
};

// Два режима игры: CLASSIC (все бонусы только через коины) и FREE PLAY (свободная песочница)
enum GameMode {
    MODE_CLASSIC = 0,
    MODE_FREE_PLAY = 1
};

// Контекст всей игровой сессии: хранит шейдеры, 3D-подсистемы, физические таймеры и состояние ввода.
// Логика разнесена по модулям GameInput.cpp, GameEvents.cpp, GameRender.cpp и GameHud.cpp.
struct GameApp {
    // Размеры окна, позиция курсора мыши и счётчик времени/FPS
    unsigned int scrWidth = 1280;
    unsigned int scrHeight = 720;
    float mouseX = 640.0f;
    float mouseY = 360.0f;
    bool mouseLeftClicked = false;
    bool clickNow = false;

    float deltaTime = 0.0f; // реальная длительность 1 кадра в секундах
    float simDt = 0.0f;     // игровой шаг времени (замедляется в режиме Sandevistan [Q])
    int currentFps = 60;    // текущий FPS для заголовка окна и зелёной плашки на экране

    // Камера и все 8 пар 3D GLSL-шейдеров (9-я пара для 2D-текста хранится внутри TextRenderer)
    Camera camera;
    Shader skyShader;
    Shader terrainShader;
    Shader mountainShader;
    Shader carShader;
    Shader instancedShader;
    Shader buildingShader;
    Shader particleShader;
    Shader postprocessShader;

    // Основные 3D и 2D подсистемы сцены
    Sky sky;
    Car delorean;
    Terrain terrain;
    Mountain mountains;
    PalmTree palmTrees;
    VegasCity vegasCity;
    Track track;
    TrackRenderer trackRenderer;
    TextRenderer textRenderer;
    SpeedParticles speedParticles;
    PostProcess postProcess;

    // Цветовые темы для всех 8 локаций и флаги открытых менюшек
    std::vector<Theme> themes;
    int themeIdx = 0;

    GameState state = STATE_MENU;
    GameMode gameMode = MODE_CLASSIC;
    bool showHelpOverlay = false;
    bool showSettings = false;
    int settingsCursor = 0;

    // Варианты настроек в меню G (сила свечения Bloom, скорость, названия камер и скинов)
    int bloomLevel = 1;
    int speedLevel = 1;
    const float bloomValues[3] = {0.05f, 0.16f, 0.30f};
    const float speedMultipliers[3] = {0.84f, 1.00f, 1.20f};
    const std::string modeShort[2]  = {"CLASSIC", "FREE PLAY"};
    const std::string camNames[5]   = {"CHASE", "ACTION", "HOOD", "ORBIT 360", "AKIRA CAM"};
    const std::string levelNames[3] = {"LOW", "MEDIUM", "HIGH"};
    const std::string speedNames[3] = {"CHILL", "NORMAL", "FAST"};
    const std::string paintNames[8] = {"ORIGINAL", "STEALTH INVIS", "CYBER MATRIX", "INFERNO CRBN", "CHAMELEON", "BIO HAZARD", "24K GOLD", "ARCTIC FROST"};

    // Текущая полоса (0..2), пройденная дистанция и скорость движения
    int currentLane = 1;
    int prevLane = 1;
    float laneSwitchTimer = 0.0f;
    float scrollDistance = 0.0f;
    float baseSpeed = 14.5f;
    float currentSpeed = 14.5f;
    float boostFactor = 0.0f;

    // Очки, монеты, рекорд, серия комбо и прогресс сбора букв текущего слова
    int score = 0;
    float scoreDistAccum = 0.0f;
    int coins = 0;
    int highScore = 5000;
    int scoreMultiplier = 1;
    int comboStreak = 0;
    int nextFeverMark = 25;
    float comboDecayTimer = 0.0f;
    float timeWarpCharge = 0.0f;

    std::vector<std::string> wordList = {"NAZIRA", "MARZHAN", "AIYMZHAN", "GAZIZA", "SE-2423"};
    std::mt19937 wordRng;
    int currentWordIdx = 0;
    int currentLetterProgress = 0;

    // Таймеры временных бонусов (полёт, магнит, 2x/3x прыжок, множитель очков, щит)
    float hoverTimer = 0.0f;
    float hoverMaxTimer = 18.0f;
    float hoverCD = 0.0f;
    float ringBoostTimer = 0.0f;
    float magnetTimer = 0.0f;
    float sneakersTimer = 0.0f;
    float multiplierTimer = 0.0f;
    float shieldTimer = 0.0f;
    float shieldMaxTimer = 24.0f;
    float shieldCD = 0.0f;

    // Состояние дрона-охотника (мини-босса) и флаг езды по эстакаде ЛРТ
    float hunterTimer = 0.0f;
    float hunterPhase = 0.0f;
    int hunterLane = 1;
    float nextHunterDist = 320.0f;
    bool hunterHitDone = false;
    bool onLrtDeck = false;

    // Заряды способностей и топливо нитро для режима Classic (пополняются только бонус-коинами)
    int qCharges = 0;
    int xCharges = 0;
    int zCharges = 0;
    int nCharges = 0;
    int uCharges = 0;
    int eCharges = 0;
    int fCharges = 0;
    float nitroFuel = 0.0f;
    bool keyShiftWarned = false;
    int flightRingsCollected = 0;

    float sandevistanTimer = 0.0f;
    float sandevistanCD = 0.0f;
    float empCD = 0.0f;
    float phaseTimer = 0.0f;
    float phaseCD = 0.0f;
    float dashTimer = 0.0f;
    float dashCD = 0.0f;
    float magOverdriveTimer = 0.0f;
    float magOverdriveCD = 0.0f;
    float psychoTimer = 0.0f;
    float stuntScoreCD = 0.0f;
    int stylePoints = 0;
    float styleDecayTimer = 0.0f;
    int lastTurnDir = 1;
    float wheelieAccum = 0.0f;
    float stumbleAlertTimer = 0.0f;
    float invulnerableTimer = 0.0f;
    float shakeTimer = 0.0f;
    float flashTimer = 0.0f;
    float sceneClock = 0.0f;
    glm::vec3 flashColor = glm::vec3(0.2f, 0.8f, 1.0f);
    float bannerTimer = 0.0f;
    std::string bannerText = "";

    // Флаги анти-дребезга клавиш
    bool keyCPressed = false, keyTPressed = false, keyLPressed = false;
    bool keyVPressed = false, keyFPressed = false, keyOPressed = false;
    bool keyEPressed = false, keyHPressed = false, keyGPressed = false;
    bool keyQPressed = false, keyXPressed = false, keyZPressed = false, keyBPressed = false;
    bool keyKPressed = false, keyNPressed = false, keyUPressed = false, keySPressed = false;
    bool keyJPressed = false;
    bool keyTabPressed = false, keyJumpPressed = false, keyAPressed = false;
    bool keyDPressed = false, keyPPressed = false, keyEnterPressed = false;
    bool keyRPressed = false, keyEscPressed = false;
    bool keyUpPressed = false, keyDownPressed = false;
    bool keyLeftPressed = false, keyRightPressed = false;

    // Параметры служебных режимов (--shot для скриншотов и --bench для замера FPS)
    bool shotMode = false;
    std::string shotPath;
    float shotYaw = 28.0f, shotPitch = 18.0f, shotDist = 6.2f;
    int shotBike = 1, shotTheme = 0, shotSkin = 0, shotPlay = 0;
    int shotCrt = 0, shotHover = 0;
    int shotFrames = 0;

    GameApp(unsigned int w, unsigned int h);

    void addStyle(int sp);
    int awardPoints(int basePts);
    bool destroyHunter();

    void applySettingChange(int idx, int dir);
    void resetGame();
    void handleGlobalKeys(GLFWwindow* window);
    void updateMenu(GLFWwindow* window);
    void updatePlaying(GLFWwindow* window);
    void updatePlayingTrackAndEvents(bool classicMode, bool duckInput, bool boostInput, bool allowAirJumps);
    void updatePausedAndGameOver(GLFWwindow* window);

    void render3DScene();
    void renderHud();
    void saveBmpScreenshot(const std::string& path) const;
};

#endif
