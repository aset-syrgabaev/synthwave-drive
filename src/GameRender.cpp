#include "GameApp.h"
#include <cstdio>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

// Этот файл отвечает за отрисовку всей 3D-сцены в каждом кадре.
// Порядок рисования важен: сначала задний фон (небо), потом город и горы, дорога, объекты трассы,
// транспорт игрока, полупрозрачные эффекты и частицы, а в конце пост-обработка Bloom + CRT.
void GameApp::render3DScene() {
    bool overlayOpen = showHelpOverlay || showSettings;

    // Считаем высоту дороги прямо под камерой, чтобы на спуске в туннель камера не проваливалась под землю
    float expectedCamZ = camera.expectedZ(delorean.posY);
    float camRoadY     = (state == STATE_MENU) ? 0.0f : Track::tunnelDepthAt(scrollDistance - expectedCamZ);
    bool carInTunnel   = (delorean.roadY < -1.2f && !delorean.onTunnelRoof && !delorean.hover);
    bool camUnderRoof  = (carInTunnel && camRoadY < -3.8f);
    // На паузе время для камеры останавливаем, кроме режима свободной орбиты (4)
    float camDt = ((state == STATE_PLAYING && !overlayOpen) || state == STATE_MENU || camera.Mode == ORBIT_CAM) ? deltaTime : 0.0f;
    // Передаём в камеру тангаж (pitch), факт прыжка и трюка, чтобы транспорт НИКОГДА не перекрывал вид
    bool carAirborne = (!delorean.grounded && !delorean.hover);
    bool carTricking = (delorean.trickTimer > 0.0f);
    camera.UpdateChase(delorean.posX, delorean.posY, delorean.tilt, boostFactor, camDt,
                       camRoadY, camUnderRoof, delorean.pitch, carAirborne, carTricking);

    // Главные матрицы камеры (требование R3):
    // projection — перспективная проекция (угол обзора Zoom, соотношение сторон, ближняя/дальняя плоскости)
    // view — матрица вида (где стоит камера и куда смотрит)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)scrWidth / (float)scrHeight, 0.1f, 340.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // Если была тряска от удара или EMP, слегка сдвигаем матрицу камеры в случайную сторону
    if (shakeTimer > 0.0f && state == STATE_PLAYING && !overlayOpen) {
        float s = (shakeTimer / 0.45f) * 0.22f;
        float rX = ((float)rand() / (float)RAND_MAX - 0.5f) * s;
        float rY = ((float)rand() / (float)RAND_MAX - 0.5f) * s;
        view = glm::translate(glm::mat4(1.0f), glm::vec3(rX, rY, 0.0f)) * view;
    }

    // Берём цветовую палитру текущей локации (0..7) и задаём позицию основного света на горизонте
    const Theme& th = themes[themeIdx];
    glm::vec3 sunLightPos(0.0f, 22.0f, -75.0f);

    // 1) Начинаем рисовать не сразу на экран, а в оффскрин HDR MSAA фреймбуфер (нужно для Bloom и CRT)
    postProcess.begin();

    // 2) Небо рисуется полноэкранным квадратом на самой дальней глубине (z = 0.9999)
    sky.draw(skyShader, sceneClock, themeIdx, glm::vec2((float)scrWidth, (float)scrHeight),
             th.skyTop, th.skyMid, th.skyHorizon);

    // 3) Все здания, мосты, пирамиды и билборды по бокам дороги рисуются одним вызовом Instancing
    vegasCity.draw(buildingShader, view, projection, scrollDistance, camera.Position, sceneClock, themeIdx, th.mountainBase);

    // 4) Процедурные горы и песчаные дюны (в космосе — локация 4 — горы не рисуем)
    if (themeIdx != 4) {
        mountains.draw(mountainShader, view, projection, camera.Position,
                       th.mountainBase, th.mountainPeak, th.mountainBase, themeIdx);
    }

    // 5) Бесконечная прокручиваемая дорога с сеткой fwidth() и лучом фар машины
    terrain.draw(terrainShader, view, projection, scrollDistance, camera.Position,
                 glm::vec3(delorean.posX, delorean.posY, 0.0f), delorean.headlights,
                 th.gridLine, th.road, themeIdx, sceneClock, delorean.getHeadlightBeamColor());

    // 6) Инстансированные пальмы (Stretch Goal #1) — рисуются в Las Vegas (1), Сахаре (5) и Джунглях (6)
    if (themeIdx == 1 || themeIdx == 5 || themeIdx == 6) {
        palmTrees.draw(instancedShader, view, projection, scrollDistance,
                       sunLightPos, th.accent, camera.Position);
    }

    // 7) Настраиваем общий PBR-шейдер carShader для всех 3D-объектов трассы и транспорта игрока:
    // передаём 2 глобальных источника света (горизонт + городской контровой) и позицию/вкл фар (требование R4)
    carShader.use();
    carShader.setMat4("projection", projection);
    carShader.setMat4("view", view);
    carShader.setVec3("viewPos", camera.Position);
    carShader.setFloat("time", sceneClock);
    carShader.setVec3("lightPos1", sunLightPos);
    carShader.setVec3("lightColor1", th.skyHorizon);
    carShader.setVec3("lightPos2", glm::vec3(0.0f, 10.0f, 14.0f));
    carShader.setVec3("lightColor2", (themeIdx == 0 || themeIdx == 2) ? glm::vec3(0.32f, 0.44f, 0.58f) : glm::vec3(0.58f, 0.68f, 0.85f));
    carShader.setVec3("carPos", glm::vec3(delorean.posX, delorean.posY, 0.0f));
    carShader.setBool("headlightsOn", delorean.headlights);

    // В самом заезде рисуем препятствия, поезда, ЛРТ, монеты, полицейского дрона и босса-охотника
    if (state != STATE_MENU) {
        trackRenderer.draw(carShader, track, scrollDistance, sceneClock, themeIdx);
        trackRenderer.drawPolicePursuit(carShader, delorean.posX, delorean.posY, stumbleAlertTimer, sceneClock);
        trackRenderer.drawHunterGunship(carShader, Track::laneX(hunterLane), delorean.roadY, hunterTimer, hunterPhase, sceneClock);
    }

    // 8) Рисуем сначала непрозрачные части машины/мотоцикла, а затем отдельно полупрозрачные эффекты
    // (стёкла, невидимый скин, шлейфы фар, щит, волну EMP — чтобы прозрачность не перекрывала фон)
    delorean.draw(carShader, th.glow);
    delorean.drawTranslucentEffects(carShader);

    // 9) Частицы дождя, снега, песка и полосы скорости при ускорении
    float particleDt = (state == STATE_MENU && !overlayOpen) ? deltaTime
                     : ((state == STATE_PLAYING && !overlayOpen) ? simDt : 0.0f);
    speedParticles.updateAndDraw(particleShader, view, projection, camera.Position, currentSpeed, boostFactor, themeIdx, carInTunnel, particleDt);

    // 10) Копируем MSAA-буфер в обычную текстуру и рисуем её на экран с Bloom, ACES и CRT (Stretch #2 и #3)
    postProcess.end();
    postProcess.render(postprocessShader, sceneClock);
}

// Считывает пиксели готового кадра из видеопамяти (glReadPixels) и записывает в BMP-файл
void GameApp::saveBmpScreenshot(const std::string& path) const {
    std::vector<unsigned char> px((size_t)scrWidth * scrHeight * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, scrWidth, scrHeight, GL_BGR, GL_UNSIGNED_BYTE, px.data());
    FILE* fp = fopen(path.c_str(), "wb");
    if (!fp) return;
    // Формируем стандартный 54-байтный заголовок 24-битного BMP-файла
    unsigned int rowBytes = scrWidth * 3;
    unsigned int imgSize = rowBytes * scrHeight;
    unsigned char hdr[54] = {'B', 'M'};
    unsigned int fileSize = 54 + imgSize;
    std::memcpy(hdr + 2, &fileSize, 4);
    unsigned int off = 54, dib = 40, planesBpp = (24u << 16) | 1u;
    std::memcpy(hdr + 10, &off, 4);
    std::memcpy(hdr + 14, &dib, 4);
    std::memcpy(hdr + 18, &scrWidth, 4);
    std::memcpy(hdr + 22, &scrHeight, 4);
    std::memcpy(hdr + 26, &planesBpp, 4);
    std::memcpy(hdr + 34, &imgSize, 4);
    fwrite(hdr, 1, 54, fp);
    fwrite(px.data(), 1, px.size(), fp);
    fclose(fp);
}
