#version 330 core
// Фрагментный шейдер дороги и окружения: у каждого из 8 биомов своё уникальное покрытие дороги,
// разметка, бордюры и обочины (мокрый асфальт, бетонная эстакада Токио, треснувший лёд Арктики,
// титановая палуба орбиты, занесённая песком трасса Сахары, мшистый камень джунглей, базальт Марса).
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 camPos;         // позиция камеры
uniform vec3 carPos;         // позиция машины/мотоцикла
uniform bool headlightsOn;   // включены ли фары (клавиша L)
uniform vec3 gridLineColor;  // акцентный цвет линий текущей локации
uniform vec3 roadColor;      // базовый тон дороги текущей локации
uniform int era;             // номер биома (0..7)
uniform float time;          // время сцены для анимации ряби и потоков
uniform vec3 headlightColor; // цвет луча фар (меняется при смене скина машины)

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float valueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbmWet(vec2 p) {
    float v = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; ++i) {
        v += amp * valueNoise(p);
        p *= 2.05;
        amp *= 0.5;
    }
    return v;
}

void main()
{
    // ВАЖНО ДЛЯ ЗАЩИТЫ (критерий R5 + ядро Темы #13 — антиалиасинг сетки через fwidth()):
    // Обычная сетка вдали на горизонте начинает сильно рябить (эффект муара / aliasing).
    // Функция GLSL fwidth(gridUV) возвращает скорость изменения UV между соседними пикселями экрана (|dUV/dx| + |dUV/dy|).
    // Деля расстояние до линии сетки на fwidth(gridUV), мы делаем линии идеально чёткими вблизи
    // и плавно размываем их вдали БЕЗ единой PNG-текстуры и мипмапов!
    vec2 gridUV = TexCoord;
    vec2 gridD = abs(fract(gridUV - 0.5) - 0.5) / max(fwidth(gridUV), vec2(0.0001));
    float line = 1.0 - min(min(gridD.x, gridD.y), 1.0); // 1.0 точно на линии сетки, 0.0 внутри клетки

    float x = FragPos.x;
    float roadHalfWidth = 4.80;
    bool onRoad = abs(x) < roadHalfWidth;
    bool isWall = abs(x) >= 4.78 && abs(x) <= 5.12 && FragPos.y < -0.05;

    // При спуске в подземный туннель (y < -0.6) плавно приглушаем наружный свет
    float underground = smoothstep(-0.6, -5.2, FragPos.y);
    float ambientScale = mix(1.0, 0.015, underground);

    vec3 color;
    float wetPatch = 0.0;
    vec3 V = normalize(camPos - FragPos);
    float fresnel = pow(1.0 - max(V.y, 0.0), 3.2);

    if (isWall) {
        // Бетонные стены подземного спуска в туннель с оранжевыми катафотами
        float tileZ = step(0.08, fract(TexCoord.y * 0.5));
        float tileY = step(0.10, fract(FragPos.y * 1.2));
        vec3 concrete = vec3(0.06, 0.065, 0.085) * (0.72 + 0.28 * tileZ * tileY);
        float reflectStrip = 1.0 - smoothstep(0.05, 0.18, abs(FragPos.y - (-5.0)));
        color = concrete * ambientScale + vec3(1.0, 0.30, 0.06) * reflectStrip * 0.18;
    } else {
        wetPatch = smoothstep(0.38, 0.72, fbmWet(TexCoord * vec2(0.85, 0.36)));
        float fineGrain = (valueNoise(TexCoord * 24.0) - 0.5) * 0.020;
        float roadBorderDist = abs(abs(x) - roadHalfWidth);
        float borderLine = 1.0 - smoothstep(0.03, 0.14, roadBorderDist);

        if (era == 0) {
            // 0: 2077 NIGHT CITY — мокрый асфальт после ливня, лужи с рябью и отражениями билбордов
            vec3 asphalt = mix(vec3(0.036, 0.040, 0.052) + vec3(fineGrain), vec3(0.016, 0.020, 0.030), wetPatch * 0.65);
            vec3 sidewalk = vec3(0.024, 0.028, 0.038) * (0.85 + 0.15 * step(0.04, fract(TexCoord.x * 0.25)) * step(0.04, fract(TexCoord.y * 0.25)));
            color = (onRoad ? asphalt : sidewalk) * ambientScale;

            if (onRoad) {
                vec2 rCell = floor(TexCoord * 3.5);
                vec2 rLocal = fract(TexCoord * 3.5) - 0.5;
                float rPhase = fract(time * 0.95 + hash21(rCell) * 6.0);
                float ripple = exp(-abs(length(rLocal) - rPhase * 0.44) * 32.0) * (1.0 - rPhase) * wetPatch;

                float b1 = 0.5 + 0.5 * sin(TexCoord.y * 0.35 + x * 0.65);
                vec3 billRefl = mix(vec3(0.75, 0.12, 0.22), vec3(0.12, 0.56, 0.75), b1);
                color += billRefl * fresnel * (0.10 + 0.42 * wetPatch) * ambientScale;
                color += vec3(0.24, 0.36, 0.48) * ripple * 0.18 * ambientScale;
            }
            float curbStripe = step(0.5, fract(TexCoord.y * 0.35));
            color += mix(vec3(0.22, 0.24, 0.28), vec3(0.72, 0.16, 0.16), curbStripe) * borderLine * ambientScale;
            float dashY = step(0.46, fract(TexCoord.y * 0.45));
            float laneDash = dashY * ((1.0 - smoothstep(0.025, 0.09, abs(x - 1.2))) + (1.0 - smoothstep(0.025, 0.09, abs(x + 1.2))));
            color += vec3(0.68, 0.56, 0.24) * laneDash * (0.78 * ambientScale + 0.02);
        } else if (era == 2) {
            // 2: NEO-TOKYO AKIRA — скоростная бетонная эстакада с температурными швами и белой японской разметкой
            float joint = 1.0 - smoothstep(0.015, 0.055, abs(fract(TexCoord.y * 0.20) - 0.5));
            vec3 deck = mix(vec3(0.046, 0.044, 0.054) + vec3(fineGrain), vec3(0.022, 0.020, 0.028), wetPatch * 0.6);
            deck *= (1.0 - joint * 0.35);
            vec3 shoulder = vec3(0.028, 0.026, 0.034) + vec3(0.04, 0.01, 0.01) * (1.0 - smoothstep(0.2, 1.5, abs(x) - roadHalfWidth));
            color = (onRoad ? deck : shoulder) * ambientScale;

            if (onRoad) {
                float b1 = 0.5 + 0.5 * sin(TexCoord.y * 0.42 + x * 0.55);
                vec3 tokyoRefl = mix(vec3(0.88, 0.10, 0.14), vec3(0.82, 0.44, 0.10), b1);
                color += tokyoRefl * fresnel * (0.12 + 0.44 * wetPatch) * ambientScale;
            }
            color += vec3(0.88, 0.16, 0.14) * borderLine * ambientScale;
            float dashY = step(0.42, fract(TexCoord.y * 0.48));
            float laneDash = dashY * ((1.0 - smoothstep(0.025, 0.085, abs(x - 1.2))) + (1.0 - smoothstep(0.025, 0.085, abs(x + 1.2))));
            color += vec3(0.82, 0.84, 0.88) * laneDash * (0.85 * ambientScale + 0.02);
        } else if (era == 3) {
            // 3: ARCTIC ICEWAY — скользкий полупрозрачный лёд с трещинами и снежными сугробами по обочинам
            float crack = smoothstep(0.92, 0.99, abs(sin(TexCoord.x * 3.8 + sin(TexCoord.y * 1.9) * 2.2) * cos(TexCoord.y * 2.7 - TexCoord.x * 1.5)));
            vec3 glacialIce = mix(vec3(0.14, 0.30, 0.46), vec3(0.32, 0.62, 0.84), wetPatch) + vec3(0.55, 0.85, 1.0) * crack * 0.45;
            vec3 deepSnow = vec3(0.68, 0.80, 0.92) + vec3(fineGrain * 1.5);
            // Надутый снег по самым краям ледяной трассы
            float driftEdge = smoothstep(3.9, 4.85, abs(x));
            vec3 roadCol = mix(glacialIce, deepSnow * 0.88, driftEdge * (0.45 + 0.55 * wetPatch));
            color = (onRoad ? roadCol : deepSnow) * ambientScale;
            if (onRoad) {
                color += vec3(0.32, 0.85, 1.0) * fresnel * (0.28 + 0.35 * wetPatch) * ambientScale;
            }
            color += vec3(0.38, 0.92, 1.0) * borderLine * 1.15 * ambientScale;
            float dashY = step(0.48, fract(TexCoord.y * 0.45));
            float laneDash = dashY * ((1.0 - smoothstep(0.03, 0.10, abs(x - 1.2))) + (1.0 - smoothstep(0.03, 0.10, abs(x + 1.2))));
            color += vec3(0.50, 0.96, 1.0) * laneDash * (0.95 * ambientScale + 0.02);
        } else if (era == 4) {
            // 4: ORBITAL STATION — титановая обшивка космического моста с заклёпками и энергоканалами
            float panelX = 1.0 - smoothstep(0.015, 0.045, abs(fract(TexCoord.x * 0.5) - 0.5));
            float panelY = 1.0 - smoothstep(0.015, 0.045, abs(fract(TexCoord.y * 0.35) - 0.5));
            float seam = clamp(panelX + panelY, 0.0, 1.0);
            vec3 titanium = vec3(0.11, 0.14, 0.20) * (1.0 - seam * 0.48) + vec3(fineGrain * 0.5);
            // За краем орбитального моста — глубокий тёмный космос со слабой силовой сеткой
            vec3 spaceVoid = vec3(0.008, 0.012, 0.024) + vec3(0.08, 0.28, 0.65) * line * 0.18;
            color = (onRoad ? titanium : spaceVoid) * ambientScale;
            if (onRoad) {
                float energyPulse = 0.65 + 0.35 * sin(TexCoord.y * 1.4 - time * 5.0);
                float conduit = (1.0 - smoothstep(0.03, 0.10, abs(x - 1.2))) + (1.0 - smoothstep(0.03, 0.10, abs(x + 1.2)));
                color += vec3(0.14, 0.78, 1.0) * conduit * energyPulse * 0.95 * ambientScale;
                color += vec3(0.24, 0.55, 0.95) * fresnel * 0.22 * ambientScale;
            }
            color += vec3(0.22, 0.88, 1.0) * borderLine * 1.45 * ambientScale;
        } else if (era == 5) {
            // 5: SAHARA OASIS — раскалённое пустынное шоссе с песчаными наносами, каменными бордюрами и барханами по краям
            float sandDrift = smoothstep(0.32, 0.74, fbmWet(TexCoord * vec2(0.55, 0.25)));
            float edgeSand = smoothstep(2.9, 4.75, abs(x)) * sandDrift;
            float ripple = 0.92 + 0.08 * sin(TexCoord.y * 4.5 + sin(TexCoord.x * 1.2) * 2.0);
            vec3 desertAsphalt = vec3(0.15, 0.12, 0.09) + vec3(fineGrain * 1.4);
            vec3 goldenSand = vec3(0.76, 0.52, 0.22) * ripple;
            vec3 roadMix = mix(desertAsphalt, goldenSand * 0.92, edgeSand * 0.85);
            color = (onRoad ? roadMix : goldenSand) * ambientScale;
            color += vec3(0.82, 0.48, 0.18) * borderLine * 0.95 * ambientScale;
            float dashY = step(0.46, fract(TexCoord.y * 0.45));
            float laneDash = dashY * ((1.0 - smoothstep(0.03, 0.095, abs(x - 1.2))) + (1.0 - smoothstep(0.03, 0.095, abs(x + 1.2))));
            color += vec3(0.95, 0.76, 0.30) * laneDash * (1.0 - edgeSand * 0.75) * ambientScale;
        } else if (era == 6) {
            // 6: NEON JUNGLE — влажная каменная дорога храмов джунглей с мхом и светящимися корнями по краям
            float stoneX = 1.0 - smoothstep(0.02, 0.06, abs(fract(TexCoord.x * 0.65) - 0.5));
            float stoneY = 1.0 - smoothstep(0.02, 0.06, abs(fract(TexCoord.y * 0.42) - 0.5));
            float mossEdge = smoothstep(3.2, 4.75, abs(x)) * wetPatch;
            vec3 wetStone = vec3(0.055, 0.085, 0.075) * (1.0 - clamp(stoneX + stoneY, 0.0, 1.0) * 0.35) + vec3(fineGrain);
            vec3 jungleFloor = mix(vec3(0.03, 0.11, 0.06), vec3(0.06, 0.26, 0.14), wetPatch);
            vec3 roadSurf = mix(wetStone, vec3(0.06, 0.28, 0.16), mossEdge * 0.75);
            color = (onRoad ? roadSurf : jungleFloor) * ambientScale;
            if (onRoad) {
                color += vec3(0.10, 0.72, 0.50) * fresnel * (0.14 + 0.32 * wetPatch) * ambientScale;
            }
            float vinePulse = 0.72 + 0.28 * sin(TexCoord.y * 1.1 - time * 2.8);
            color += vec3(0.14, 0.96, 0.62) * borderLine * 1.25 * vinePulse * ambientScale;
            float dashY = step(0.48, fract(TexCoord.y * 0.45));
            float laneDash = dashY * ((1.0 - smoothstep(0.03, 0.09, abs(x - 1.2))) + (1.0 - smoothstep(0.03, 0.09, abs(x + 1.2))));
            color += vec3(0.28, 0.96, 0.66) * laneDash * 0.85 * ambientScale;
        } else if (era == 7) {
            // 7: MARS COLONY — базальтовые плиты колонии и ржаво-красная марсианская пыль по обочинам
            float plateY = 1.0 - smoothstep(0.02, 0.06, abs(fract(TexCoord.y * 0.30) - 0.5));
            float dustPatch = smoothstep(0.36, 0.75, fbmWet(TexCoord * vec2(0.75, 0.35)));
            vec3 basaltRoad = mix(vec3(0.14, 0.075, 0.060), vec3(0.34, 0.13, 0.07), dustPatch * 0.52) * (1.0 - plateY * 0.28);
            vec3 marsRegolith = mix(vec3(0.38, 0.14, 0.07), vec3(0.56, 0.22, 0.10), dustPatch) + vec3(fineGrain * 1.5);
            color = (onRoad ? basaltRoad : marsRegolith) * ambientScale;
            float hazard = step(0.5, fract(TexCoord.y * 0.5));
            color += mix(vec3(0.15, 0.12, 0.12), vec3(0.95, 0.42, 0.10), hazard) * borderLine * 1.15 * ambientScale;
            float dashY = step(0.46, fract(TexCoord.y * 0.45));
            float laneDash = dashY * ((1.0 - smoothstep(0.03, 0.09, abs(x - 1.2))) + (1.0 - smoothstep(0.03, 0.09, abs(x + 1.2))));
            color += vec3(0.96, 0.54, 0.18) * laneDash * 0.88 * ambientScale;
        } else {
            // 1: 1985 LAS VEGAS — классический ретро-бульвар синтвейва с неоновой сеткой по краям
            vec3 asphaltCol = mix(roadColor + vec3(fineGrain), roadColor * 0.62, wetPatch * 0.6);
            vec3 groundBase = onRoad ? asphaltCol : (roadColor * 0.48);
            color = (groundBase + gridLineColor * line * (onRoad ? 0.16 : 0.62)) * ambientScale;
            if (onRoad) {
                float cityRefl = 0.5 + 0.5 * sin(TexCoord.y * 0.85 + x * 0.5);
                vec3 neonSkyRefl = mix(gridLineColor, vec3(0.12, 0.88, 1.0), cityRefl);
                color += neonSkyRefl * fresnel * (0.16 + 0.34 * wetPatch) * ambientScale;
            }
            color += gridLineColor * borderLine * (1.65 * ambientScale + 0.025);
            float dashY = step(0.45, fract(TexCoord.y * 0.5));
            float laneDash = dashY * ((1.0 - smoothstep(0.035, 0.12, abs(x - 1.2))) + (1.0 - smoothstep(0.035, 0.12, abs(x + 1.2))));
            color += vec3(0.95, 0.82, 0.22) * laneDash * (1.25 * ambientScale + 0.025);
        }

        // Отражение подсветки днища и огней транспорта на покрытии дороги
        if (onRoad && abs(FragPos.y - (carPos.y - 0.54)) < 2.6) {
            float dxCar = FragPos.x - carPos.x;
            float dzCar = FragPos.z - carPos.z;
            float underPool = exp(-(dxCar * dxCar * 0.95 + dzCar * dzCar * 0.26));
            color += headlightColor * underPool * (0.14 + 0.16 * wetPatch);

            float dzTail = FragPos.z - (carPos.z + 2.15);
            float tailPool = exp(-(dxCar * dxCar * 0.95 + dzTail * dzTail * 0.42));
            color += vec3(0.92, 0.10, 0.14) * tailPool * (0.20 + 0.22 * wetPatch);
        }
    }

    // ВАЖНО ДЛЯ ЗАЩИТЫ (критерий R4 — динамический переключаемый прожектор фар на клавишу L):
    // Две фары (левая и правая) работают как конусные источники (Spotlight):
    //   theta = dot(направление_на_пиксель, ось_фары) — косинус угла отклонения от центра луча;
    //   smooth-граница между cutoff (внутренний яркий конус) и outerCutoff (мягкий край);
    //   atten = 1 / (1 + linK*d + quadK*d^2) — физическое затухание света с расстоянием d.
    if (headlightsOn) {
        vec3 lightPosL = carPos + vec3(-0.68, 0.18, -1.9);
        vec3 lightPosR = carPos + vec3( 0.68, 0.18, -1.9);
        vec3 spotDir = normalize(vec3(0.0, -0.09, -1.0));

        vec3 toFragL = normalize(FragPos - lightPosL);
        float thetaL = dot(toFragL, spotDir);
        float distL = length(FragPos - lightPosL);

        vec3 toFragR = normalize(FragPos - lightPosR);
        float thetaR = dot(toFragR, spotDir);
        float distR = length(FragPos - lightPosR);

        float cutoff = 0.88;
        float outerCutoff = 0.69;

        float linK  = mix(0.042, 0.020, underground);
        float quadK = mix(0.0075, 0.0022, underground);
        float attenL = 1.0 / (1.0 + linK * distL + quadK * distL * distL);
        float attenR = 1.0 / (1.0 + linK * distR + quadK * distR * distR);

        float intensityL = clamp((thetaL - outerCutoff) / (cutoff - outerCutoff), 0.0, 1.0) * attenL;
        float intensityR = clamp((thetaR - outerCutoff) / (cutoff - outerCutoff), 0.0, 1.0) * attenR;

        float wetMod = mix(0.65 + 0.50 * wetPatch, 1.0, underground);
        float boostInTunnel = mix((era == 0 || era == 2) ? 0.30 : 0.22, 1.42, underground);
        color += (intensityL + intensityR) * headlightColor * boostInTunnel * wetMod;
    }

    // Дальний атмосферный туман под цвет текущего биома
    float dist = length(FragPos - camPos);
    float fogFactor = smoothstep(55.0, 175.0, dist);
    vec3 bioFog = (era == 5) ? vec3(0.42, 0.26, 0.14)
                : (era == 7) ? vec3(0.28, 0.10, 0.06)
                : (era == 3) ? vec3(0.14, 0.24, 0.38)
                : (era == 6) ? vec3(0.02, 0.08, 0.07)
                : (era == 4) ? vec3(0.01, 0.02, 0.05)
                : (era == 2) ? vec3(0.04, 0.018, 0.024)
                : (era == 0) ? vec3(0.022, 0.028, 0.044)
                : vec3(0.030, 0.045, 0.075);
    vec3 fogColor = mix(bioFog, vec3(0.005, 0.005, 0.010), underground);
    color = mix(color, fogColor, fogFactor);

    FragColor = vec4(color, 1.0);
}
