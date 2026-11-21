#include "TrackRenderer.h"

// Отрисовка поездов, эстакад LRT, туннелей, барьеров, лазеров и мини-босса дрона-охотника

// Детализированный бетонный дорожный барьер "Джерси" со стальными опорами и мигалками (перепрыгивается SPACE)
void TrackRenderer::drawBarrier(const Shader& s, const glm::vec3& pos, float time, int era) const {
    BiomeObsStyle bs = biomeStyle(era);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    float pulse = 0.55f + 0.45f * std::sin(time * 11.0f + pos.z);
    glm::vec3 concrete = bs.secondaryMat;
    glm::vec3 concreteDark = bs.secondaryMat * 0.65f;
    glm::vec3 steel = bs.primaryMat;

    // Трапециевидное бетонное основание барьера
    box(s, m, glm::vec3(0.0f, 0.14f, 0.0f), glm::vec3(1.94f, 0.28f, 0.52f), concreteDark, glm::vec3(0.0f), 28.0f, 0.08f);
    box(s, m, glm::vec3(0.0f, 0.36f, 0.0f), glm::vec3(1.88f, 0.20f, 0.34f), concrete, glm::vec3(0.0f), 32.0f, 0.10f);
    // Верхняя сигнальная балка на стальных стойках
    box(s, m, glm::vec3(-0.84f, 0.56f, 0.0f), glm::vec3(0.12f, 0.80f, 0.22f), steel, glm::vec3(0.0f), 96.0f, 0.45f);
    box(s, m, glm::vec3( 0.84f, 0.56f, 0.0f), glm::vec3(0.12f, 0.80f, 0.22f), steel, glm::vec3(0.0f), 96.0f, 0.45f);
    box(s, m, glm::vec3(0.0f, 0.68f, 0.0f), glm::vec3(1.96f, 0.32f, 0.16f), steel, glm::vec3(0.0f), 80.0f, 0.35f);

    // Светоотражающие красно-жёлтые полосы опасности
    for (int i = 0; i < 7; ++i) {
        bool red = (i % 2) == 0;
        glm::vec3 col = red ? bs.warnGlow : bs.accentGlow;
        glm::vec3 em  = col * (red ? 1.35f * pulse : 0.95f);
        box(s, m, glm::vec3(-0.78f + (float)i * 0.26f, 0.68f, 0.095f), glm::vec3(0.20f, 0.24f, 0.025f), col, em, 96.0f, 0.25f);
    }

    // Парные проблесковые маячки наверху стоек
    for (float sx : {-0.84f, 0.84f}) {
        glm::mat4 lamp = glm::translate(m, glm::vec3(sx, 0.98f, 0.0f));
        lamp = glm::rotate(lamp, glm::radians(90.0f), glm::vec3(0, 0, 1));
        drawPart(s, cylinder, glm::scale(lamp, glm::vec3(0.14f, 0.09f, 0.09f)), glm::vec3(1.0f, 0.45f, 0.08f), glm::vec3(2.4f, 0.75f, 0.12f) * pulse, 96.0f, 0.2f);
    }
}

// Движущийся патрульный кибер-дрон с вращающимися винтами и красно-синей мигалкой
void TrackRenderer::drawMovingBarrier(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.56f + 0.05f * std::sin(time * 8.0f), 0.0f));
    glm::vec3 armor(0.16f, 0.18f, 0.24f);
    glm::vec3 gunmetal(0.32f, 0.35f, 0.42f);
    bool sirenToggle = (static_cast<int>(time * 10.0f) % 2) == 0;
    glm::vec3 redSiren(2.5f, 0.10f, 0.18f);
    glm::vec3 blueSiren(0.12f, 0.50f, 2.7f);

    box(s, m, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.76f, 0.34f, 0.48f), armor, glm::vec3(0.0f), 140.0f, 0.60f);
    box(s, m, glm::vec3(0.0f, 0.0f, 0.25f), glm::vec3(1.30f, 0.14f, 0.06f), gunmetal, glm::vec3(0.0f), 160.0f, 0.70f);
    box(s, m, glm::vec3(-0.48f, 0.24f, 0.0f), glm::vec3(0.40f, 0.12f, 0.24f), glm::vec3(1.0f, 0.1f, 0.1f), sirenToggle ? redSiren : redSiren * 0.12f);
    box(s, m, glm::vec3( 0.48f, 0.24f, 0.0f), glm::vec3(0.40f, 0.12f, 0.24f), glm::vec3(0.1f, 0.4f, 1.0f), sirenToggle ? blueSiren * 0.12f : blueSiren);

    for (float sx : {-0.76f, 0.76f}) {
        glm::mat4 thr = glm::translate(m, glm::vec3(sx, -0.22f, 0.0f));
        thr = glm::rotate(thr, glm::radians(90.0f), glm::vec3(0, 0, 1));
        drawPart(s, cylinder, glm::scale(thr, glm::vec3(0.06f, 0.24f, 0.24f)), glm::vec3(0.12f, 0.92f, 1.0f), glm::vec3(0.15f, 1.6f, 2.0f), 64.0f, 0.0f);
    }
}

// Бронированная лазерная турель с вращающимися плазменными лопастями
void TrackRenderer::drawSpinner(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.48f, 0.0f));
    glm::vec3 darkMetal(0.14f, 0.15f, 0.20f);
    glm::vec3 steel(0.55f, 0.58f, 0.64f);
    glm::vec3 neonMagenta(1.0f, 0.15f, 0.72f);
    glm::vec3 neonCyan(0.12f, 0.95f, 1.0f);

    glm::mat4 baseCyl = glm::translate(m, glm::vec3(0.0f, -0.34f, 0.0f));
    baseCyl = glm::rotate(baseCyl, glm::radians(90.0f), glm::vec3(0, 0, 1));
    drawPart(s, cylinder, glm::scale(baseCyl, glm::vec3(0.28f, 0.34f, 0.34f)), darkMetal, glm::vec3(0.0f), 110.0f, 0.5f);
    drawPart(s, sphere, glm::scale(m, glm::vec3(0.28f)), steel, neonCyan * 0.6f, 160.0f, 0.7f);

    glm::mat4 rot1 = glm::rotate(m, time * 5.2f + pos.z * 0.2f, glm::vec3(0, 1, 0));
    drawPart(s, cube, glm::scale(rot1, glm::vec3(1.94f, 0.10f, 0.10f)), neonMagenta, neonMagenta * 1.9f, 96.0f, 0.3f);

    glm::mat4 rot2 = glm::rotate(m, time * 5.2f + pos.z * 0.2f + 1.5708f, glm::vec3(0, 1, 0));
    drawPart(s, cube, glm::scale(rot2, glm::vec3(1.94f, 0.10f, 0.10f)), neonCyan, neonCyan * 1.8f, 96.0f, 0.3f);
}

// Высокий промышленный лазерный шлагбаум (нужно приседать клавишей S, чтобы проскочить снизу)
void TrackRenderer::drawGate(const Shader& s, const glm::vec3& pos, float time, int era) const {
    BiomeObsStyle bs = biomeStyle(era);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    float pulse = 0.72f + 0.28f * std::sin(time * 9.0f + pos.x);
    glm::vec3 steel = bs.primaryMat;
    glm::vec3 concrete = bs.secondaryMat;
    glm::vec3 neonCyan = bs.accentGlow;
    glm::vec3 neonOrange = bs.warnGlow;

    for (float sx : {-0.96f, 0.96f}) {
        box(s, m, glm::vec3(sx, 0.16f, 0.0f), glm::vec3(0.32f, 0.32f, 0.46f), concrete, glm::vec3(0.0f), 32.0f, 0.1f);
        box(s, m, glm::vec3(sx, 1.28f, 0.0f), glm::vec3(0.16f, 2.48f, 0.28f), steel, glm::vec3(0.0f), 110.0f, 0.5f);
        box(s, m, glm::vec3(sx, 1.28f, 0.16f), glm::vec3(0.04f, 2.30f, 0.03f), neonCyan, neonCyan * 1.2f);
    }

    // Верхняя балка с предупреждающими полосами и лазерным лучом на высоте 1.15м
    box(s, m, glm::vec3(0.0f, 1.90f, 0.0f), glm::vec3(2.12f, 0.96f, 0.32f), steel, glm::vec3(0.0f), 110.0f, 0.5f);
    box(s, m, glm::vec3(0.0f, 1.38f, 0.0f), glm::vec3(1.96f, 0.07f, 0.07f), neonOrange, neonOrange * (2.2f * pulse));
    box(s, m, glm::vec3(0.0f, 1.18f, 0.0f), glm::vec3(1.96f, 0.05f, 0.05f), neonOrange, neonOrange * (1.8f * pulse));

    for (int i = 0; i < 6; ++i) {
        bool warn = (i % 2) == 0;
        glm::vec3 c = warn ? neonOrange : glm::vec3(1.0f, 0.88f, 0.18f);
        box(s, m, glm::vec3(-0.78f + (float)i * 0.31f, 1.90f, 0.175f), glm::vec3(0.23f, 0.68f, 0.025f), c, c * (1.05f * pulse));
    }
}

// Секция 200-метрового подземного туннеля: бетонные стены, стальные арки, вентиляторы
// и полноценная верхняя дорожная эстакада (y = 0.0), на которую можно приземлиться сверху из полёта
void TrackRenderer::drawTunnel(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const {
    float floorY = -6.00f;
    float h = 5.45f;
    float len = e.length;

    glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, floorY, pos.z));
    glm::vec3 darkConcrete(0.07f, 0.07f, 0.10f);
    glm::vec3 ribSteel(0.15f, 0.16f, 0.21f);
    glm::vec3 overpassAsphalt(0.05f, 0.04f, 0.09f);
    glm::vec3 dimEmergency(0.95f, 0.28f, 0.06f);

    for (float sx : {-1.0f, 1.0f}) {
        box(s, m, glm::vec3(sx * 4.88f, h * 0.5f, 0.0f), glm::vec3(0.45f, h, len), darkConcrete, glm::vec3(0.0f), 32.0f, 0.05f);
        box(s, m, glm::vec3(sx * 4.62f, 0.85f, 0.0f), glm::vec3(0.04f, 0.06f, len - 0.4f), dimEmergency, dimEmergency * 0.14f);

        // Несущие стальные рёбра вдоль внешних стен (не мешают перестроению по полосам)
        for (int k = -2; k <= 2; ++k) {
            float pz = (float)k * (len * 0.20f);
            box(s, m, glm::vec3(sx * 4.58f, h * 0.5f, pz), glm::vec3(0.32f, h, 0.48f), ribSteel, glm::vec3(0.0f), 64.0f, 0.20f);
            box(s, m, glm::vec3(sx * 4.38f, 1.4f, pz), glm::vec3(0.06f, 0.26f, 0.10f), dimEmergency, dimEmergency * 0.30f);
        }

        // Промышленный вытяжной вентилятор под потолком на боковой стене
        glm::mat4 fanM = glm::translate(m, glm::vec3(sx * 4.35f, h - 1.15f, 0.0f));
        fanM = glm::rotate(fanM, time * 4.5f * sx, glm::vec3(1, 0, 0));
        drawPart(s, cube, glm::scale(fanM, glm::vec3(0.08f, 1.15f, 0.16f)), ribSteel, glm::vec3(0.0f), 96.0f, 0.35f);
        drawPart(s, cube, glm::scale(fanM, glm::vec3(0.08f, 0.16f, 1.15f)), ribSteel, glm::vec3(0.0f), 96.0f, 0.35f);

        // Отбойники безопасности наверху на крыше-эстакаде туннеля (worldY = 0.0..0.48)
        box(s, m, glm::vec3(sx * 4.82f, 6.22f, 0.0f), glm::vec3(0.24f, 0.46f, len), ribSteel, glm::vec3(0.0f), 80.0f, 0.30f);
        box(s, m, glm::vec3(sx * 4.68f, 6.32f, 0.0f), glm::vec3(0.05f, 0.06f, len), glm::vec3(0.95f, 0.15f, 0.65f), glm::vec3(0.85f, 0.15f, 0.55f));
    }

    // Твёрдое бетонное перекрытие туннеля и дорожное полотно эстакады наверху (верх ровно на worldY = 0.0, внутри простор 5.5м)
    box(s, m, glm::vec3(0.0f, 5.82f, 0.0f), glm::vec3(9.8f, 0.36f, len), overpassAsphalt, glm::vec3(0.0f), 48.0f, 0.18f);

    // Прерывистая жёлтая разметка полос прямо на крыше-эстакаде туннеля
    for (int d = -4; d <= 4; ++d) {
        float dz = (float)d * 4.2f;
        for (float lx : {-1.2f, 1.2f}) {
            box(s, m, glm::vec3(lx, 6.01f, dz), glm::vec3(0.14f, 0.02f, 2.0f), glm::vec3(1.0f, 0.88f, 0.22f), glm::vec3(0.75f, 0.65f, 0.12f));
        }
    }

    // Тонкие потолочные стальные балки под перекрытием (высоко — не мешают камере при прыжках)
    for (int k = -2; k <= 2; ++k) {
        float pz = (float)k * (len * 0.20f);
        box(s, m, glm::vec3(0.0f, 5.56f, pz), glm::vec3(9.4f, 0.14f, 0.42f), ribSteel, glm::vec3(0.0f), 64.0f, 0.20f);
    }

    // Входной (index == 0) и выходной (index == 4) порталы 200-метрового туннеля
    if (e.index == 0) {
        float portalZ = len * 0.5f;
        box(s, m, glm::vec3(0.0f, h + 0.35f, portalZ), glm::vec3(10.4f, 0.72f, 0.55f), ribSteel, glm::vec3(0.0f), 64.0f, 0.2f);
        box(s, m, glm::vec3(0.0f, h - 0.04f, portalZ), glm::vec3(9.4f, 0.10f, 0.60f), dimEmergency, dimEmergency * 0.9f);
    } else if (e.index == 4) {
        float portalZ = -len * 0.5f;
        box(s, m, glm::vec3(0.0f, h + 0.35f, portalZ), glm::vec3(10.4f, 0.72f, 0.55f), ribSteel, glm::vec3(0.0f), 64.0f, 0.2f);
        box(s, m, glm::vec3(0.0f, h - 0.04f, portalZ), glm::vec3(9.4f, 0.10f, 0.60f), dimEmergency, dimEmergency * 0.9f);
    }
}


// Настоящая надземная эстакада LRT на высоте 3.85м:
// массивная бетонная коробчатая балка (толщина 0.62м совпадает с физическим потолком), рельсы, шпалы и опоры
void TrackRenderer::drawLrtPlatform(const Shader& s, const Entity& e, const glm::vec3& pos, float time, int era) const {
    BiomeObsStyle bs = biomeStyle(era);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    float len = e.length;
    glm::vec3 concrete = bs.secondaryMat * 0.88f;
    glm::vec3 darkSteel = bs.primaryMat * 0.72f;
    glm::vec3 railSteel(0.62f, 0.66f, 0.74f);
    glm::vec3 accent = bs.accentGlow;
    glm::vec3 warn = bs.warnGlow;
    float pulse = 0.72f + 0.28f * std::sin(time * 8.0f + pos.z);

    // Твёрдая несущая плита эстакады LRT (верх на pos.y = 3.85м, низ на 3.23м)
    box(s, m, glm::vec3(0.0f, -0.31f, 0.0f), glm::vec3(2.22f, 0.62f, len), concrete, glm::vec3(0.0f), 48.0f, 0.18f);
    // Две стальные рельсы LRT + поперечные шпалы сверху
    for (float sx : {-0.62f, 0.62f}) {
        box(s, m, glm::vec3(sx, 0.025f, 0.0f), glm::vec3(0.11f, 0.05f, len - 0.4f), railSteel, glm::vec3(0.0f), 210.0f, 0.82f);
    }
    int ties = std::max(4, (int)(len / 1.8f));
    for (int t = 0; t < ties; ++t) {
        float tz = -len * 0.46f + (float)t * (len * 0.92f / (float)(ties - 1));
        box(s, m, glm::vec3(0.0f, 0.008f, tz), glm::vec3(1.65f, 0.025f, 0.20f), darkSteel, glm::vec3(0.0f), 64.0f, 0.2f);
    }
    // Боковые парапеты эстакады + светящиеся габаритные полосы
    for (float sx : {-1.08f, 1.08f}) {
        box(s, m, glm::vec3(sx, 0.12f, 0.0f), glm::vec3(0.12f, 0.34f, len), darkSteel, glm::vec3(0.0f), 110.0f, 0.45f);
        box(s, m, glm::vec3(sx, 0.25f, 0.0f), glm::vec3(0.04f, 0.06f, len - 0.3f), accent, accent * 1.45f);
    }
    // Предупреждающая красная/янтарная полоса на нижней кромке (сигнализирует о твёрдом потолке снизу)
    // и яркий бирюзовый маяк на верхней кромке заезда
    box(s, m, glm::vec3(0.0f, -0.56f, len * 0.5f + 0.04f), glm::vec3(2.14f, 0.10f, 0.08f), warn, warn * 1.6f);
    box(s, m, glm::vec3(0.0f, -0.06f, len * 0.5f + 0.05f), glm::vec3(2.04f, 0.14f, 0.10f), accent, accent * (2.2f * pulse));

    // Массивные боковые опоры LRT, стоящие строго по краям полосы (не перекрывают проезд внизу)
    float pillarH = e.y - 0.62f;
    for (float sz : {-len * 0.42f, 0.0f, len * 0.42f}) {
        for (float sx : {-1.12f, 1.12f}) {
            box(s, m, glm::vec3(sx, -0.62f - pillarH * 0.5f, sz), glm::vec3(0.24f, pillarH, 0.64f), concrete * 0.85f, glm::vec3(0.0f), 55.0f, 0.20f);
        }
        box(s, m, glm::vec3(0.0f, -0.70f, sz), glm::vec3(2.38f, 0.22f, 0.72f), darkSteel, glm::vec3(0.0f), 70.0f, 0.28f);
    }
}

// Современный обтекаемый поезд LRT наверху эстакады (стоит на рельсах на высоте 3.85м)
void TrackRenderer::drawLrtTrain(const Shader& s, const Entity& e, const glm::vec3& pos, float time, int era) const {
    BiomeObsStyle bs = biomeStyle(era);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    float len = e.length;
    float h = Track::kLrtTrainHeight;
    glm::vec3 whiteBody = glm::mix(glm::vec3(0.84f, 0.88f, 0.94f), bs.primaryMat, 0.28f);
    glm::vec3 darkGlass(0.04f, 0.08f, 0.15f);
    glm::vec3 accent = bs.accentGlow;
    float pulse = 0.85f + 0.15f * std::sin(time * 9.0f);

    // Обтекаемый светлый корпус вагона LRT + скошенный нос
    box(s, m, glm::vec3(0.0f, h * 0.50f, 0.0f), glm::vec3(1.88f, h * 0.84f, len), whiteBody, glm::vec3(0.0f), 200.0f, 0.75f);
    box(s, m, glm::vec3(0.0f, h * 0.42f, len * 0.5f + 0.32f), glm::vec3(1.74f, h * 0.66f, 0.68f), whiteBody * 0.92f, glm::vec3(0.0f), 200.0f, 0.78f);
    box(s, m, glm::vec3(0.0f, h - 0.08f, 0.0f), glm::vec3(1.66f, 0.16f, len - 0.2f), bs.secondaryMat, glm::vec3(0.0f), 160.0f, 0.65f);
    // Панорамное лобовое стекло кабины LRT и светящиеся LED-фары
    box(s, m, glm::vec3(0.0f, h * 0.58f, len * 0.5f + 0.67f), glm::vec3(1.48f, 0.52f, 0.04f), darkGlass, accent * 0.35f, 230.0f, 0.92f);
    for (float sx : {-0.58f, 0.58f}) {
        box(s, m, glm::vec3(sx, 0.36f, len * 0.5f + 0.67f), glm::vec3(0.32f, 0.12f, 0.04f), glm::vec3(1.0f), glm::vec3(2.4f, 2.5f, 2.8f) * pulse);
    }
    // Светящаяся фирменная линия LRT и панорамные окна вдоль всего вагона
    for (float sx : {-1.0f, 1.0f}) {
        box(s, m, glm::vec3(sx * 0.955f, h * 0.24f, 0.0f), glm::vec3(0.03f, 0.10f, len - 0.2f), accent, accent * 1.45f);
        for (int w = -2; w <= 2; ++w) {
            float wz = (float)w * (len * 0.18f);
            box(s, m, glm::vec3(sx * 0.958f, h * 0.58f, wz), glm::vec3(0.025f, 0.48f, len * 0.13f), glm::vec3(0.88f, 0.95f, 1.0f), accent * 0.72f);
        }
    }
}

// Высотная подвесная киберпанк-платформа (5.75м) — цель для тройного прыжка Акиры или прыжка с ЛРТ
void TrackRenderer::drawSkyCatwalk(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    float len = e.length;
    glm::vec3 darkHull(0.06f, 0.07f, 0.11f);
    glm::vec3 crimson(1.0f, 0.14f, 0.26f);
    glm::vec3 holoCyan(0.14f, 0.94f, 1.0f);
    float pulse = 0.72f + 0.28f * std::sin(time * 10.0f + pos.z);

    // Тонкий металлический настил с неоновыми направляющими
    box(s, m, glm::vec3(0.0f, -0.12f, 0.0f), glm::vec3(2.10f, 0.24f, len), darkHull, glm::vec3(0.0f), 140.0f, 0.55f);
    for (float sx : {-1.02f, 1.02f}) {
        box(s, m, glm::vec3(sx, 0.05f, 0.0f), glm::vec3(0.08f, 0.14f, len), crimson, crimson * (1.8f * pulse));
    }
    // Поперечные светящиеся полосы настила и передний маяк
    box(s, m, glm::vec3(0.0f, -0.06f, len * 0.5f + 0.04f), glm::vec3(1.98f, 0.14f, 0.08f), holoCyan, holoCyan * 2.2f);
    for (int k = -2; k <= 2; ++k) {
        box(s, m, glm::vec3(0.0f, 0.015f, (float)k * (len * 0.19f)), glm::vec3(1.75f, 0.02f, 0.18f), holoCyan, holoCyan * 1.3f);
    }
}

// Киберпанк лазерная ловушка: две бронированные турели по краям полосы и красный лазерный луч
// e.index == 0 -> нижний луч (перепрыгивать SPACE), e.index == 1 -> верхний луч (приседать S / AKIRA SLIDE)
void TrackRenderer::drawLaserSweep(const Shader& s, const Entity& e, const glm::vec3& pos, float time, int era) const {
    BiomeObsStyle bs = biomeStyle(era);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    bool lowBeam = (e.index == 0);
    float beamY = lowBeam ? 0.52f : 1.38f;
    float pylonH = lowBeam ? 0.92f : 1.92f;
    float flicker = 0.80f + 0.20f * std::sin(time * 36.0f + pos.z);
    glm::vec3 armor = bs.primaryMat;
    glm::vec3 laserCol = lowBeam ? bs.warnGlow : bs.accentGlow;

    for (float sx : {-0.96f, 0.96f}) {
        box(s, m, glm::vec3(sx, pylonH * 0.5f, 0.0f), glm::vec3(0.22f, pylonH, 0.30f), armor, glm::vec3(0.0f), 140.0f, 0.55f);
        box(s, m, glm::vec3(sx, beamY, 0.0f), glm::vec3(0.28f, 0.16f, 0.22f), laserCol, laserCol * 2.2f);
    }
    // Сам горизонтальный лазерный луч + яркое белое ядро
    box(s, m, glm::vec3(0.0f, beamY, 0.0f), glm::vec3(1.88f, 0.10f, 0.10f), laserCol, laserCol * (2.6f * flicker));
    box(s, m, glm::vec3(0.0f, beamY, 0.0f), glm::vec3(1.88f, 0.04f, 0.04f), glm::vec3(1.0f), glm::vec3(2.8f, 2.2f, 2.0f) * flicker);
    if (!lowBeam) {
        // Верхняя балка-подсказка, чтобы издалека было видно, что под луч нужно проскользить
        box(s, m, glm::vec3(0.0f, 1.88f, 0.0f), glm::vec3(2.14f, 0.18f, 0.28f), armor, glm::vec3(0.0f), 120.0f, 0.45f);
    }
}


// Стальная наклонная рампа для заезда на крыши поездов с бегущими жёлтыми стрелками-шевронами
void TrackRenderer::drawRamp(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const {
    glm::vec3 neon = palette(e.index);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    m = glm::scale(m, glm::vec3(1.0f, Track::kTrainHeight, e.length));
    drawPart(s, rampMesh, m, glm::vec3(0.16f, 0.18f, 0.24f), neon * 0.10f, 140.0f, 0.55f);

    glm::mat4 base = glm::translate(glm::mat4(1.0f), pos);
    float angle = std::atan2(Track::kTrainHeight, e.length);
    float slopeLen = std::sqrt(e.length * e.length + Track::kTrainHeight * Track::kTrainHeight);
    for (float sx : {-0.96f, 0.96f}) {
        glm::mat4 rail = glm::translate(base, glm::vec3(sx, Track::kTrainHeight * 0.54f, 0.0f));
        rail = glm::rotate(rail, angle, glm::vec3(1, 0, 0));
        drawPart(s, cube, glm::scale(rail, glm::vec3(0.10f, 0.14f, slopeLen)),
                 glm::vec3(0.24f, 0.26f, 0.32f), neon * 0.45f, 120.0f, 0.5f);
        drawPart(s, cube, glm::scale(glm::translate(rail, glm::vec3(0.0f, 0.08f, 0.0f)), glm::vec3(0.05f, 0.04f, slopeLen)),
                 neon, neon * 1.5f, 96.0f, 0.3f);
    }

    for (int k = 0; k < 5; ++k) {
        float t = ((float)k + 0.5f) / 5.0f;
        float cz = (0.5f - t) * e.length;
        float cy = t * Track::kTrainHeight + 0.03f;
        float pulse = 0.55f + 0.45f * std::sin(time * 10.0f - (float)k * 1.1f);
        box(s, base, glm::vec3(0.0f, cy, cz), glm::vec3(1.55f, 0.04f, 0.20f), glm::vec3(1.0f, 0.90f, 0.22f), glm::vec3(1.0f, 0.82f, 0.14f) * (1.25f * pulse));
    }
}

// Детализированный экспресс-вагон метро: колёсные тележки, кондиционеры на крыше, двери и кабина машиниста
void TrackRenderer::drawTrain(const Shader& s, const Entity& e, const glm::vec3& pos, float time, int era) const {
    BiomeObsStyle bs = biomeStyle(era);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    glm::vec3 neon = (era == 0 || era == 1) ? palette(e.index) : ((e.index % 2 == 0) ? bs.accentGlow : bs.warnGlow);
    glm::vec3 steelBody = bs.primaryMat + neon * 0.04f;
    glm::vec3 roofSteel = bs.secondaryMat;
    glm::vec3 darkBogie = bs.primaryMat * 0.35f;
    float len = e.length;
    float h = Track::kTrainHeight;

    // Основной стальной корпус, скошенная крыша и нижняя юбка
    box(s, m, glm::vec3(0.0f, h * 0.52f, 0.0f), glm::vec3(2.00f, h * 0.78f, len), steelBody, glm::vec3(0.0f), 160.0f, 0.62f);
    box(s, m, glm::vec3(0.0f, h - 0.08f, 0.0f), glm::vec3(1.86f, 0.16f, len - 0.15f), roofSteel, glm::vec3(0.0f), 180.0f, 0.70f);
    box(s, m, glm::vec3(0.0f, 0.20f, 0.0f), glm::vec3(2.04f, 0.18f, len + 0.08f), darkBogie, glm::vec3(0.0f), 64.0f, 0.25f);

    // 3 блока промышленных кондиционеров (HVAC) на крыше вагона
    for (int ac = -1; ac <= 1; ++ac) {
        float az = (float)ac * (len * 0.28f);
        box(s, m, glm::vec3(0.0f, h + 0.05f, az), glm::vec3(1.18f, 0.10f, 1.85f), glm::vec3(0.18f, 0.20f, 0.25f), glm::vec3(0.0f), 96.0f, 0.4f);
    }

    // Колёсные тележки и стальные железнодорожные колёса под вагоном
    for (float bz : {-len * 0.32f, len * 0.32f}) {
        box(s, m, glm::vec3(0.0f, 0.14f, bz), glm::vec3(1.65f, 0.22f, 2.2f), darkBogie, glm::vec3(0.0f), 48.0f, 0.2f);
        for (float wz : {-0.65f, 0.65f}) {
            for (float sx : {-0.86f, 0.86f}) {
                glm::mat4 wm = glm::translate(m, glm::vec3(sx, 0.19f, bz + wz));
                drawPart(s, cylinder, glm::scale(wm, glm::vec3(0.14f, 0.18f, 0.18f)), glm::vec3(0.55f, 0.58f, 0.64f), glm::vec3(0.0f), 180.0f, 0.75f);
            }
        }
    }

    // Лобовое стекло кабины машиниста, электронное табло маршрута и передняя сцепка
    box(s, m, glm::vec3(0.0f, h * 0.68f, len * 0.50f + 0.02f), glm::vec3(1.66f, 0.58f, 0.04f), glm::vec3(0.04f, 0.07f, 0.14f), glm::vec3(0.06f, 0.18f, 0.32f), 220.0f, 0.90f);
    box(s, m, glm::vec3(0.0f, h * 0.88f, len * 0.50f + 0.025f), glm::vec3(1.15f, 0.14f, 0.04f), glm::vec3(0.04f), neon * 1.35f);
    box(s, m, glm::vec3(0.0f, 0.22f, len * 0.50f + 0.12f), glm::vec3(0.52f, 0.16f, 0.20f), darkBogie, glm::vec3(0.0f), 96.0f, 0.4f);

    bool moving = e.speed > 0.1f;
    float beamPulse = moving ? (1.1f + 0.3f * std::sin(time * 18.0f)) : 0.65f;
    glm::vec3 frontLamp = moving ? glm::vec3(2.5f, 2.2f, 1.7f) * beamPulse : glm::vec3(1.8f, 0.12f, 0.16f);

    for (float sx : {-1.0f, 1.0f}) {
        box(s, m, glm::vec3(sx * 0.68f, 0.44f, len * 0.50f + 0.03f), glm::vec3(0.32f, 0.15f, 0.04f), glm::vec3(0.95f), frontLamp);
        box(s, m, glm::vec3(sx * 1.015f, h * 0.40f, 0.0f), glm::vec3(0.03f, 0.09f, len - 0.2f), neon, neon * 1.25f);
        box(s, m, glm::vec3(sx * 1.015f, h * 0.86f, 0.0f), glm::vec3(0.03f, 0.045f, len - 0.2f), neon, neon * 1.05f);

        // Освещённые пассажирские окна и раздвижные двери по бортам вагона
        for (int w = -2; w <= 2; ++w) {
            float wz = (float)w * (len * 0.175f);
            bool isDoor = (w == -1 || w == 1);
            if (isDoor) {
                box(s, m, glm::vec3(sx * 1.015f, h * 0.52f, wz), glm::vec3(0.025f, h * 0.62f, len * 0.11f), glm::vec3(0.28f, 0.31f, 0.38f), glm::vec3(0.0f), 140.0f, 0.55f);
                box(s, m, glm::vec3(sx * 1.028f, h * 0.64f, wz), glm::vec3(0.02f, 0.32f, len * 0.08f), glm::vec3(0.95f, 0.85f, 0.55f), glm::vec3(0.75f, 0.62f, 0.30f));
            } else {
                box(s, m, glm::vec3(sx * 1.02f, h * 0.64f, wz), glm::vec3(0.025f, 0.36f, len * 0.12f), glm::vec3(0.95f, 0.85f, 0.55f), glm::vec3(0.80f, 0.66f, 0.32f));
            }
        }
    }
}


// Полицейский дрон-преследователь, который садится на хвост при зацепе препятствия (Stumble)
void TrackRenderer::drawPolicePursuit(const Shader& s, float carX, float carY, float stumbleTimer, float time) const {
    if (stumbleTimer <= 0.0f) return;
    float bob = 0.14f * std::sin(time * 9.0f);
    float zOffset = 2.65f + (1.0f - std::min(1.0f, stumbleTimer)) * 1.8f;
    glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(carX, carY + 0.78f + bob, zOffset));
    m = glm::rotate(m, glm::radians(8.0f * std::sin(time * 7.0f)), glm::vec3(0, 0, 1));

    bool flash = (static_cast<int>(time * 14.0f) % 2) == 0;
    glm::vec3 redCol(2.6f, 0.10f, 0.18f);
    glm::vec3 blueCol(0.12f, 0.52f, 2.6f);
    glm::vec3 darkHull(0.12f, 0.13f, 0.18f);

    box(s, m, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.18f, 0.24f, 0.68f), darkHull, glm::vec3(0.0f), 140.0f, 0.65f);
    box(s, m, glm::vec3(-0.34f, 0.16f, 0.0f), glm::vec3(0.32f, 0.10f, 0.22f), glm::vec3(1.0f, 0.1f, 0.1f), flash ? redCol : redCol * 0.1f);
    box(s, m, glm::vec3( 0.34f, 0.16f, 0.0f), glm::vec3(0.32f, 0.10f, 0.22f), glm::vec3(0.1f, 0.4f, 1.0f), flash ? blueCol * 0.1f : blueCol);
}

// Боевой дрон-охотник Нео-Токио (мини-босс): зависает впереди, наводит красный лазерный прицел на полосу и бьёт плазмой
void TrackRenderer::drawHunterGunship(const Shader& s, float targetLaneX, float roadY, float activeTimer, float cyclePhase, float time) const {
    if (activeTimer <= 0.0f) return;
    float bob = 0.22f * std::sin(time * 4.5f);
    float zPos = -17.5f;
    float yPos = roadY + 5.6f + bob;
    glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(targetLaneX, yPos, zPos));
    m = glm::rotate(m, glm::radians(6.0f * std::sin(time * 3.0f)), glm::vec3(0, 0, 1));

    glm::vec3 armor(0.09f, 0.10f, 0.14f);
    glm::vec3 gunmetal(0.24f, 0.26f, 0.32f);
    glm::vec3 crimson(1.0f, 0.10f, 0.16f);
    glm::vec3 cyan(0.14f, 0.88f, 1.0f);

    // Бронированный корпус и крылья боевого корабля-охотника
    box(s, m, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.8f, 0.72f, 2.2f), armor, glm::vec3(0.0f), 160.0f, 0.65f);
    box(s, m, glm::vec3(0.0f, -0.10f, 0.95f), glm::vec3(1.1f, 0.32f, 0.45f), crimson, crimson * 2.4f);
    for (float sx : {-1.95f, 1.95f}) {
        box(s, m, glm::vec3(sx, 0.05f, -0.15f), glm::vec3(1.3f, 0.24f, 1.4f), gunmetal, glm::vec3(0.0f), 150.0f, 0.60f);
        box(s, m, glm::vec3(sx, -0.22f, -0.15f), glm::vec3(0.55f, 0.22f, 0.55f), cyan, cyan * 2.2f);
    }

    // Фаза наведения (cyclePhase < 1.35с) -> тонкий мигающий луч прицела вдоль выбранной полосы
    // Фаза выстрела (cyclePhase >= 1.35с) -> широкий красный плазменный столб удара по полосе
    bool firing = (cyclePhase >= 1.35f);
    float blink = firing ? 1.0f : (0.35f + 0.65f * std::fabs(std::sin(time * 24.0f)));
    float beamW = firing ? 1.45f : 0.16f;
    glm::vec3 beamCol = firing ? glm::vec3(2.8f, 0.14f, 0.18f) : glm::vec3(2.0f, 0.10f, 0.14f);
    // Луч прицела/удара тянется прямо по полосе к игроку
    box(s, glm::mat4(1.0f), glm::vec3(targetLaneX, roadY + (firing ? 1.4f : 0.12f), -9.0f),
        glm::vec3(beamW, firing ? 2.8f : 0.14f, 18.0f), crimson, beamCol * blink, 64.0f, 0.1f);
    if (firing) {
        box(s, glm::mat4(1.0f), glm::vec3(targetLaneX, roadY + 1.4f, -9.0f),
            glm::vec3(0.45f, 2.8f, 18.0f), glm::vec3(1.0f), glm::vec3(3.0f, 2.2f, 1.8f), 64.0f, 0.0f);
    }
}

// Главный цикл отрисовки всех видимых объектов трассы (с отсечением по дальности)
void TrackRenderer::draw(const Shader& s, const Track& track, float distance, float time, int era) {
    for (const Entity& e : track.entities) {
        if (!e.alive) continue;
        float dz = e.dist - distance;
        if (dz > 235.0f || dz < -48.0f) continue;

        float roadY = Track::tunnelDepthAt(e.dist);
        float worldY = roadY + e.y;
        glm::vec3 pos(e.x, worldY, -dz);

        switch (e.type) {
        case EntityType::Coin: drawCoin(s, pos, time); break;
        case EntityType::Letter: drawLetter(s, e, pos, time); break;
        case EntityType::MysteryBox: drawMysteryBox(s, pos, time); break;
        case EntityType::JumpPad: drawJumpPad(s, pos, time); break;
        case EntityType::BoostPad: drawBoostPad(s, pos, time); break;
        case EntityType::AkiraCapsule: drawAkiraCapsule(s, pos, time); break;
        case EntityType::SkyRing: drawSkyRing(s, pos, time); break;
        case EntityType::SkyDrone: drawSkyDrone(s, pos, time); break;
        case EntityType::LrtPlatform: drawLrtPlatform(s, e, pos, time, era); break;
        case EntityType::LrtTrain: drawLrtTrain(s, e, pos, time, era); break;
        case EntityType::SkyCatwalk: drawSkyCatwalk(s, e, pos, time); break;
        case EntityType::LaserSweep: drawLaserSweep(s, e, pos, time, era); break;
        case EntityType::ShieldOrb: drawShieldOrb(s, pos, time); break;
        case EntityType::NitroOrb: drawNitroOrb(s, pos, time); break;
        case EntityType::TechToken: drawTechToken(s, e, pos, time); break;
        case EntityType::FlightOrb:
        case EntityType::MagnetOrb:
        case EntityType::SneakersOrb:
        case EntityType::MultiplierOrb:
            drawOrb(s, e, pos, time);
            break;
        case EntityType::Barrier: drawBarrier(s, pos, time, era); break;
        case EntityType::MovingBarrier: drawMovingBarrier(s, pos, time); break;
        case EntityType::Spinner: drawSpinner(s, pos, time); break;
        case EntityType::Gate: drawGate(s, pos, time, era); break;
        case EntityType::Tunnel: drawTunnel(s, e, pos, time); break;
        case EntityType::Ramp: drawRamp(s, e, pos, time); break;
        case EntityType::Truck: drawTrain(s, e, pos, time, era); break;
        }
    }
}
