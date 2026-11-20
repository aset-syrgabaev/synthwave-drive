#include "VegasCity.h"

// Генерация архитектурных ансамблей локаций 0-2 (Night City, 1985 Las Vegas, Neo-Tokyo Akira)

void VegasCity::addSlot(int era, glm::vec3 pos, glm::vec3 size, glm::vec4 params, int animType, float animPhase, float rotZ) {
    eraSlots[era].push_back({pos, size, params, animType, animPhase, rotZ});
}

void VegasCity::addResortTower(int era, float x, float z, float depth, float height, float frontage,
                    const glm::vec3& c1, const glm::vec3& c2, float seed) {
    float podiumH = 4.2f;
    addSlot(era, glm::vec3(x - (x > 0 ? 2.2f : -2.2f), podiumH * 0.5f, z),
            glm::vec3(depth + 4.4f, podiumH, frontage + 3.0f), glm::vec4(c2, seed + 1.0f));
    addSlot(era, glm::vec3(x - (x > 0 ? 4.5f : -4.5f), podiumH + 0.18f, z),
            glm::vec3(0.35f, 0.28f, frontage + 2.6f), glm::vec4(c2, -1.0f));
    addSlot(era, glm::vec3(x, height * 0.5f, z),
            glm::vec3(depth, height, frontage), glm::vec4(c1, seed));
    addSlot(era, glm::vec3(x - (x > 0 ? depth * 0.52f : -depth * 0.52f), height * 0.52f, z),
            glm::vec3(0.45f, height * 0.94f, frontage * 0.24f), glm::vec4(c2, seed + 5.0f));
    float crownH = height * 0.18f;
    addSlot(era, glm::vec3(x, height + crownH * 0.5f, z),
            glm::vec3(depth * 0.68f, crownH, frontage * 0.68f), glm::vec4(c2, seed + 9.0f));
    addSlot(era, glm::vec3(x, height + crownH + 3.5f, z),
            glm::vec3(0.32f, 7.0f, 0.32f), glm::vec4(c1, -1.0f));
}

// Аккуратная ступенчатая пирамида (Сахара или храм джунглей) — все ярусы корректно состыкованы
void VegasCity::addPyramid(int era, float x, float z, float baseW, float totalH, int tiers,
                const glm::vec3& bodyCol, const glm::vec3& trimCol, bool withBeam) {
    float th = totalH / (float)tiers;
    for (int t = 0; t < tiers; ++t) {
        float u = (float)t / (float)tiers;
        float w = baseW * (1.0f - u * 0.90f);
        float y = th * 0.5f + (float)t * th;
        glm::vec3 col = bodyCol * (0.85f + 0.22f * u);
        addSlot(era, glm::vec3(x, y, z), glm::vec3(w, th + 0.06f, w), glm::vec4(col, -6.0f));
    }
    // Золотой/бирюзовый венец на вершине
    addSlot(era, glm::vec3(x, totalH + 1.2f, z), glm::vec3(baseW * 0.10f, 2.4f, baseW * 0.10f), glm::vec4(trimCol, -1.0f));
    if (withBeam) {
        addSlot(era, glm::vec3(x, totalH + 62.0f, z), glm::vec3(0.85f, 120.0f, 0.85f), glm::vec4(trimCol * 0.72f, -1.0f));
    }
}

// Гигантское тропическое дерево джунглей с корнями, стволом и многоуровневой пышной кроной
void VegasCity::addJungleTree(int era, float x, float z, float trunkH, float crownW, const glm::vec3& bark, const glm::vec3& leaf1, const glm::vec3& leaf2, const glm::vec3& bioGlow) {
    // Широкое основание-корни и прочный ствол
    addSlot(era, glm::vec3(x, 1.4f, z), glm::vec3(3.4f, 2.8f, 3.4f), glm::vec4(bark * 0.85f, -6.0f));
    addSlot(era, glm::vec3(x, trunkH * 0.5f, z), glm::vec3(2.1f, trunkH, 2.1f), glm::vec4(bark, -6.0f));
    // Три гармоничных яруса пышной кроны
    addSlot(era, glm::vec3(x, trunkH + 1.6f, z), glm::vec3(crownW, 3.6f, crownW), glm::vec4(leaf1, -6.0f));
    addSlot(era, glm::vec3(x, trunkH + 4.8f, z), glm::vec3(crownW * 0.78f, 3.2f, crownW * 0.78f), glm::vec4(leaf2, -6.0f));
    addSlot(era, glm::vec3(x, trunkH + 7.4f, z), glm::vec3(crownW * 0.48f, 2.4f, crownW * 0.48f), glm::vec4(leaf1 * 1.15f, -6.0f));
    // Биолюминесцентные светящиеся споры аккуратно под нижней кромкой кроны
    for (float ox : {-crownW * 0.34f, crownW * 0.34f}) {
        addSlot(era, glm::vec3(x + ox, trunkH - 0.6f, z), glm::vec3(0.32f, 1.4f, 0.32f), glm::vec4(bioGlow, -1.0f));
    }
}

// 0: 2077 NIGHT CITY — мрачный дождливый киберпанк с бетонными аркологиями и ОГРОМНЫМИ электронными билбордами
void VegasCity::buildEra0_NightCity2077() {
    glm::vec3 cyberTeal(0.08f, 0.82f, 0.92f);
    glm::vec3 cyberAmber(0.96f, 0.56f, 0.12f);
    glm::vec3 cyberCrimson(0.96f, 0.14f, 0.22f);
    glm::vec3 cyberMagenta(0.88f, 0.16f, 0.62f);
    glm::vec3 darkSteel(0.10f, 0.12f, 0.16f);
    glm::vec3 darkConcrete(0.14f, 0.15f, 0.19f);

    // Тёмные бетонные отбойники вдоль шоссе и наклонённые натриевые фонари (дождливая ночная магистраль)
    for (int side : {-1, 1}) {
        for (int k = 0; k < 20; ++k) {
            float pz = -12.0f - (float)k * 14.0f;
            glm::vec3 pCol = (k % 3 == 0) ? cyberCrimson : ((k % 3 == 1) ? cyberAmber : cyberTeal);
            addSlot(0, glm::vec3((float)side * 6.8f, 0.85f, pz), glm::vec3(0.55f, 1.7f, 13.6f), glm::vec4(darkConcrete, -5.0f));
            addSlot(0, glm::vec3((float)side * 7.6f, 4.5f, pz), glm::vec3(0.32f, 9.0f, 0.32f), glm::vec4(darkSteel, -5.0f));
            addSlot(0, glm::vec3((float)side * 6.9f, 8.9f, pz), glm::vec3(1.45f, 0.22f, 0.42f), glm::vec4(pCol * 0.9f, -1.0f));
            addSlot(0, glm::vec3((float)side * 10.2f, 0.30f, pz), glm::vec3(2.2f, 0.60f, 13.5f), glm::vec4(darkConcrete * 0.7f, -5.0f));
        }
    }

    // Массивные бруталистские небоскрёбы + КОЛОССАЛЬНЫЕ электронные LED-билборды (Params.a = -7..-10)
    for (int side : {-1, 1}) {
        for (int b = 0; b < 8; ++b) {
            float bz = -20.0f - (float)b * 34.0f;
            float bx = (float)side * (25.5f + (float)(b % 2) * 4.5f);
            float bh = 68.0f + (float)((b * 5 + (side > 0 ? 3 : 0)) % 4) * 14.0f;
            glm::vec3 c1 = (b % 2 == 0) ? cyberTeal : cyberCrimson;
            glm::vec3 c2 = (b % 3 == 0) ? cyberCrimson : ((b % 3 == 1) ? cyberAmber : cyberMagenta);
            float billStyle = -7.0f - (float)((b + (side > 0 ? 1 : 0)) % 4); // 4 разных анимированных дизайна билбордов

            // Основной массивный бетонно-стеклянный небоскрёб
            addSlot(0, glm::vec3(bx, 11.0f, bz), glm::vec3(20.0f, 22.0f, 24.0f), glm::vec4(c1, 10.0f + (float)b * 4.0f));
            addSlot(0, glm::vec3(bx, bh * 0.5f, bz), glm::vec3(15.0f, bh, 18.0f), glm::vec4(c1, 20.0f + (float)b * 5.0f));
            addSlot(0, glm::vec3(bx - (float)side * 4.5f, bh * 0.64f, bz), glm::vec3(11.0f, 4.5f, 19.0f), glm::vec4(darkSteel, -5.0f));

            // ОГРОМНЫЙ вертикальный электронный билборд на фасаде небоскрёба, обращённый к трассе
            float screenX = bx - (float)side * 8.4f;
            addSlot(0, glm::vec3(screenX, 24.0f + (float)(b % 2) * 8.0f, bz - 2.5f),
                    glm::vec3(0.65f, 26.0f, 13.5f), glm::vec4(c2, billStyle));
            // Стальные кронштейны крепления билборда к стене
            addSlot(0, glm::vec3(screenX + (float)side * 0.8f, 36.5f + (float)(b % 2) * 8.0f, bz - 2.5f),
                    glm::vec3(1.6f, 0.7f, 14.0f), glm::vec4(darkSteel, -5.0f));
            addSlot(0, glm::vec3(screenX + (float)side * 0.8f, 11.5f + (float)(b % 2) * 8.0f, bz - 2.5f),
                    glm::vec3(1.6f, 0.7f, 14.0f), glm::vec4(darkSteel, -5.0f));

            // Второй широкоформатный электронный рекламный щит на подиуме ближе к дороге
            addSlot(0, glm::vec3((float)side * 12.8f, 8.8f, bz + 12.0f),
                    glm::vec3(0.55f, 8.5f, 14.5f), glm::vec4((b % 2 == 0) ? cyberTeal : cyberAmber, -7.0f - (float)((b + 2) % 4)));
            addSlot(0, glm::vec3((float)side * 13.4f, 4.2f, bz + 12.0f),
                    glm::vec3(0.75f, 8.4f, 0.75f), glm::vec4(darkSteel, -5.0f));

            // Шпиль и красный авиационный маячок наверху башни
            addSlot(0, glm::vec3(bx, bh + 11.0f, bz), glm::vec3(0.65f, 22.0f, 0.65f), glm::vec4(darkSteel, -5.0f));
            addSlot(0, glm::vec3(bx, bh + 22.4f, bz), glm::vec3(1.25f, 1.25f, 1.25f), glm::vec4(cyberCrimson, -1.0f));

            // Второй эшелон гигантских фоновых башен в смоге
            addSlot(0, glm::vec3((float)side * 50.0f, (bh + 28.0f) * 0.5f, bz + 12.0f),
                    glm::vec3(18.0f, bh + 28.0f, 24.0f), glm::vec4(c1 * 0.7f, 70.0f + (float)b * 3.0f));
        }
    }

    // ОГРОМНЫЕ наддорожные электронные билборд-арки и эстакады с летающим трафиком прямо над трассой
    for (int hw = 0; hw < 5; ++hw) {
        float hz = -32.0f - (float)hw * 54.0f;
        float hy = 13.5f + (float)(hw % 2) * 4.5f;
        glm::vec3 hCol = (hw % 3 == 0) ? cyberCrimson : ((hw % 3 == 1) ? cyberTeal : cyberAmber);
        // Мощные стальные фермы арки по бокам дороги
        for (float sx : {-1.0f, 1.0f}) {
            addSlot(0, glm::vec3(sx * 13.5f, hy * 0.5f, hz), glm::vec3(1.2f, hy + 2.0f, 1.4f), glm::vec4(darkSteel, -5.0f));
        }
        addSlot(0, glm::vec3(0.0f, hy, hz), glm::vec3(42.0f, 1.1f, 2.4f), glm::vec4(darkSteel, -5.0f));
        // ОГРОМНЫЙ электронный экран 19 x 7.5 метра прямо над дорогой (обращён лицом к игроку)
        addSlot(0, glm::vec3(0.0f, hy + 4.4f, hz + 1.35f), glm::vec3(19.0f, 7.6f, 0.55f),
                glm::vec4(hCol, -7.0f - (float)(hw % 4)));
        addSlot(0, glm::vec3(0.0f, hy + 0.55f, hz + 1.25f), glm::vec3(28.0f, 0.16f, 0.16f), glm::vec4(hCol, -1.0f));
        // Летающие спиннеры-такси, пролетающие над эстакадой
        for (int fc = 0; fc < 4; ++fc) {
            float phase = (float)hw * 1.7f + (float)fc * 1.57f;
            addSlot(0, glm::vec3(0.0f, hy + 1.8f + (float)(fc % 2) * 1.6f, hz - 1.2f),
                    glm::vec3(3.2f, 0.65f, 1.35f), glm::vec4((fc % 2 == 0) ? cyberAmber : cyberCrimson, -1.0f), 5, phase);
        }
    }
}

// 1: 1985 LAS VEGAS STRIP
void VegasCity::buildEra1_LasVegas1985() {
    glm::vec3 cyan(0.12f, 0.88f, 1.00f), gold(1.00f, 0.76f, 0.22f), magenta(0.96f, 0.18f, 0.62f);
    glm::vec3 violet(0.58f, 0.28f, 0.98f), coral(0.98f, 0.44f, 0.16f), emerald(0.12f, 0.88f, 0.60f);

    for (int side : {-1, 1}) {
        for (int k = 0; k < 20; ++k) {
            float lz = -17.0f - (float)k * 14.0f;
            glm::vec3 lampCol = (k % 2 == 0) ? cyan : magenta;
            addSlot(1, glm::vec3((float)side * 6.4f, 3.0f, lz), glm::vec3(0.12f, 6.0f, 0.12f), glm::vec4(lampCol * 0.25f, 1.0f));
            addSlot(1, glm::vec3((float)side * 5.85f, 5.95f, lz), glm::vec3(1.15f, 0.12f, 0.16f), glm::vec4(lampCol * 0.85f, -1.0f));
            addSlot(1, glm::vec3((float)side * 10.6f, 0.22f, lz + 3.5f), glm::vec3(2.8f, 0.44f, 13.8f), glm::vec4(violet * 0.30f, 2.0f));
            addSlot(1, glm::vec3((float)side * 9.3f, 0.46f, lz + 3.5f), glm::vec3(0.12f, 0.08f, 13.2f), glm::vec4(lampCol * 0.65f, -1.0f));
        }
    }
    const float gantryZs[3] = {-58.0f, -148.0f, -238.0f};
    for (int g = 0; g < 3; ++g) {
        float gz = gantryZs[g];
        glm::vec3 gCol = (g == 0) ? cyan : ((g == 1) ? magenta : gold);
        for (float sx : {-1.0f, 1.0f}) addSlot(1, glm::vec3(sx * 11.8f, 5.4f, gz), glm::vec3(0.65f, 10.8f, 0.65f), glm::vec4(gCol, 4.0f));
        addSlot(1, glm::vec3(0.0f, 10.6f, gz), glm::vec3(24.2f, 0.55f, 0.65f), glm::vec4(gCol, 6.0f));
        addSlot(1, glm::vec3(0.0f, 11.4f, gz), glm::vec3(11.0f, 1.6f, 0.38f), glm::vec4(gCol, -2.0f));
    }

    addResortTower(1, -29.0f, -35.0f, 12.0f, 46.0f, 15.0f, gold, cyan, 11.0f);
    addSlot(1, glm::vec3(-26.5f, 18.5f, -21.0f), glm::vec3(10.0f, 37.0f, 11.5f), glm::vec4(gold, 17.0f));
    addSlot(1, glm::vec3(-26.5f, 18.5f, -49.0f), glm::vec3(10.0f, 37.0f, 11.5f), glm::vec4(gold, 23.0f));
    addSlot(1, glm::vec3(-14.2f, 0.25f, -35.0f), glm::vec3(3.6f, 0.5f, 36.0f), glm::vec4(cyan * 0.45f, -1.0f));
    for (int j = 0; j < 15; ++j)
        addSlot(1, glm::vec3(-14.2f, 2.0f, -19.0f - (float)j * 2.3f), glm::vec3(0.24f, 1.0f, 0.24f), glm::vec4((j % 3 == 0) ? gold : cyan, -1.0f), 2, (float)j * 0.52f);

    addResortTower(1, 28.5f, -24.0f, 11.5f, 43.0f, 13.0f, cyan, magenta, 14.0f);
    addResortTower(1, 28.5f, -46.0f, 11.5f, 49.0f, 13.0f, magenta, gold, 31.0f);
    addSlot(1, glm::vec3(26.5f, 31.5f, -35.0f), glm::vec3(6.5f, 3.8f, 11.0f), glm::vec4(gold, 19.0f));

    float pyrX = -34.0f, pyrZ = -95.0f;
    for (int tier = 0; tier < 14; ++tier) {
        float t = (float)tier / 13.0f;
        float w = 38.0f * (1.0f - t * 0.86f);
        addSlot(1, glm::vec3(pyrX, 1.5f + (float)tier * 3.0f, pyrZ), glm::vec3(w, 2.95f, w), glm::vec4(gold, 10.0f + (float)tier * 3.0f));
    }
    addSlot(1, glm::vec3(pyrX, 43.2f, pyrZ), glm::vec3(4.4f, 3.2f, 4.4f), glm::vec4(cyan, -1.0f));
    addSlot(1, glm::vec3(pyrX, 132.0f, pyrZ), glm::vec3(1.4f, 175.0f, 1.4f), glm::vec4(cyan * 0.82f, -1.0f));

    float wheelX = 25.5f, wheelZ = -95.0f, wheelCY = 23.5f;
    addSlot(1, glm::vec3(wheelX, 1.5f, wheelZ), glm::vec3(7.0f, 3.0f, 22.0f), glm::vec4(violet, 12.0f));
    for (float ox : {-1.5f, 1.5f})
        for (float oz : {-3.4f, 3.4f})
            addSlot(1, glm::vec3(wheelX + ox, wheelCY * 0.5f, wheelZ + oz), glm::vec3(0.80f, wheelCY, 0.80f), glm::vec4(cyan, 12.0f));
    addSlot(1, glm::vec3(wheelX, wheelCY, wheelZ), glm::vec3(3.8f, 2.4f, 2.4f), glm::vec4(cyan, -1.0f));
    for (int k = 0; k < 24; ++k) {
        float ang = 6.2831853f * (float)k / 24.0f;
        addSlot(1, glm::vec3(wheelX, wheelCY, wheelZ), glm::vec3(1.55f, 1.55f, 1.55f), glm::vec4((k % 2 == 0) ? magenta : cyan, -1.0f), 1, ang);
        addSlot(1, glm::vec3(wheelX, wheelCY, wheelZ), glm::vec3(0.60f, 0.60f, 0.60f), glm::vec4(gold, -1.0f), 3, ang);
    }

    for (int sl = 0; sl < 9; ++sl) {
        float yn = ((float)sl - 4.0f) / 4.3f;
        float r = std::sqrt(std::max(0.05f, 1.0f - yn * yn)) * 12.5f;
        addSlot(1, glm::vec3(-28.0f, 12.5f + yn * 11.2f, -139.0f), glm::vec3(r * 1.40f, 2.2f, r * 1.40f), glm::vec4((sl % 2 == 0) ? cyan : violet, -3.0f), 4, (float)sl * 0.45f);
    }
    addResortTower(1, -28.5f, -165.0f, 11.0f, 52.0f, 12.5f, gold, coral, 41.0f);
    addResortTower(1, 29.5f, -152.0f, 13.0f, 48.0f, 26.0f, coral, gold, 27.0f);
    addSlot(1, glm::vec3(14.2f, 0.30f, -152.0f), glm::vec3(3.4f, 0.6f, 24.0f), glm::vec4(coral * 0.45f, -1.0f));
    for (int j = 0; j < 9; ++j)
        addSlot(1, glm::vec3(14.2f, 2.0f, -142.0f - (float)j * 2.5f), glm::vec3(0.32f, 1.0f, 0.32f), glm::vec4((j % 2 == 0) ? coral : gold, -1.0f), 2, (float)j * 0.75f + 1.2f);

    float eX = -27.0f, eZ = -210.0f;
    for (float lx : {-5.2f, 5.2f})
        for (float lz : {-5.2f, 5.2f})
            addSlot(1, glm::vec3(eX + lx, 5.0f, eZ + lz), glm::vec3(3.4f, 10.0f, 3.4f), glm::vec4(gold, 18.0f));
    for (int lv = 1; lv < 11; ++lv) {
        float t = (float)lv / 10.0f;
        float w = 13.8f * (1.0f - t) * (1.0f - t) + 1.5f;
        addSlot(1, glm::vec3(eX, 6.0f + (float)lv * 6.0f, eZ), glm::vec3(w, 5.8f, w), glm::vec4(gold, 25.0f + (float)lv));
    }
    addSlot(1, glm::vec3(eX, 76.0f, eZ), glm::vec3(0.85f, 20.0f, 0.85f), glm::vec4(gold, -1.0f));

    float tX = 30.0f, tZ = -210.0f;
    addSlot(1, glm::vec3(tX, 6.0f, tZ), glm::vec3(14.5f, 12.0f, 14.5f), glm::vec4(violet, 14.0f));
    addSlot(1, glm::vec3(tX, 36.0f, tZ), glm::vec3(6.4f, 60.0f, 6.4f), glm::vec4(magenta, 18.0f));
    addSlot(1, glm::vec3(tX, 68.0f, tZ), glm::vec3(18.8f, 3.8f, 18.8f), glm::vec4(cyan, 22.0f));
    addSlot(1, glm::vec3(tX, 72.5f, tZ), glm::vec3(14.8f, 3.4f, 14.8f), glm::vec4(gold, 28.0f));
    addSlot(1, glm::vec3(tX, 93.0f, tZ), glm::vec3(0.95f, 30.0f, 0.95f), glm::vec4(cyan, -1.0f));

    addResortTower(1, -28.5f, -256.0f, 12.0f, 44.0f, 18.0f, emerald, cyan, 44.0f);
    addResortTower(1, 28.5f, -256.0f, 12.0f, 48.0f, 18.0f, violet, magenta, 38.0f);

    const glm::vec3 backCols[5] = {cyan, magenta, gold, violet, emerald};
    for (int side : {-1, 1}) {
        for (int b = 0; b < 10; ++b) {
            float bz = -14.0f - (float)b * 28.0f;
            float bx = (float)side * (47.0f + (float)(b % 2) * 5.5f);
            float bh = 52.0f + (float)((b * 3 + (side > 0 ? 2 : 0)) % 5) * 7.5f;
            addSlot(1, glm::vec3(bx, bh * 0.5f, bz), glm::vec3(13.0f, bh, 17.5f), glm::vec4(backCols[b % 5], 60.0f + (float)b * 3.7f));
        }
    }
}

// 2: NEO-TOKYO AKIRA (2019) — точная копия магистрали из аниме "Акира":
// бетонные стены шоссе с техническими нишами, тёмный парк с фонарями за стеной,
// плотная стена небоскрёбов Нео-Токио до 115м, купол Олимпийского стадиона 2020 и красные мосты с поездами-пулями
void VegasCity::buildEra2_NeoTokyoAkira() {
    glm::vec3 concreteWall(0.30f, 0.34f, 0.42f);
    glm::vec3 darkPark(0.06f, 0.14f, 0.11f);
    glm::vec3 crimson(0.95f, 0.16f, 0.22f);
    glm::vec3 darkSteel(0.16f, 0.17f, 0.23f);
    glm::vec3 warmLamp(1.00f, 0.86f, 0.42f);
    glm::vec3 neoCyan(0.28f, 0.82f, 1.00f);
    glm::vec3 neonWhite(0.92f, 0.95f, 1.00f);

    // Бетонные отбойные стены автострады Акиры с нишами + тёмная кромка парка + тёплые фонари вдоль стены
    for (int side : {-1, 1}) {
        for (int k = 0; k < 20; ++k) {
            float pz = -10.0f - (float)k * 14.0f;
            // Высокая серо-синяя бетонная стена шоссе (как на скриншоте #3)
            addSlot(2, glm::vec3((float)side * 6.9f, 1.15f, pz), glm::vec3(0.55f, 2.3f, 13.8f), glm::vec4(concreteWall, -5.0f));
            // Техническая ниша-лестница в стене каждые 14м
            addSlot(2, glm::vec3((float)side * 6.60f, 1.10f, pz), glm::vec3(0.12f, 2.1f, 0.85f), glm::vec4(darkSteel, -5.0f));
            // Пышные тёмные кроны парка сразу за бетонной стеной
            addSlot(2, glm::vec3((float)side * 10.6f, 2.4f, pz + 2.0f), glm::vec3(5.8f, 3.8f, 13.6f), glm::vec4(darkPark, -6.0f));
            addSlot(2, glm::vec3((float)side * 11.4f, 4.4f, pz - 3.0f), glm::vec3(4.2f, 2.6f, 7.5f), glm::vec4(darkPark * 1.2f, -6.0f));
            // Тёплый уличный фонарь над стеной парка (как светящиеся точки вверху на кадре Акиры)
            addSlot(2, glm::vec3((float)side * 7.6f, 4.8f, pz), glm::vec3(0.16f, 5.4f, 0.16f), glm::vec4(darkSteel, -5.0f));
            addSlot(2, glm::vec3((float)side * 7.2f, 7.4f, pz), glm::vec3(0.85f, 0.22f, 0.40f), glm::vec4(warmLamp, -1.0f));
        }
    }

    // 2 гигантских красных моста Нео-Токио (высоко над магистралью)
    const float bridgeZs[2] = {-65.0f, -210.0f};
    for (int br = 0; br < 2; ++br) {
        float bz = bridgeZs[br];
        for (float sx : {-1.0f, 1.0f}) {
            addSlot(2, glm::vec3(sx * 14.0f, 28.0f, bz), glm::vec3(2.4f, 56.0f, 3.2f), glm::vec4(crimson * 0.82f, -5.0f));
            addSlot(2, glm::vec3(sx * 14.0f, 57.5f, bz), glm::vec3(1.1f, 3.2f, 1.1f), glm::vec4(crimson, -1.0f));
        }
        addSlot(2, glm::vec3(0.0f, 19.0f, bz), glm::vec3(30.0f, 1.8f, 2.2f), glm::vec4(crimson * 0.85f, -5.0f));
        addSlot(2, glm::vec3(0.0f, 48.0f, bz), glm::vec3(32.0f, 2.4f, 2.6f), glm::vec4(crimson, -5.0f));
        addSlot(2, glm::vec3(0.0f, 20.6f, bz), glm::vec3(14.0f, 2.2f, 0.55f), glm::vec4(crimson, -2.0f));
    }

    // КУЛЬТОВЫЙ ОЛИМПИЙСКИЙ СТАДИОН НЕО-ТОКИО 2020 (из финала "Акиры") с прожекторными башнями
    float stX = -32.0f, stZ = -142.0f;
    for (int sl = 0; sl < 6; ++sl) {
        float u = (float)sl / 5.0f;
        float r = std::sqrt(std::max(0.18f, 1.0f - u * u)) * 18.0f;
        addSlot(2, glm::vec3(stX, 2.5f + (float)sl * 3.2f, stZ), glm::vec3(r * 1.4f, 3.1f, r * 1.4f),
                glm::vec4((sl == 2 || sl == 4) ? neoCyan : concreteWall * 1.25f, (sl == 2 || sl == 4) ? -3.0f : -5.0f));
    }
    for (float ox : {-11.0f, 11.0f}) {
        for (float oz : {-11.0f, 11.0f}) {
            addSlot(2, glm::vec3(stX + ox, 18.0f, stZ + oz), glm::vec3(1.4f, 36.0f, 1.4f), glm::vec4(darkSteel, -5.0f));
            addSlot(2, glm::vec3(stX + ox, 37.0f, stZ + oz), glm::vec3(2.8f, 1.4f, 2.8f), glm::vec4(neonWhite, -1.0f));
        }
    }

    // Плотный футуристический скайлайн Нео-Токио (2 эшелона высоток до 115м с маячками и вертикальными вывесками)
    for (int side : {-1, 1}) {
        for (int b = 0; b < 10; ++b) {
            float bz = -14.0f - (float)b * 28.0f;
            // Оставляем пространство для Олимпийского стадиона слева в районе z = -142
            if (side < 0 && std::fabs(bz - stZ) < 28.0f) continue;

            float bx = (float)side * (26.0f + (float)(b % 2) * 5.0f);
            float bh = 58.0f + (float)((b * 7 + (side > 0 ? 3 : 0)) % 5) * 12.0f;
            glm::vec3 c1 = (b % 3 == 0) ? neoCyan : ((b % 3 == 1) ? crimson : warmLamp);
            glm::vec3 c2 = (b % 2 == 0) ? crimson : neoCyan;

            addSlot(2, glm::vec3(bx, bh * 0.5f, bz), glm::vec3(15.0f, bh, 17.5f), glm::vec4(c1, 18.0f + (float)b * 4.0f));
            addSlot(2, glm::vec3(bx, bh + 4.5f, bz), glm::vec3(10.0f, 9.0f, 11.0f), glm::vec4(c2, 42.0f + (float)b));
            // Большие анимированные электронные билборды Нео-Токио (капсула Акиры, иероглифы, сканеры)
            addSlot(2, glm::vec3(bx - (float)side * 8.2f, 24.0f + (float)(b % 2) * 8.0f, bz - 2.0f),
                    glm::vec3(0.60f, 24.0f, 11.5f), glm::vec4(c2, -7.0f - (float)(b % 4)));
            addSlot(2, glm::vec3(bx - (float)side * 8.2f, 10.5f, bz + 7.5f),
                    glm::vec3(0.55f, 10.0f, 6.2f), glm::vec4(c1, -7.0f - (float)((b + 1) % 4)));
            // Красный авиационный маячок на шпиле каждой высотки Нео-Токио
            addSlot(2, glm::vec3(bx, bh + 15.0f, bz), glm::vec3(0.55f, 12.0f, 0.55f), glm::vec4(darkSteel, -5.0f));
            addSlot(2, glm::vec3(bx, bh + 21.8f, bz), glm::vec3(1.1f, 1.1f, 1.1f), glm::vec4(crimson, -1.0f));

            // Второй эшелон мега-башен Нео-Токио
            addSlot(2, glm::vec3((float)side * 48.0f, (bh + 28.0f) * 0.5f, bz + 10.0f),
                    glm::vec3(16.0f, bh + 28.0f, 22.0f), glm::vec4((b % 2 == 0) ? neoCyan : warmLamp, 78.0f + (float)b * 3.0f));
        }
    }

    // 4 эстакады скоростного монорельса с несущимися поездами-пулями сверху
    for (int tr = 0; tr < 4; ++tr) {
        float tz = -22.0f - (float)tr * 70.0f;
        float ty = 13.5f + (float)(tr % 2) * 5.0f;
        addSlot(2, glm::vec3(0.0f, ty, tz), glm::vec3(54.0f, 1.1f, 3.0f), glm::vec4(darkSteel, -5.0f));
        addSlot(2, glm::vec3(0.0f, ty + 1.5f, tz), glm::vec3(14.0f, 1.5f, 1.8f), glm::vec4(crimson, -1.0f), 7, (float)tr * 2.1f);
    }
}

