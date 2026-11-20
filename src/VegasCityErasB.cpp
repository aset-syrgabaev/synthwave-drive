#include "VegasCity.h"

// Генерация локаций 3-7 (Arctic, Orbital, Sahara, Jungle, Mars) и инстансированный рендер

// 3: ARCTIC AURORA — ледники, научные купола, радары и северное сияние
void VegasCity::buildEra3_ArcticAurora() {
    glm::vec3 glacierIce(0.42f, 0.80f, 0.96f), deepIce(0.18f, 0.52f, 0.78f);
    glm::vec3 labWhite(0.76f, 0.84f, 0.92f), auroraMint(0.14f, 0.96f, 0.74f), cryoCyan(0.18f, 0.86f, 1.00f);

    for (int side : {-1, 1}) {
        for (int k = 0; k < 20; ++k) {
            float pz = -12.0f - (float)k * 14.0f;
            addSlot(3, glm::vec3((float)side * 6.8f, 2.8f, pz), glm::vec3(0.32f, 5.6f, 0.32f), glm::vec4(labWhite, -5.0f));
            addSlot(3, glm::vec3((float)side * 6.4f, 5.6f, pz), glm::vec3(1.1f, 0.18f, 0.22f), glm::vec4((k % 2 == 0) ? auroraMint : cryoCyan, -1.0f));
            addSlot(3, glm::vec3((float)side * 9.6f, 0.35f, pz), glm::vec3(2.4f, 0.70f, 13.6f), glm::vec4(glacierIce * 0.7f, -4.0f));
        }
    }
    for (int side : {-1, 1}) {
        for (int ic = 0; ic < 10; ++ic) {
            float iz = -18.0f - (float)ic * 27.5f;
            float ix = (float)side * (26.0f + (float)(ic % 3) * 6.5f);
            float ih = 28.0f + (float)((ic * 5 + (side > 0 ? 3 : 0)) % 4) * 11.0f;
            addSlot(3, glm::vec3(ix, ih * 0.45f, iz), glm::vec3(9.5f, ih, 11.0f), glm::vec4(glacierIce, -4.0f), 0, 0.0f, (float)side * 0.16f);
            addSlot(3, glm::vec3(ix + (float)side * 8.5f, ih * 0.32f, iz + 6.5f), glm::vec3(11.0f, ih * 0.72f, 13.0f), glm::vec4(deepIce, -4.0f), 0, 0.0f, -(float)side * 0.12f);
        }
    }
    const float domeZs[4] = {-42.0f, -112.0f, -182.0f, -248.0f};
    for (int d = 0; d < 4; ++d) {
        float sx = (d % 2 == 0) ? -1.0f : 1.0f;
        float dx = sx * 22.0f, dz = domeZs[d];
        for (int sl = 0; sl < 6; ++sl) {
            float u = (float)sl / 5.0f;
            float r = std::sqrt(std::max(0.08f, 1.0f - u * u)) * 12.5f;
            addSlot(3, glm::vec3(dx, 1.6f + (float)sl * 2.1f, dz), glm::vec3(r * 1.40f, 2.0f, r * 1.40f),
                    glm::vec4((sl == 1 || sl == 2) ? cryoCyan : labWhite, (sl == 1 || sl == 2) ? -3.0f : -5.0f));
        }
        addSlot(3, glm::vec3(-dx, 14.0f, dz), glm::vec3(3.2f, 28.0f, 3.2f), glm::vec4(labWhite * 0.85f, -5.0f));
        addSlot(3, glm::vec3(-dx, 29.5f, dz), glm::vec3(9.0f, 1.8f, 1.8f), glm::vec4(auroraMint, -1.0f), 6, (float)d * 1.4f);
    }
    for (int g = 0; g < 3; ++g) {
        float gz = -65.0f - (float)g * 90.0f;
        for (float sx : {-1.0f, 1.0f})
            addSlot(3, glm::vec3(sx * 11.8f, 7.2f, gz), glm::vec3(1.8f, 14.4f, 2.2f), glm::vec4(labWhite, -5.0f));
        addSlot(3, glm::vec3(0.0f, 14.2f, gz), glm::vec3(25.4f, 1.4f, 2.2f), glm::vec4(labWhite, -5.0f));
        addSlot(3, glm::vec3(0.0f, 13.3f, gz), glm::vec3(21.0f, 0.24f, 0.35f), glm::vec4(auroraMint, -1.0f));
    }
}

// 4: ORBITAL STATION — космическая магистраль, солнечные батареи и орбитальные шаттлы
void VegasCity::buildEra4_OrbitalStation() {
    glm::vec3 titanium(0.46f, 0.52f, 0.64f), darkHull(0.14f, 0.17f, 0.24f);
    glm::vec3 plasmaBlue(0.18f, 0.78f, 1.00f), solarBlue(0.12f, 0.42f, 0.96f), dockAmber(1.00f, 0.65f, 0.18f);

    for (int side : {-1, 1}) {
        for (int k = 0; k < 20; ++k) {
            float pz = -10.0f - (float)k * 14.0f;
            addSlot(4, glm::vec3((float)side * 7.2f, 0.75f, pz), glm::vec3(1.2f, 1.5f, 13.6f), glm::vec4(darkHull, -5.0f));
            addSlot(4, glm::vec3((float)side * 6.55f, 1.35f, pz), glm::vec3(0.14f, 0.14f, 4.5f), glm::vec4((k % 2 == 0) ? plasmaBlue : dockAmber, -1.0f));
        }
    }
    for (int a = 0; a < 7; ++a) {
        float az = -20.0f - (float)a * 40.0f;
        for (float sx : {-1.0f, 1.0f}) {
            addSlot(4, glm::vec3(sx * 13.0f, 12.0f, az), glm::vec3(2.2f, 24.0f, 3.4f), glm::vec4(titanium, -5.0f));
            addSlot(4, glm::vec3(sx * 11.7f, 12.0f, az), glm::vec3(0.24f, 22.0f, 0.35f), glm::vec4(plasmaBlue, -1.0f));
        }
        addSlot(4, glm::vec3(0.0f, 24.0f, az), glm::vec3(28.2f, 2.4f, 3.6f), glm::vec4(titanium, -5.0f));
        addSlot(4, glm::vec3(0.0f, 22.6f, az), glm::vec3(22.0f, 0.35f, 0.55f), glm::vec4(plasmaBlue, -1.0f));
    }
    for (int side : {-1, 1}) {
        for (int p = 0; p < 5; ++p) {
            float pz = -36.0f - (float)p * 56.0f;
            float px = (float)side * 28.0f;
            addSlot(4, glm::vec3(px, 18.0f, pz), glm::vec3(7.5f, 36.0f, 7.5f), glm::vec4(darkHull, -5.0f));
            addSlot(4, glm::vec3(px, 18.0f, pz), glm::vec3(8.0f, 28.0f, 1.4f), glm::vec4(plasmaBlue, -1.0f));
            addSlot(4, glm::vec3(px + (float)side * 16.0f, 22.0f, pz), glm::vec3(24.0f, 0.65f, 8.5f), glm::vec4(solarBlue, -3.0f), 6, (float)p * 0.8f);
            addSlot(4, glm::vec3(px + (float)side * 16.0f, 12.0f, pz), glm::vec3(20.0f, 0.65f, 6.5f), glm::vec4(solarBlue, -3.0f), 6, (float)p * 0.8f + 1.2f);
        }
    }
    for (int sh = 0; sh < 5; ++sh) {
        float sz = -32.0f - (float)sh * 54.0f;
        addSlot(4, glm::vec3(0.0f, 15.5f + (float)(sh % 3) * 3.0f, sz), glm::vec3(5.8f, 1.1f, 2.2f), glm::vec4((sh % 2 == 0) ? plasmaBlue : dockAmber, -1.0f), 7, (float)sh * 1.9f);
    }
}

// 5: SAHARA PYRAMIDS — гармоничный золотой кибер-Египет: великие пирамиды, обелиски и арки оазиса
void VegasCity::buildEra5_SaharaPyramids() {
    glm::vec3 goldSand(0.76f, 0.56f, 0.28f);
    glm::vec3 obsidian(0.14f, 0.13f, 0.16f);
    glm::vec3 brightGold(1.00f, 0.78f, 0.22f);
    glm::vec3 oasisTeal(0.14f, 0.90f, 0.86f);

    // Аккуратные каменные обелиски с золотой подсветкой вдоль пустынного проспекта
    for (int side : {-1, 1}) {
        for (int k = 0; k < 16; ++k) {
            float oz = -14.0f - (float)k * 17.5f;
            float ox = (float)side * 7.6f;
            addSlot(5, glm::vec3(ox, 0.5f, oz), glm::vec3(1.5f, 1.0f, 1.5f), glm::vec4(goldSand * 0.82f, -6.0f));
            addSlot(5, glm::vec3(ox, 4.0f, oz), glm::vec3(0.85f, 6.2f, 0.85f), glm::vec4((k % 2 == 0) ? goldSand : obsidian, -6.0f));
            addSlot(5, glm::vec3(ox, 7.5f, oz), glm::vec3(0.65f, 0.95f, 0.65f), glm::vec4((k % 2 == 0) ? brightGold : oasisTeal, -1.0f));
            addSlot(5, glm::vec3((float)side * 9.8f, 0.28f, oz), glm::vec3(2.2f, 0.56f, 17.0f), glm::vec4(goldSand * 0.65f, -6.0f));
        }
    }

    // 2 монументальные гладкие пирамиды вдалеке (28 тонких слоёв без ступенек) — а сами песчаные горы рисует Mountain.h
    addPyramid(5, -52.0f,  -72.0f, 46.0f, 38.0f, 28, goldSand, brightGold, true);
    addPyramid(5,  54.0f, -195.0f, 48.0f, 42.0f, 28, obsidian, oasisTeal,  true);

    // Солнечные зеркальные башни пустынного оазиса (концентраторы энергии)
    for (int st = 0; st < 2; ++st) {
        float sx = (st == 0) ? 1.0f : -1.0f;
        float sz = (st == 0) ? -105.0f : -232.0f;
        addSlot(5, glm::vec3(sx * 24.0f, 11.0f, sz), glm::vec3(2.8f, 22.0f, 2.8f), glm::vec4(obsidian, -5.0f));
        addSlot(5, glm::vec3(sx * 24.0f, 23.2f, sz), glm::vec3(4.2f, 3.2f, 4.2f), glm::vec4(brightGold, -1.0f));
        for (int ring = -1; ring <= 1; ring += 2) {
            addSlot(5, glm::vec3(sx * (24.0f + (float)ring * 6.5f), 2.2f, sz), glm::vec3(4.5f, 0.35f, 6.0f), glm::vec4(oasisTeal, -3.0f));
        }
    }

    // 3 величественные храмовые арки-пилоны над трассой (чёткая симметрия, высоко над дорогой)
    const float gateZs[3] = {-72.0f, -155.0f, -238.0f};
    for (int g = 0; g < 3; ++g) {
        float gz = gateZs[g];
        for (float sx : {-1.0f, 1.0f}) {
            addSlot(5, glm::vec3(sx * 12.2f, 7.5f, gz), glm::vec3(4.2f, 15.0f, 4.6f), glm::vec4(goldSand * 0.92f, -6.0f));
            addSlot(5, glm::vec3(sx * 10.0f, 7.5f, gz), glm::vec3(0.24f, 12.0f, 0.45f), glm::vec4(oasisTeal, -1.0f));
        }
        addSlot(5, glm::vec3(0.0f, 15.2f, gz), glm::vec3(29.0f, 2.4f, 4.4f), glm::vec4(goldSand, -6.0f));
        addSlot(5, glm::vec3(0.0f, 13.8f, gz), glm::vec3(20.0f, 0.35f, 0.55f), glm::vec4(brightGold, -1.0f));
    }

    // Оазисные колоннады храмов в промежутках между пирамидами
    for (int c = 0; c < 4; ++c) {
        float cz = -22.0f - (float)c * 68.0f;
        float sx = (c % 2 == 0) ? 1.0f : -1.0f;
        addSlot(5, glm::vec3(sx * 22.0f, 0.8f, cz), glm::vec3(11.0f, 1.6f, 24.0f), glm::vec4(goldSand * 0.8f, -6.0f));
        for (int col = -2; col <= 2; ++col) {
            addSlot(5, glm::vec3(sx * 18.0f, 6.2f, cz + (float)col * 4.5f), glm::vec3(1.5f, 9.6f, 1.5f), glm::vec4(goldSand * 1.05f, -6.0f));
        }
        addSlot(5, glm::vec3(sx * 20.0f, 11.8f, cz), glm::vec3(10.0f, 1.8f, 23.0f), glm::vec4(obsidian, -6.0f));
        addSlot(5, glm::vec3(sx * 15.0f, 11.8f, cz), glm::vec3(0.35f, 0.35f, 22.0f), glm::vec4(brightGold, -1.0f));
    }
}

// 6: NEON JUNGLE — биолюминесцентный ночной лес Пандоры, древние храмы и светящиеся арки лиан
void VegasCity::buildEra6_NeonJungle() {
    glm::vec3 bark(0.24f, 0.16f, 0.11f);
    glm::vec3 leafDeep(0.08f, 0.32f, 0.18f);
    glm::vec3 leafBright(0.12f, 0.48f, 0.26f);
    glm::vec3 mossStone(0.22f, 0.30f, 0.24f);
    glm::vec3 bioCyan(0.14f, 0.96f, 0.82f);
    glm::vec3 bioViolet(0.76f, 0.24f, 0.96f);

    // Светящиеся природные кристаллы-маяки вдоль тропической дороги
    for (int side : {-1, 1}) {
        for (int k = 0; k < 20; ++k) {
            float pz = -12.0f - (float)k * 14.0f;
            glm::vec3 glowC = (k % 2 == 0) ? bioCyan : bioViolet;
            addSlot(6, glm::vec3((float)side * 7.2f, 0.9f, pz), glm::vec3(0.95f, 1.8f, 0.95f), glm::vec4(mossStone, -6.0f));
            addSlot(6, glm::vec3((float)side * 7.2f, 2.6f, pz), glm::vec3(0.42f, 1.9f, 0.42f), glm::vec4(glowC, -1.0f));
            addSlot(6, glm::vec3((float)side * 9.8f, 0.35f, pz), glm::vec3(2.4f, 0.70f, 13.6f), glm::vec4(leafDeep, -6.0f));
        }
    }

    // Могучие гигантские деревья тропического леса с широкими кронами
    for (int side : {-1, 1}) {
        for (int t = 0; t < 8; ++t) {
            float tz = -18.0f - (float)t * 34.0f;
            float tx = (float)side * (20.0f + (float)(t % 2) * 5.0f);
            float th = 16.0f + (float)((t * 3 + (side > 0 ? 1 : 0)) % 4) * 3.5f;
            float cw = 13.5f + (float)(t % 3) * 2.5f;
            addJungleTree(6, tx, tz, th, cw, bark, leafDeep, leafBright, (t % 2 == 0) ? bioCyan : bioViolet);
            // Второй эшелон ещё более высоких деревьев в глубине джунглей
            addJungleTree(6, (float)side * 42.0f, tz + 14.0f, th + 10.0f, cw + 5.0f, bark, leafDeep * 0.85f, leafBright * 0.9f, bioCyan);
        }
    }

    // 2 древних каменных храма майя среди джунглей с изумрудными алтарями
    addPyramid(6, -34.0f, -104.0f, 30.0f, 24.0f, 10, mossStone, bioCyan, false);
    addPyramid(6,  34.0f, -208.0f, 32.0f, 26.0f, 11, mossStone, bioViolet, false);

    // 3 высоких сводчатых арки из древних деревьев и светящихся лиан над трассой
    const float archZs[3] = {-56.0f, -148.0f, -242.0f};
    for (int a = 0; a < 3; ++a) {
        float az = archZs[a];
        for (float sx : {-1.0f, 1.0f}) {
            addSlot(6, glm::vec3(sx * 11.8f, 7.5f, az), glm::vec3(2.8f, 15.0f, 3.0f), glm::vec4(bark, -6.0f));
        }
        addSlot(6, glm::vec3(0.0f, 15.4f, az), glm::vec3(27.0f, 2.6f, 4.2f), glm::vec4(leafBright, -6.0f));
        addSlot(6, glm::vec3(0.0f, 13.6f, az), glm::vec3(20.0f, 0.30f, 0.45f), glm::vec4((a % 2 == 0) ? bioCyan : bioViolet, -1.0f));
    }
}

// 7: MARS COLONY 2099 — пресс-купола с зелёными теплицами, космодром с ракетами и шлюзовые мосты
void VegasCity::buildEra7_MarsColony() {
    glm::vec3 marsRock(0.56f, 0.22f, 0.14f);
    glm::vec3 marsDust(0.70f, 0.32f, 0.18f);
    glm::vec3 habWhite(0.84f, 0.86f, 0.90f);
    glm::vec3 nasaOrange(0.96f, 0.42f, 0.12f);
    glm::vec3 hydroGreen(0.18f, 0.86f, 0.46f);
    glm::vec3 beaconCyan(0.22f, 0.88f, 1.00f);

    // Осветительные шлюзовые маяки и защитные барьеры марсианского шоссе
    for (int side : {-1, 1}) {
        for (int k = 0; k < 20; ++k) {
            float pz = -12.0f - (float)k * 14.0f;
            addSlot(7, glm::vec3((float)side * 7.2f, 2.6f, pz), glm::vec3(0.36f, 5.2f, 0.36f), glm::vec4(habWhite * 0.82f, -5.0f));
            addSlot(7, glm::vec3((float)side * 6.8f, 5.2f, pz), glm::vec3(1.15f, 0.20f, 0.24f), glm::vec4((k % 2 == 0) ? nasaOrange : beaconCyan, -1.0f));
            addSlot(7, glm::vec3((float)side * 9.6f, 0.35f, pz), glm::vec3(2.2f, 0.70f, 13.5f), glm::vec4(marsRock, -6.0f));
        }
    }

    // 4 жилых марсианских биокупола с зелёными гидропонными садами внутри
    const float domeZs[4] = {-38.0f, -108.0f, -178.0f, -248.0f};
    for (int d = 0; d < 4; ++d) {
        float sx = (d % 2 == 0) ? -1.0f : 1.0f;
        float dx = sx * 23.0f, dz = domeZs[d];
        // Бетонное герметичное основание модуля
        addSlot(7, glm::vec3(dx, 1.2f, dz), glm::vec3(20.0f, 2.4f, 20.0f), glm::vec4(habWhite * 0.75f, -5.0f));
        for (int sl = 0; sl < 6; ++sl) {
            float u = (float)sl / 5.0f;
            float r = std::sqrt(std::max(0.08f, 1.0f - u * u)) * 11.8f;
            bool glassRow = (sl >= 1 && sl <= 3);
            addSlot(7, glm::vec3(dx, 2.8f + (float)sl * 2.0f, dz), glm::vec3(r * 1.42f, 1.95f, r * 1.42f),
                    glm::vec4(glassRow ? hydroGreen : habWhite, glassRow ? -3.0f : -5.0f));
        }
        // Напротив купола — стартовая площадка марсианского космодрома со звездолётом
        float rx = -dx;
        addSlot(7, glm::vec3(rx, 1.5f, dz), glm::vec3(16.0f, 3.0f, 16.0f), glm::vec4(marsRock * 0.8f, -5.0f));
        // Корпус вертикальной ракеты (нержавейка + оранжевые ступени + носовой обтекатель)
        addSlot(7, glm::vec3(rx, 16.0f, dz), glm::vec3(4.2f, 26.0f, 4.2f), glm::vec4(habWhite, -5.0f));
        addSlot(7, glm::vec3(rx, 31.5f, dz), glm::vec3(2.8f, 5.5f, 2.8f), glm::vec4(nasaOrange, -5.0f));
        addSlot(7, glm::vec3(rx, 35.8f, dz), glm::vec3(1.2f, 3.5f, 1.2f), glm::vec4(beaconCyan, -1.0f));
        // Башня обслуживания космодрома рядом с ракетой (аккуратно сбоку, не пересекается с ракетой)
        addSlot(7, glm::vec3(rx + (-sx) * 6.0f, 16.5f, dz), glm::vec3(2.2f, 30.0f, 2.2f), glm::vec4(nasaOrange * 0.88f, -5.0f));
        addSlot(7, glm::vec3(rx + (-sx) * 6.0f, 32.5f, dz), glm::vec3(1.2f, 1.2f, 1.2f), glm::vec4(nasaOrange, -1.0f));
    }

    // 3 герметичных шлюзовых моста через марсианское шоссе + автономные грузовые дроны сверху
    const float bridgeZs[3] = {-74.0f, -144.0f, -214.0f};
    for (int b = 0; b < 3; ++b) {
        float bz = bridgeZs[b];
        for (float sx : {-1.0f, 1.0f}) {
            addSlot(7, glm::vec3(sx * 12.2f, 7.2f, bz), glm::vec3(2.4f, 14.4f, 3.0f), glm::vec4(habWhite * 0.85f, -5.0f));
        }
        addSlot(7, glm::vec3(0.0f, 14.2f, bz), glm::vec3(26.8f, 2.8f, 3.4f), glm::vec4(habWhite, -5.0f));
        addSlot(7, glm::vec3(0.0f, 14.2f, bz + 1.78f), glm::vec3(20.0f, 1.1f, 0.25f), glm::vec4(beaconCyan, -3.0f));
        addSlot(7, glm::vec3(0.0f, 18.0f, bz), glm::vec3(4.5f, 1.0f, 1.8f), glm::vec4(nasaOrange, -1.0f), 7, (float)b * 2.3f);
    }

    // Солнечные фермы и антенны связи колонии (марсианские горы на фоне рисует гладкий рельеф Mountain.h)
    for (int side : {-1, 1}) {
        for (int r = 0; r < 5; ++r) {
            float rz = -22.0f - (float)r * 54.0f;
            float rx = (float)side * 36.0f;
            addSlot(7, glm::vec3(rx, 4.2f, rz), glm::vec3(1.2f, 8.4f, 1.2f), glm::vec4(habWhite * 0.85f, -5.0f));
            addSlot(7, glm::vec3(rx, 7.6f, rz), glm::vec3(9.5f, 0.35f, 4.2f), glm::vec4(beaconCyan * 0.75f, -3.0f));
            addSlot(7, glm::vec3(rx, 9.2f, rz), glm::vec3(0.6f, 0.6f, 0.6f), glm::vec4(nasaOrange, -1.0f));
        }
    }
}

void VegasCity::uploadEraParams(int era) {
    activeEra = era;
    const auto& slots = eraSlots[era];
    std::vector<glm::vec4> params;
    params.reserve(slots.size());
    for (const Slot& s : slots) params.push_back(s.params);

    glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
    glBufferData(GL_ARRAY_BUFFER, slots.size() * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, paramVBO);
    glBufferData(GL_ARRAY_BUFFER, params.size() * sizeof(glm::vec4), params.data(), GL_STATIC_DRAW);
}


VegasCity::VegasCity() {
    std::vector<float> v;
    Primitives::appendBox(v, glm::vec3(0.0f), glm::vec3(1.0f));
    vertexCount = static_cast<int>(v.size() / 6);

    buildEra0_NightCity2077();
    buildEra1_LasVegas1985();
    buildEra2_NeoTokyoAkira();
    buildEra3_ArcticAurora();
    buildEra4_OrbitalStation();
    buildEra5_SaharaPyramids();
    buildEra6_NeonJungle();
    buildEra7_MarsColony();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &matrixVBO);
    glGenBuffers(1, &paramVBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
    for (int i = 0; i < 4; ++i) {
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(3 + i);
        glVertexAttribDivisor(3 + i, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, paramVBO);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glEnableVertexAttribArray(7);
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);
    uploadEraParams(0);
}

void VegasCity::draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float scroll, const glm::vec3& camPos,
          float time, int era, const glm::vec3& fogColor) {
    era = std::clamp(era, 0, 7);
    if (era != activeEra) uploadEraParams(era);

    const auto& slots = eraSlots[era];
    std::vector<glm::mat4> mats;
    mats.reserve(slots.size());
    for (const Slot& s : slots) {
        float z = std::fmod(s.pos.z + scroll + 268.0f, kLoop);
        if (z < 0.0f) z += kLoop;
        z -= 268.0f;

        glm::vec3 p(s.pos.x, s.pos.y, z);
        glm::vec3 sz = s.size;
        float rotY = 0.0f;

        if (s.animType == 1) {
            float a = s.animPhase + time * 0.45f;
            p.y = s.pos.y + std::sin(a) * 16.5f;
            p.z = z + std::cos(a) * 16.5f;
        } else if (s.animType == 2) {
            float jetH = 1.6f + 8.0f * (0.5f + 0.5f * std::sin(time * 3.6f + s.animPhase));
            sz.y = jetH;
            p.y  = 0.5f + jetH * 0.5f;
        } else if (s.animType == 3) {
            float a = s.animPhase + time * 0.45f;
            p.y = s.pos.y + std::sin(a) * 11.0f;
            p.z = z + std::cos(a) * 11.0f;
        } else if (s.animType == 4) {
            float pulse = 1.0f + 0.025f * std::sin(time * 2.8f + s.animPhase);
            sz.x *= pulse;
            sz.z *= pulse;
        } else if (s.animType == 5) {
            float dir = (std::fmod(s.animPhase, 2.0f) < 1.0f) ? 1.0f : -1.0f;
            float cycle = std::fmod(time * 14.0f + s.animPhase * 11.0f, 52.0f) - 26.0f;
            p.x = cycle * dir;
        } else if (s.animType == 6) {
            rotY = time * 0.65f + s.animPhase;
        } else if (s.animType == 7) {
            float dir = (std::fmod(s.animPhase, 2.0f) < 1.0f) ? 1.0f : -1.0f;
            float cycle = std::fmod(time * 24.0f + s.animPhase * 15.0f, 68.0f) - 34.0f;
            p.x = cycle * dir;
        }

        glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
        if (rotY != 0.0f) m = glm::rotate(m, rotY, glm::vec3(0, 1, 0));
        if (s.rotZ != 0.0f) m = glm::rotate(m, s.rotZ, glm::vec3(0, 0, 1));
        mats.push_back(glm::scale(m, sz));
    }

    glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mats.size() * sizeof(glm::mat4), mats.data());

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("viewPos", camPos);
    shader.setFloat("time", time);
    shader.setVec3("fogColor", fogColor);
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, static_cast<int>(slots.size()));
    glBindVertexArray(0);
}
