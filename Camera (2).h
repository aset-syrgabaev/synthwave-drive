#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================================================================
// МОДУЛЬ КАМЕРЫ (Camera.h) — отвечает за критерии R3 (View-матрица), R6 и C1
// Пять режимов камеры, переключаются цифрами 1–5 или клавишей V:
//   1 (CHASE_CAM)  — классический вид от 3-го лица сзади-сверху
//   2 (ACTION_CAM) — динамичный вид от 3-го лица ближе к транспорту
//   3 (HOOD_CAM)   — вид от 1-го лица спереди (перед носом машины/мотоцикла)
//   4 (ORBIT_CAM)  — свободная орбитальная камера 360° мышкой и стрелками
//   5 (AKIRA_CAM)  — кинематографичный боковой ракурс 3/4 (для дрифта Акиры)
// ============================================================================
enum CameraMode {
    CHASE_CAM = 0,
    ACTION_CAM = 1,
    HOOD_CAM = 2,
    ORBIT_CAM = 3,
    AKIRA_CAM = 4
};

// Класс виртуальной камеры: хранит позицию, ортонормированный базис (Front, Up, Right),
// угол обзора (Zoom / FOV) и собирает матрицу вида (View Matrix) через glm::lookAt.
class Camera {
public:
    glm::vec3 Position; // текущая точка в 3D-мире, где находится «объектив» камеры
    glm::vec3 Front;    // единичный вектор направления взгляда камеры
    glm::vec3 Up;       // локальный вектор «вверх» камеры (перпендикулярен Front)
    glm::vec3 Right;    // локальный вектор «вправо» (векторное произведение Front x WorldUp)
    glm::vec3 WorldUp;  // глобальная вертикаль мира (0, 1, 0)

    float Yaw;              // горизонтальный угол поворота по Эйлеру (в градусах)
    float Pitch;            // вертикальный угол наклона по Эйлеру (в градусах)
    float OrbitYaw;         // горизонтальный азимут в орбитальном режиме (4)
    float OrbitPitch;       // угол возвышения над машиной в орбитальном режиме (4)
    float OrbitDist;        // радиус орбиты — расстояние от камеры до центра машины
    float MovementSpeed;    // базовая скорость сдвига камеры
    float MouseSensitivity; // чувствительность мыши при вращении с зажатой ЛКМ/ПКМ
    float Zoom;             // вертикальный угол обзора FOV в градусах (растёт при нитро)

    CameraMode Mode; // текущий активный режим камеры (0..4)
    bool snapNext;   // флаг мгновенной перестановки при выходе из HOOD_CAM (чтобы не лететь сквозь салон)

    Camera(glm::vec3 position = glm::vec3(0.0f, 3.50f, 7.60f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = -14.0f)
        : Front(glm::vec3(0.0f, -0.2f, -1.0f)),
          OrbitYaw(28.0f),
          OrbitPitch(18.0f),
          OrbitDist(6.2f),
          MovementSpeed(18.0f),
          MouseSensitivity(0.18f),
          Zoom(54.0f),
          Mode(CHASE_CAM),
          snapNext(false) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Возвращает матрицу вида V = glm::lookAt(eye, target, up) для формулы MVP = P * V * M (R3).
    // Эта матрица переносит весь 3D-мир в систему координат, где камера стоит в (0,0,0) и смотрит вдоль -Z.
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Управление орбитальной камерой с клавиатуры (стрелки вращают камеру вокруг модели, R6).
    // Умножение на deltaTime делает скорость вращения независимой от FPS (критерий C1).
    void ProcessKeyboard(int direction, float deltaTime) {
        if (Mode != ORBIT_CAM) return;
        float speed = 65.0f * deltaTime;
        if (direction == 0) OrbitDist = std::max(3.2f, OrbitDist - 6.0f * deltaTime);
        if (direction == 1) OrbitDist = std::min(14.0f, OrbitDist + 6.0f * deltaTime);
        if (direction == 2) OrbitYaw -= speed;
        if (direction == 3) OrbitYaw += speed;
        if (direction == 4) OrbitPitch = std::min(65.0f, OrbitPitch + speed * 0.6f);
        if (direction == 5) OrbitPitch = std::max(8.0f, OrbitPitch - speed * 0.6f);
    }

    // Вращение орбитальной камеры мышью (работает при зажатой левой или правой кнопке, R6).
    // Угол OrbitPitch ограничен [8°, 65°], чтобы камера не переворачивалась вверх ногами и не уходила под асфальт.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        if (Mode != ORBIT_CAM) return;
        OrbitYaw += xoffset * MouseSensitivity;
        OrbitPitch += yoffset * MouseSensitivity;
        if (constrainPitch) {
            OrbitPitch = std::clamp(OrbitPitch, 8.0f, 65.0f);
        }
    }

    // Расчёт ожидаемой координаты Z камеры — нужен до вызова UpdateChase,
    // чтобы проверить высоту дороги и потолка туннеля точно под точкой камеры.
    float expectedZ(float carY) const {
        float airH = std::clamp(carY - 0.54f, 0.0f, 6.0f);
        if (Mode == CHASE_CAM)  return 7.60f + 0.48f * airH;
        if (Mode == ACTION_CAM) return 6.25f + 0.50f * airH;
        if (Mode == HOOD_CAM)   return -2.28f;
        if (Mode == AKIRA_CAM)  return 5.45f + 0.46f * airH;
        float radYaw = glm::radians(OrbitYaw);
        float radPitch = glm::radians(std::clamp(OrbitPitch, 8.0f, 65.0f));
        return OrbitDist * std::cos(radPitch) * std::cos(radYaw);
    }

    // Главная функция плавного слежения камеры за транспортом (вызывается 1 раз за кадр).
    // Защищает сразу от 3 проблем:
    //   1) провал камеры под дорогу/склон туннеля (minSafeY);
    //   2) выход камеры сквозь бетонный потолок туннеля (kCeilingMaxY);
    //   3) перекрытие обзора кузовом/мотоциклом при прыжке, 2x/3x прыжке, wheelie или сальто!
    void UpdateChase(float carX, float carY, float carTilt, float boostFactor, float deltaTime,
                     float camRoadY = 0.0f, bool camUnderRoof = false,
                     float carPitch = 0.0f, bool isAirborne = false, bool doingTrick = false) {
        (void)carTilt;
        const float kFloorClearance = 0.95f;
        const float kCeilingMaxY    = -1.08f;
        float minSafeY = camRoadY + kFloorClearance;
        // Сколько метров транспорт поднялся над уровнем дороги (в прыжке, на эстакаде ЛРТ или в полёте)
        float airHeight = std::clamp(carY - camRoadY - 0.54f, 0.0f, 6.0f);

        // Зажимает высоту камеры между полом и (внутри туннеля) потолком
        auto clampCamY = [&](float y) {
            if (camUnderRoof) {
                return std::clamp(y, minSafeY, kCeilingMaxY);
            }
            return std::max(y, minSafeY);
        };

        // Экспоненциальное сглаживание (1 - exp(-k * dt)) работает одинаково при 60 и при 180 FPS (C1).
        // Важный фикс прыжков: вверх камера следует мгновеннее (ky = 32), а жёсткий порог minAboveCar
        // не позволяет камере отстать и провалиться за задний бампер или сиденье байка при взлёте!
        auto smoothFollow = [&](float targetX, float targetY, float targetZ,
                                float kx, float kyDown, float kz, float minAboveCar) {
            if (snapNext) {
                Position = glm::vec3(targetX, targetY, targetZ);
                snapNext = false;
            } else {
                float ky = (targetY > Position.y) ? 32.0f : kyDown;
                Position.x += (targetX - Position.x) * (1.0f - std::exp(-kx * deltaTime));
                Position.y += (targetY - Position.y) * (1.0f - std::exp(-ky * deltaTime));
                Position.z += (targetZ - Position.z) * (1.0f - std::exp(-kz * deltaTime));
            }
            if (!camUnderRoof) {
                Position.y = std::max(Position.y, carY + minAboveCar);
            }
            Position.y = clampCamY(Position.y);
        };

        if (Mode == CHASE_CAM) {
            // [1] Классический вид от 3-го лица: в прыжке приподнимается выше и чуть отъезжает назад,
            // чтобы машина/байк оставались в нижней трети экрана, а вся дорога и препятствия были видны.
            float extraY = camUnderRoof ? 0.0f : (0.34f * airHeight + (isAirborne ? 0.60f : 0.0f) + (doingTrick ? 0.45f : 0.0f));
            float extraZ = camUnderRoof ? 0.0f : (0.48f * airHeight + (isAirborne ? 0.85f : 0.0f) + (doingTrick ? 1.10f : 0.0f));
            float targetX = carX * 0.78f;
            float targetY = clampCamY(carY + (camUnderRoof ? 2.25f : 2.95f) + extraY);
            float targetZ = 7.60f + extraZ;

            smoothFollow(targetX, targetY, targetZ, 14.0f, 14.0f, 12.0f,
                         2.90f + 0.24f * airHeight + (isAirborne ? 0.35f : 0.0f));

            float lookY = camUnderRoof ? std::min(carY + 0.10f, -2.05f) : (carY + 0.05f - 0.36f * airHeight);
            glm::vec3 targetLook = glm::vec3(carX * 0.85f, lookY, -24.0f);
            Front = glm::normalize(targetLook - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));

            // Динамическое расширение поля зрения (FOV) при нитро даёт ощущение разгона
            float targetZoom = 54.0f + boostFactor * 9.0f;
            Zoom += (targetZoom - Zoom) * (1.0f - std::exp(-8.0f * deltaTime));
        } else if (Mode == ACTION_CAM) {
            // [2] Ближний вид от 3-го лица: гарантированно держится выше крыши/обтекателя в любой фазе прыжка
            float extraY = camUnderRoof ? 0.0f : (0.36f * airHeight + (isAirborne ? 0.70f : 0.0f) + (doingTrick ? 0.50f : 0.0f));
            float extraZ = camUnderRoof ? 0.0f : (0.50f * airHeight + (isAirborne ? 0.90f : 0.0f) + (doingTrick ? 1.15f : 0.0f));
            float targetX = carX * 0.88f;
            float targetY = clampCamY(carY + (camUnderRoof ? 1.90f : 2.40f) + extraY);
            float targetZ = 6.25f + extraZ;

            smoothFollow(targetX, targetY, targetZ, 16.0f, 14.0f, 12.0f,
                         2.35f + 0.24f * airHeight + (isAirborne ? 0.35f : 0.0f));

            float lookY = camUnderRoof ? std::min(carY + 0.15f, -1.95f) : (carY + 0.12f - 0.32f * airHeight);
            glm::vec3 targetLook = glm::vec3(carX * 0.92f, lookY, -26.0f);
            Front = glm::normalize(targetLook - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));

            float targetZoom = 56.0f + boostFactor * 10.0f;
            Zoom += (targetZoom - Zoom) * (1.0f - std::exp(-8.0f * deltaTime));
        } else if (Mode == HOOD_CAM) {
            // [3] Вид от 1-го лица: вынесен на z = -2.28 (впереди бампера DeLorean -2.16 и колеса Акиры -1.72),
            // поэтому ни нос, ни крыло, ни ветровик НИКОГДА не залезают в кадр при прыжке, wheelie или сальто!
            float pitchRad = glm::radians(std::max(0.0f, carPitch));
            float noseRise = 2.20f * std::sin(pitchRad) + (isAirborne ? 0.22f : 0.0f);
            float hoodZ = doingTrick ? -2.65f : -2.28f;
            float hoodY = clampCamY(carY + 1.12f + noseRise);
            Position = glm::vec3(carX, hoodY, hoodZ);
            snapNext = false;

            glm::vec3 targetLook = glm::vec3(carX, hoodY - 0.24f - 0.12f * airHeight, -40.0f);
            Front = glm::normalize(targetLook - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));

            float targetZoom = 62.0f + boostFactor * 10.0f;
            Zoom += (targetZoom - Zoom) * (1.0f - std::exp(-8.0f * deltaTime));
        } else if (Mode == ORBIT_CAM) {
            // [4] Сферическая орбитальная камера (x = R*cos(p)*sin(y), y = R*sin(p), z = R*cos(p)*cos(y)).
            // Используется в главном меню и при осмотре модели на 360° (требование R6).
            float radYaw = glm::radians(OrbitYaw);
            float radPitch = glm::radians(std::clamp(OrbitPitch, 8.0f, 65.0f));
            glm::vec3 center(carX, carY + 0.35f, 0.0f);
            glm::vec3 offset(
                OrbitDist * std::cos(radPitch) * std::sin(radYaw),
                OrbitDist * std::sin(radPitch),
                OrbitDist * std::cos(radPitch) * std::cos(radYaw)
            );
            Position = center + offset;
            Position.y = clampCamY(Position.y);
            snapNext = false;
            Front = glm::normalize(center - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));
            Zoom = 50.0f;
        } else if (Mode == AKIRA_CAM) {
            // [5] Боковой 3/4 ракурс Акиры: поднимается и чуть отодвигается в прыжке,
            // чтобы мотоцикл/машина красиво читались сбоку, не перекрывая трассу впереди.
            float sideSign = (carX > 0.4f) ? -1.0f : 1.0f;
            float extraY = camUnderRoof ? 0.0f : (0.34f * airHeight + (isAirborne ? 0.65f : 0.0f) + (doingTrick ? 0.45f : 0.0f));
            float extraZ = camUnderRoof ? 0.0f : (0.46f * airHeight + (isAirborne ? 0.85f : 0.0f) + (doingTrick ? 1.05f : 0.0f));
            float targetX = carX + sideSign * (2.75f + 0.22f * airHeight);
            float targetY = clampCamY(carY + (camUnderRoof ? 1.65f : 2.15f) + extraY);
            float targetZ = 5.45f + extraZ;

            smoothFollow(targetX, targetY, targetZ, 9.0f, 12.0f, 10.0f,
                         2.05f + 0.22f * airHeight + (isAirborne ? 0.35f : 0.0f));

            glm::vec3 targetLook = glm::vec3(carX * 0.90f, carY + 0.12f - 0.28f * airHeight, -16.0f);
            Front = glm::normalize(targetLook - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));

            float targetZoom = 56.0f + boostFactor * 11.0f;
            Zoom += (targetZoom - Zoom) * (1.0f - std::exp(-8.0f * deltaTime));
        }
    }

    // Переключает режим камеры; при входе/выходе из 1-го лица (HOOD_CAM) сразу переставляет камеру
    void SetMode(CameraMode newMode) {
        if (newMode != Mode) {
            if (Mode == HOOD_CAM || newMode == HOOD_CAM) {
                snapNext = true;
            }
            Mode = newMode;
        }
    }

private:
    // Пересчитывает базисные векторы Front, Right и Up по углам Эйлера (Yaw, Pitch)
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif
