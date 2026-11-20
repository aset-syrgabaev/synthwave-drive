#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Пять режимов камеры, переключаются цифрами 1-5 или клавишей V:
// CHASE — классический вид сзади, ACTION — ближе и ниже, HOOD — с капота,
// ORBIT — свободное вращение вокруг транспорта, AKIRA — низкий боковой ракурс 3/4.
enum CameraMode {
    CHASE_CAM = 0,
    ACTION_CAM = 1,
    HOOD_CAM = 2,
    ORBIT_CAM = 3,
    AKIRA_CAM = 4
};

// Класс виртуальной камеры: хранит позицию, базисные векторы (Front, Up, Right),
// угол обзора (Zoom / FOV) и вычисляет матрицу вида (View Matrix) для OpenGL.
class Camera {
public:
    glm::vec3 Position; // мировые координаты самой камеры
    glm::vec3 Front;    // единичный вектор направления взгляда
    glm::vec3 Up;       // локальный вектор "вверх" камеры
    glm::vec3 Right;    // локальный вектор "вправо" камеры
    glm::vec3 WorldUp;  // глобальная ось Y мира (0, 1, 0)

    float Yaw;              // горизонтальный угол поворота (в градусах)
    float Pitch;            // вертикальный угол наклона (в градусах)
    float OrbitYaw;         // угол орбитальной камеры вокруг машины
    float OrbitPitch;       // высота орбитальной камеры
    float OrbitDist;        // расстояние до машины в режиме Orbit
    float MovementSpeed;
    float MouseSensitivity; // чувствительность мыши при зажатой кнопке
    float Zoom;             // вертикальный угол обзора FOV (увеличивается при нитро)

    CameraMode Mode;

    Camera(glm::vec3 position = glm::vec3(0.0f, 3.1f, 7.4f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = -14.0f)
        : Front(glm::vec3(0.0f, -0.2f, -1.0f)),
          OrbitYaw(28.0f),
          OrbitPitch(18.0f),
          OrbitDist(6.2f),
          MovementSpeed(18.0f),
          MouseSensitivity(0.18f),
          Zoom(54.0f),
          Mode(CHASE_CAM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Возвращает матрицу вида через glm::lookAt (переводит сцену из мировых координат в координаты камеры)
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Управление орбитальной камерой с клавиатуры (стрелки вращают камеру вокруг модели)
    void ProcessKeyboard(int direction, float deltaTime) {
        if (Mode != ORBIT_CAM) return;
        float speed = 65.0f * deltaTime;
        if (direction == 0) OrbitDist = std::max(3.2f, OrbitDist - 6.0f * deltaTime);
        if (direction == 1) OrbitDist = std::min(14.0f, OrbitDist + 6.0f * deltaTime);
        if (direction == 2) OrbitYaw -= speed;
        if (direction == 3) OrbitYaw += speed;
        if (direction == 4) OrbitPitch = std::min(65.0f, OrbitPitch + speed * 0.6f);
        if (direction == 5) OrbitPitch = std::max(5.0f, OrbitPitch - speed * 0.6f);
    }

    // Вращение орбитальной камеры мышью (работает при зажатой ЛКМ или ПКМ)
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        if (Mode != ORBIT_CAM) return;
        OrbitYaw += xoffset * MouseSensitivity;
        OrbitPitch += yoffset * MouseSensitivity;
        if (constrainPitch) {
            OrbitPitch = std::clamp(OrbitPitch, 5.0f, 65.0f);
        }
    }

    // Расчёт ожидаемой координаты Z камеры для проверки высоты дороги и потолка туннеля под ней
    float expectedZ(float carY) const {
        if (Mode == CHASE_CAM)  return 7.15f + (carY > 1.8f ? 0.55f : 0.0f);
        if (Mode == ACTION_CAM) return 5.35f;
        if (Mode == HOOD_CAM)   return -0.35f;
        if (Mode == AKIRA_CAM)  return 4.40f;
        float radYaw = glm::radians(OrbitYaw);
        float radPitch = glm::radians(OrbitPitch);
        return OrbitDist * std::cos(radPitch) * std::cos(radYaw);
    }

    // Плавное слежение камеры за транспортом с защитой от провала под пол и застревания в потолке туннеля.
    // camRoadY — высота дороги прямо под камерой, camUnderRoof — находится ли камера внутри туннеля.
    void UpdateChase(float carX, float carY, float carTilt, float boostFactor, float deltaTime,
                     float camRoadY = 0.0f, bool camUnderRoof = false) {
        const float kFloorClearance = 0.92f;
        const float kCeilingMaxY    = -1.08f;
        float minSafeY = camRoadY + kFloorClearance;

        // Ограничиваем высоту камеры снизу полом, а в туннеле ещё и сверху потолком
        auto clampCamY = [&](float y) {
            if (camUnderRoof) {
                return std::clamp(y, minSafeY, kCeilingMaxY);
            }
            return std::max(y, minSafeY);
        };

        if (Mode == CHASE_CAM) {
            float targetX = carX * 0.78f;
            float targetY = clampCamY(carY + (camUnderRoof ? 2.20f : 2.50f));
            float targetZ = 7.15f + (carY > 1.8f ? 0.55f : 0.0f);

            // Экспоненциальное сглаживание (1 - exp(-k*dt)) работает одинаково при любом FPS
            Position.x += (targetX - Position.x) * (1.0f - std::exp(-14.0f * deltaTime));
            Position.y += (targetY - Position.y) * (1.0f - std::exp(-14.0f * deltaTime));
            Position.z += (targetZ - Position.z) * (1.0f - std::exp(-12.0f * deltaTime));
            Position.y = clampCamY(Position.y);

            float lookY = camUnderRoof ? std::min(carY + 0.12f, -2.05f) : (carY + 0.15f);
            glm::vec3 targetLook = glm::vec3(carX * 0.85f, lookY, -24.0f);
            Front = glm::normalize(targetLook - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));

            // При ускорении слегка расширяем угол обзора для ощущения скорости
            float targetZoom = 54.0f + boostFactor * 9.0f;
            Zoom += (targetZoom - Zoom) * (1.0f - std::exp(-8.0f * deltaTime));
        } else if (Mode == ACTION_CAM) {
            float targetX = carX * 0.88f;
            float targetY = clampCamY(carY + 1.38f);
            float targetZ = 5.35f;

            Position.x += (targetX - Position.x) * (1.0f - std::exp(-16.0f * deltaTime));
            Position.y += (targetY - Position.y) * (1.0f - std::exp(-14.0f * deltaTime));
            Position.z += (targetZ - Position.z) * (1.0f - std::exp(-12.0f * deltaTime));
            Position.y = clampCamY(Position.y);

            float lookY = camUnderRoof ? std::min(carY + 0.25f, -1.95f) : (carY + 0.35f);
            glm::vec3 targetLook = glm::vec3(carX * 0.92f, lookY, -28.0f);
            Front = glm::normalize(targetLook - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));

            float targetZoom = 56.0f + boostFactor * 10.0f;
            Zoom += (targetZoom - Zoom) * (1.0f - std::exp(-8.0f * deltaTime));
        } else if (Mode == HOOD_CAM) {
            // Вид от первого лица (с капота / руля)
            float hoodY = clampCamY(carY + 0.80f);
            Position = glm::vec3(carX, hoodY, -0.35f);
            glm::vec3 targetLook = glm::vec3(carX, hoodY - 0.18f, -40.0f);
            Front = glm::normalize(targetLook - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));

            float targetZoom = 62.0f + boostFactor * 10.0f;
            Zoom += (targetZoom - Zoom) * (1.0f - std::exp(-8.0f * deltaTime));
        } else if (Mode == ORBIT_CAM) {
            // Орбитальная камера по сферическим координатам (используется в меню и для осмотра модели)
            float radYaw = glm::radians(OrbitYaw);
            float radPitch = glm::radians(OrbitPitch);
            glm::vec3 center(carX, carY + 0.25f, 0.0f);
            glm::vec3 offset(
                OrbitDist * std::cos(radPitch) * std::sin(radYaw),
                OrbitDist * std::sin(radPitch),
                OrbitDist * std::cos(radPitch) * std::cos(radYaw)
            );
            Position = center + offset;
            Position.y = clampCamY(Position.y);
            Front = glm::normalize(center - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));
            Zoom = 50.0f;
        } else if (Mode == AKIRA_CAM) {
            // Низкий боковой ракурс 3/4, удобно смотреть на силуэт мотоцикла и боковой дрифт
            float sideSign = (carX > 0.4f) ? -1.0f : 1.0f;
            float targetX = carX + sideSign * 2.45f;
            float targetY = clampCamY(carY + 0.95f);
            float targetZ = 4.40f;

            Position.x += (targetX - Position.x) * (1.0f - std::exp(-9.0f * deltaTime));
            Position.y += (targetY - Position.y) * (1.0f - std::exp(-12.0f * deltaTime));
            Position.z += (targetZ - Position.z) * (1.0f - std::exp(-10.0f * deltaTime));
            Position.y = clampCamY(Position.y);

            glm::vec3 targetLook = glm::vec3(carX * 0.90f, carY + 0.25f, -14.0f);
            Front = glm::normalize(targetLook - Position);
            Right = glm::normalize(glm::cross(Front, WorldUp));
            Up = glm::normalize(glm::cross(Right, Front));

            float targetZoom = 56.0f + boostFactor * 11.0f;
            Zoom += (targetZoom - Zoom) * (1.0f - std::exp(-8.0f * deltaTime));
        }
    }

    void SetMode(CameraMode newMode) {
        Mode = newMode;
    }

private:
    // Пересчитывает векторы Front, Right и Up по углам Эйлера (Yaw, Pitch)
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
