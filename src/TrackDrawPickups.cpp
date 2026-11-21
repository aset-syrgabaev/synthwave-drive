#include "TrackRenderer.h"

// ============================================================================
// ОТРИСОВКА БОНУСОВ, МОНЕТ, 3D-БУКВ И КОЛЕЦ ПОЛЁТА (TrackDrawPickups.cpp) — R2
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   - biomeStyle(): подбирает материалы препятствий и неона под каждый из 8 биомов (0..7).
//   - letterMesh(): кэширует объёмные 3D-буквы (из Font5x7) в std::map, чтобы не
//     пересоздавать VBO каждый кадр (критерий B4 — эффективность).
//   - drawSkyRing / drawNitroOrb / drawShieldOrb / drawTechToken: анимация вращения и свечения.
// ============================================================================

BiomeObsStyle TrackRenderer::biomeStyle(int era) {
    static const BiomeObsStyle kStyles[8] = {
        // 0: 2077 NIGHT CITY — тёмный металл, циан и алый неон
        {glm::vec3(0.16f, 0.18f, 0.24f), glm::vec3(0.28f, 0.31f, 0.38f), glm::vec3(0.12f, 0.90f, 1.00f), glm::vec3(1.00f, 0.16f, 0.24f)},
        // 1: 1985 LAS VEGAS — ретро-хром, пурпурный и золотой неон
        {glm::vec3(0.24f, 0.16f, 0.34f), glm::vec3(0.42f, 0.32f, 0.50f), glm::vec3(1.00f, 0.16f, 0.72f), glm::vec3(1.00f, 0.84f, 0.18f)},
        // 2: NEO-TOKYO AKIRA — графитовый бетон, алый и янтарный
        {glm::vec3(0.15f, 0.15f, 0.19f), glm::vec3(0.32f, 0.28f, 0.30f), glm::vec3(1.00f, 0.14f, 0.16f), glm::vec3(1.00f, 0.54f, 0.10f)},
        // 3: ARCTIC AURORA — снежно-белая броня, голубой лёд и изумрудное сияние
        {glm::vec3(0.74f, 0.84f, 0.94f), glm::vec3(0.26f, 0.44f, 0.64f), glm::vec3(0.18f, 0.96f, 0.82f), glm::vec3(0.28f, 0.78f, 1.00f)},
        // 4: ORBITAL STATION — белый титан, графит и плазменно-синий
        {glm::vec3(0.68f, 0.74f, 0.84f), glm::vec3(0.18f, 0.22f, 0.30f), glm::vec3(0.18f, 0.78f, 1.00f), glm::vec3(1.00f, 0.62f, 0.16f)},
        // 5: SAHARA OASIS — тёплый песчаник, бронза, золото и бирюза оазиса
        {glm::vec3(0.66f, 0.46f, 0.24f), glm::vec3(0.42f, 0.28f, 0.15f), glm::vec3(1.00f, 0.78f, 0.20f), glm::vec3(0.14f, 0.92f, 0.86f)},
        // 6: NEON JUNGLE — замшелый камень храмов, тёмный нефрит и биолюминесцентный зелёный
        {glm::vec3(0.16f, 0.28f, 0.20f), glm::vec3(0.22f, 0.20f, 0.16f), glm::vec3(0.14f, 0.98f, 0.58f), glm::vec3(0.78f, 0.26f, 1.00f)},
        // 7: MARS COLONY — бело-оранжевые модули колонии и ржавый марсианский металл
        {glm::vec3(0.78f, 0.76f, 0.74f), glm::vec3(0.52f, 0.22f, 0.13f), glm::vec3(1.00f, 0.44f, 0.12f), glm::vec3(0.22f, 0.88f, 1.00f)}
    };
    return kStyles[std::clamp(era, 0, 7)];
}

// Строим клиновидный меш наклонной рампы для заезда на крыши поездов
Mesh TrackRenderer::buildRampMesh() {
    std::vector<float> v;
    glm::vec3 p0(-0.98f, 0.0f,  0.5f);
    glm::vec3 p1( 0.98f, 0.0f,  0.5f);
    glm::vec3 p2( 0.98f, 1.0f, -0.5f);
    glm::vec3 p3(-0.98f, 1.0f, -0.5f);
    glm::vec3 p4(-0.98f, 0.0f, -0.5f);
    glm::vec3 p5( 0.98f, 0.0f, -0.5f);

    glm::vec3 nSlope = glm::normalize(glm::cross(p1 - p0, p2 - p0));
    Primitives::quad(v, p0, p1, p2, p3, nSlope);
    Primitives::quad(v, p4, p3, p2, p5, glm::vec3(0, 0, -1));
    Primitives::quad(v, p0, p3, p4, p0, glm::vec3(-1, 0, 0));
    Primitives::quad(v, p1, p5, p2, p1, glm::vec3(1, 0, 0));

    Mesh m;
    m.upload(v);
    return m;
}

glm::vec3 TrackRenderer::palette(int i) const {
    const glm::vec3 p[5] = {
        glm::vec3(0.95f, 0.16f, 0.58f),
        glm::vec3(0.10f, 0.86f, 0.98f),
        glm::vec3(0.58f, 0.28f, 0.98f),
        glm::vec3(0.98f, 0.55f, 0.12f),
        glm::vec3(0.18f, 0.90f, 0.55f)
    };
    return p[((i % 5) + 5) % 5];
}

// Создаёт и кэширует объёмный 3D-меш для каждой буквы слова (NAZIRA, MARZHAN и т.д.)
const Mesh& TrackRenderer::letterMesh(char c) {
    auto it = letters.find(c);
    if (it != letters.end()) return it->second;
    std::vector<float> v;
    const char* g = findGlyph(c);
    const float px = 0.15f;
    if (g) {
        for (int r = 0; r < 7; ++r)
            for (int col = 0; col < 5; ++col)
                if (glyphPixel(g, col, r))
                    Primitives::appendBox(v, glm::vec3(((float)col - 2.0f) * px, (3.0f - (float)r) * px, 0.0f), glm::vec3(px * 0.95f, px * 0.95f, 0.26f));
    }
    Mesh m;
    m.upload(v);
    return letters.emplace(c, m).first->second;
}

void TrackRenderer::box(const Shader& s, const glm::mat4& base, const glm::vec3& p, const glm::vec3& size, const glm::vec3& color,
                        const glm::vec3& emissive, float shininess, float refl) const {
    drawPart(s, cube, glm::scale(glm::translate(base, p), size), color, emissive, shininess, refl);
}

// Вращающаяся золотая 3D-монета с рельефным ободком и центральным тиснением
void TrackRenderer::drawCoin(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.08f * std::sin(time * 5.5f + pos.z * 0.35f), 0.0f));
    m = glm::rotate(m, time * 4.5f + pos.z * 0.20f, glm::vec3(0, 1, 0));
    drawPart(s, cylinder, glm::scale(m, glm::vec3(0.075f, 0.28f, 0.28f)), glm::vec3(0.95f, 0.72f, 0.10f), glm::vec3(0.42f, 0.28f, 0.04f), 200.0f, 0.82f);
    drawPart(s, cylinder, glm::scale(m, glm::vec3(0.088f, 0.20f, 0.20f)), glm::vec3(0.76f, 0.54f, 0.08f), glm::vec3(0.22f, 0.14f, 0.02f), 160.0f, 0.65f);
    drawPart(s, cube, glm::scale(m, glm::vec3(0.105f, 0.22f, 0.06f)), glm::vec3(1.0f, 0.92f, 0.50f), glm::vec3(0.85f, 0.68f, 0.18f), 220.0f, 0.88f);
}

// Голографический жетон с буквой Word Hunt
void TrackRenderer::drawLetter(const Shader& s, const Entity& e, const glm::vec3& pos, float time) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.16f * std::sin(time * 4.0f), 0.0f));
    m = glm::rotate(m, time * 2.4f, glm::vec3(0, 1, 0));
    m = glm::scale(m, glm::vec3(1.25f));
    box(s, m, glm::vec3(0.0f), glm::vec3(0.96f, 1.32f, 0.08f), glm::vec3(0.06f, 0.02f, 0.16f), glm::vec3(0.0f, 0.25f, 0.45f), 96.0f, 0.5f);
    drawPart(s, letterMesh(e.letterChar), m, glm::vec3(1.0f, 0.85f, 0.2f), glm::vec3(2.0f, 1.3f, 0.22f), 128.0f, 0.4f);
}

// Секретный плутониевый кейс (Mystery Box) с металлическими углами и светящимся ядром
void TrackRenderer::drawMysteryBox(const Shader& s, const glm::vec3& pos, float time) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.16f * std::sin(time * 4.8f), 0.0f));
    m = glm::rotate(m, time * 2.6f, glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(10.0f * std::sin(time * 3.5f)), glm::vec3(1, 0, 1));

    glm::vec3 caseGold(0.92f, 0.70f, 0.14f);
    glm::vec3 darkSteel(0.14f, 0.15f, 0.20f);
    glm::vec3 neonCyan(0.12f, 0.92f, 1.0f);

    box(s, m, glm::vec3(0.0f), glm::vec3(0.76f, 0.56f, 0.76f), caseGold, caseGold * 0.35f, 180.0f, 0.70f);
    box(s, m, glm::vec3(0.0f, 0.26f, 0.0f), glm::vec3(0.80f, 0.10f, 0.80f), darkSteel, glm::vec3(0.0f), 140.0f, 0.6f);
    box(s, m, glm::vec3(0.0f, -0.26f, 0.0f), glm::vec3(0.80f, 0.10f, 0.80f), darkSteel, glm::vec3(0.0f), 140.0f, 0.6f);
    box(s, m, glm::vec3(0.0f), glm::vec3(0.18f, 0.62f, 0.80f), neonCyan, neonCyan * 1.4f, 120.0f, 0.4f);
    box(s, m, glm::vec3(0.0f), glm::vec3(0.80f, 0.62f, 0.18f), neonCyan, neonCyan * 1.4f, 120.0f, 0.4f);
}

// Бонусные сферы (полёт, магнит, супер-прыжок, 2X множитель)
void TrackRenderer::drawOrb(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const {
    glm::vec3 col;
    if (e.type == EntityType::FlightOrb) col = glm::vec3(0.12f, 0.92f, 1.0f);
    else if (e.type == EntityType::MagnetOrb) col = glm::vec3(1.0f, 0.20f, 0.35f);
    else if (e.type == EntityType::SneakersOrb) col = glm::vec3(0.20f, 1.0f, 0.35f);
    else col = glm::vec3(1.0f, 0.80f, 0.15f);

    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.15f * std::sin(time * 5.0f), 0.0f));
    m = glm::rotate(m, time * 3.0f, glm::vec3(0, 1, 0));

    drawPart(s, sphere, glm::scale(m, glm::vec3(0.38f)), col, col * 1.6f, 160.0f, 0.6f);

    glm::mat4 ring = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1));
    drawPart(s, cylinder, glm::scale(ring, glm::vec3(0.05f, 0.68f, 0.68f)), col, col * 2.0f, 128.0f, 0.4f);

    if (e.type == EntityType::FlightOrb) {
        box(s, m, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.22f, 0.10f, 0.26f), glm::vec3(0.88f), col * 1.6f, 180.0f, 0.75f);
    } else if (e.type == EntityType::MagnetOrb) {
        for (float sx : {-1.0f, 1.0f}) {
            box(s, m, glm::vec3(sx * 0.30f, 0.15f, 0.0f), glm::vec3(0.16f, 0.52f, 0.16f), glm::vec3(0.95f, 0.1f, 0.15f), glm::vec3(1.5f, 0.15f, 0.15f));
            box(s, m, glm::vec3(sx * 0.30f, 0.44f, 0.0f), glm::vec3(0.18f, 0.14f, 0.18f), glm::vec3(0.85f, 0.88f, 0.92f), glm::vec3(0.8f));
        }
        box(s, m, glm::vec3(0.0f, -0.10f, 0.0f), glm::vec3(0.76f, 0.16f, 0.16f), glm::vec3(0.95f, 0.1f, 0.15f), glm::vec3(1.5f, 0.15f, 0.15f));
    } else if (e.type == EntityType::SneakersOrb) {
        box(s, m, glm::vec3(0.0f, -0.15f, 0.10f), glm::vec3(0.34f, 0.20f, 0.62f), col, col * 1.6f);
        box(s, m, glm::vec3(0.0f,  0.14f, -0.08f), glm::vec3(0.30f, 0.40f, 0.30f), col, col * 1.4f);
    }
}

// Редкий синий энергоблок щита [E] (заряжает щит в режиме Classic)
void TrackRenderer::drawShieldOrb(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.18f * std::sin(time * 5.4f), 0.0f));
    m = glm::rotate(m, time * 2.8f, glm::vec3(0, 1, 0));
    glm::vec3 col(0.14f, 0.82f, 1.0f);
    drawPart(s, sphere, glm::scale(m, glm::vec3(0.42f)), col, col * 1.8f, 190.0f, 0.75f);
    glm::mat4 r1 = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1));
    glm::mat4 r2 = glm::rotate(m, time * 3.5f, glm::vec3(1, 0, 0));
    drawPart(s, cylinder, glm::scale(r1, glm::vec3(0.05f, 0.74f, 0.74f)), glm::vec3(0.9f), col * 2.2f, 140.0f, 0.5f);
    drawPart(s, cylinder, glm::scale(r2, glm::vec3(0.05f, 0.64f, 0.64f)), col, col * 1.8f, 140.0f, 0.5f);
}

// Редкий спец-токен способностей (0:Q Slow, 1:X EMP, 2:Z Phase, 3:N Dash, 4:U Magnet)
void TrackRenderer::drawTechToken(const Shader& s, const Entity& e, const glm::vec3& pos, float time) const {
    static const glm::vec3 kCols[5] = {
        glm::vec3(0.14f, 0.96f, 0.86f), // Q: Sandevistan
        glm::vec3(0.22f, 0.78f, 1.00f), // X: EMP
        glm::vec3(0.76f, 0.36f, 1.00f), // Z: Phase Shift
        glm::vec3(1.00f, 0.52f, 0.14f), // N: Shock Dash
        glm::vec3(1.00f, 0.84f, 0.18f)  // U: Coin Storm
    };
    glm::vec3 col = kCols[std::clamp(e.index, 0, 4)];
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.20f * std::sin(time * 6.0f + pos.z), 0.0f));
    m = glm::rotate(m, time * 3.2f, glm::vec3(0, 1, 0));
    glm::mat4 dia = glm::rotate(m, glm::radians(45.0f), glm::vec3(1, 0, 1));
    drawPart(s, cube, glm::scale(dia, glm::vec3(0.56f)), col, col * 1.95f, 200.0f, 0.75f);
    glm::mat4 ring = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1));
    drawPart(s, cylinder, glm::scale(ring, glm::vec3(0.05f, 0.80f, 0.80f)), glm::vec3(0.95f), col * 2.1f, 140.0f, 0.5f);
}

// Редкий огненный нитро-коин (пополняет запас ускорения SHIFT в режиме Classic)
void TrackRenderer::drawNitroOrb(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.18f * std::sin(time * 5.8f + pos.z), 0.0f));
    m = glm::rotate(m, time * 3.8f, glm::vec3(0, 1, 0));
    glm::vec3 flame(1.00f, 0.42f, 0.06f);
    glm::vec3 core(1.00f, 0.88f, 0.24f);
    glm::mat4 coinM = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 1, 0));
    drawPart(s, cylinder, glm::scale(coinM, glm::vec3(0.14f, 0.56f, 0.56f)), flame, flame * 1.85f, 180.0f, 0.65f);
    // Две стрелки-молнии на гранях нитро-коина
    box(s, m, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.24f, 0.64f, 0.18f), core, core * 2.2f);
    glm::mat4 ring = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1));
    drawPart(s, cylinder, glm::scale(ring, glm::vec3(0.04f, 0.78f, 0.78f)), flame, flame * 2.0f, 128.0f, 0.4f);
}


// Воздушное неоновое ускорительное кольцо в полёте (Sky Boost Ring): пролёт сквозь него даёт турбо + время полёта
void TrackRenderer::drawSkyRing(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    m = glm::rotate(m, time * 1.8f, glm::vec3(0, 0, 1));
    glm::vec3 cyan(0.12f, 0.95f, 1.0f);
    glm::vec3 gold(1.00f, 0.84f, 0.22f);
    float R = 1.65f;
    for (int k = 0; k < 12; ++k) {
        float ang = 6.2831853f * (float)k / 12.0f;
        glm::mat4 seg = glm::rotate(m, ang, glm::vec3(0, 0, 1));
        seg = glm::translate(seg, glm::vec3(0.0f, R, 0.0f));
        glm::vec3 col = (k % 3 == 0) ? gold : cyan;
        drawPart(s, cube, glm::scale(seg, glm::vec3(0.84f, 0.14f, 0.22f)), col, col * 1.95f, 160.0f, 0.65f);
    }
}

// Воздушный патрульный охранный дрон в полёте (Sky Drone) с красным сканером и 4 винтами
void TrackRenderer::drawSkyDrone(const Shader& s, const glm::vec3& pos, float time) const {
    float hoverBob = 0.18f * std::sin(time * 6.0f + pos.z);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, hoverBob, 0.0f));
    glm::vec3 darkHull(0.10f, 0.11f, 0.16f);
    glm::vec3 redEye(2.8f, 0.10f, 0.16f);
    glm::vec3 rotorCyan(0.14f, 0.88f, 1.0f);

    drawPart(s, sphere, glm::scale(m, glm::vec3(0.62f, 0.34f, 0.72f)), darkHull, glm::vec3(0.0f), 160.0f, 0.65f);
    box(s, m, glm::vec3(0.0f, 0.0f, 0.66f), glm::vec3(0.42f, 0.14f, 0.10f), glm::vec3(1.0f, 0.1f, 0.12f), redEye * (0.75f + 0.25f * std::sin(time * 16.0f)));
    for (float sx : {-0.75f, 0.75f}) {
        for (float sz : {-0.45f, 0.45f}) {
            box(s, m, glm::vec3(sx * 0.5f, 0.04f, sz * 0.5f), glm::vec3(0.75f, 0.06f, 0.08f), darkHull, glm::vec3(0.0f), 120.0f, 0.4f);
            glm::mat4 rot = glm::translate(m, glm::vec3(sx, 0.12f, sz));
            rot = glm::rotate(rot, glm::radians(90.0f), glm::vec3(0, 0, 1));
            drawPart(s, cylinder, glm::scale(rot, glm::vec3(0.04f, 0.28f, 0.28f)), rotorCyan, rotorCyan * 1.4f, 64.0f, 0.2f);
        }
    }
}

// Неоновая стрелочная Нитро-Полоса на асфальте (BoostPad): 3 бегущих вперёд шеврона ускорения
void TrackRenderer::drawBoostPad(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.03f, 0.0f));
    glm::vec3 plateDark(0.05f, 0.07f, 0.12f);
    glm::vec3 neonCyan(0.10f, 0.95f, 1.0f);
    glm::vec3 neonMagenta(1.0f, 0.18f, 0.52f);

    box(s, m, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.65f, 0.05f, 2.30f), plateDark, glm::vec3(0.0f), 120.0f, 0.45f);
    for (float sx : {-0.78f, 0.78f}) {
        box(s, m, glm::vec3(sx, 0.03f, 0.0f), glm::vec3(0.06f, 0.03f, 2.24f), neonMagenta, neonMagenta * 1.8f);
    }
    for (int i = 0; i < 3; ++i) {
        float wave = 0.55f + 0.45f * std::sin(time * 12.0f + (float)i * 1.4f);
        float zc = 0.65f - (float)i * 0.65f;
        for (float sx : {-1.0f, 1.0f}) {
            glm::mat4 ch = glm::translate(m, glm::vec3(sx * 0.28f, 0.035f, zc));
            ch = glm::rotate(ch, glm::radians(sx * 36.0f), glm::vec3(0, 1, 0));
            drawPart(s, cube, glm::scale(ch, glm::vec3(0.58f, 0.025f, 0.14f)), neonCyan, neonCyan * (2.2f * wave), 140.0f, 0.4f);
        }
    }
}

// Культовая сине-белая Психо-Капсула Акиры (AkiraCapsule) с вращающимся алым телекинетическим кольцом
void TrackRenderer::drawAkiraCapsule(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.16f * std::sin(time * 5.2f), 0.0f));
    m = glm::rotate(m, time * 2.8f, glm::vec3(0, 1, 0));

    glm::vec3 pillWhite(0.96f, 0.98f, 1.0f);
    glm::vec3 pillBlue(0.10f, 0.72f, 1.0f);
    glm::vec3 psychoRed(1.0f, 0.12f, 0.20f);

    // Внешнее алое психо-кольцо
    glm::mat4 ring = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1));
    drawPart(s, cylinder, glm::scale(ring, glm::vec3(0.05f, 0.74f, 0.74f)), psychoRed, psychoRed * 2.2f, 140.0f, 0.4f);

    // Наклонённая двухцветная капсула-таблетка (белая и голубая половинки + сферические торцы)
    glm::mat4 pill = glm::rotate(m, glm::radians(38.0f), glm::vec3(0, 0, 1));
    drawPart(s, cylinder, glm::scale(glm::translate(pill, glm::vec3(-0.14f, 0.0f, 0.0f)), glm::vec3(0.28f, 0.19f, 0.19f)), pillWhite, pillWhite * 0.55f, 180.0f, 0.7f);
    drawPart(s, sphere,   glm::scale(glm::translate(pill, glm::vec3(-0.28f, 0.0f, 0.0f)), glm::vec3(0.19f)), pillWhite, pillWhite * 0.55f, 180.0f, 0.7f);
    drawPart(s, cylinder, glm::scale(glm::translate(pill, glm::vec3( 0.14f, 0.0f, 0.0f)), glm::vec3(0.28f, 0.19f, 0.19f)), pillBlue,  pillBlue  * 1.35f, 180.0f, 0.7f);
    drawPart(s, sphere,   glm::scale(glm::translate(pill, glm::vec3( 0.28f, 0.0f, 0.0f)), glm::vec3(0.19f)), pillBlue,  pillBlue  * 1.35f, 180.0f, 0.7f);
}


// Кибер-трамплин (Cyber Jump Pad): подбрасывает высоко на крыши поездов и эстакады ЛРТ
void TrackRenderer::drawJumpPad(const Shader& s, const glm::vec3& pos, float time) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    float pulse = 0.65f + 0.35f * std::sin(time * 12.0f + pos.z);
    glm::vec3 darkSteel(0.12f, 0.14f, 0.18f);
    glm::vec3 cyberGreen(0.12f, 1.0f, 0.55f);
    box(s, m, glm::vec3(0.0f, 0.06f, 0.0f), glm::vec3(1.85f, 0.12f, 2.20f), darkSteel, glm::vec3(0.0f), 110.0f, 0.45f);
    for (int k = -1; k <= 1; ++k) {
        box(s, m, glm::vec3(0.0f, 0.13f, (float)k * 0.62f), glm::vec3(1.45f, 0.03f, 0.24f), cyberGreen, cyberGreen * (1.8f * pulse));
    }
}


TrackRenderer::TrackRenderer()
    : cube(Primitives::cube()),
      cylinder(Primitives::cylinder(28)),
      sphere(Primitives::sphere(14, 20)),
      rampMesh(buildRampMesh()) {}

