#version 330 core
// Фрагментный шейдер архитектуры города и электронных билбордов (рисуется через Instancing).
// По значению Params.a выбирается тип материала здания или экрана:
//   < -6.5 : большие анимированные электронные киберпанк-билборды (4 разных дизайна)
//   -6..-5 : камень/кора и титановые конструкции мостов
//   -4     : полупрозрачный арктический лёд
//   -3..-2 : LED-экраны и неоновые вывески
//   -1     : светящиеся огни, маячки, летающий трафик
//   >= 0   : тёмные бетонные и стеклянные небоскрёбы с потёками дождя и окнами
in vec3 FragPos;
in vec3 Normal;
in vec3 Size;
in vec3 Local;
in vec4 Params;
out vec4 FragColor;

uniform vec3 viewPos;
uniform float time;
uniform vec3 fogColor;

// Простая хеш-функция для случайных окон и киберпанк-глифов на экранах
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// Процедурная графика для больших электронных киберпанк-билбордов (без внешних PNG-текстур)
vec3 renderCyberBillboard(vec2 uv, vec2 worldUV, vec3 mainCol, int style)
{
    // Чёрная металлическая рамка и лёгкие полосы обновления экрана (scanlines)
    float edge = max(abs(uv.x - 0.5), abs(uv.y - 0.5));
    if (edge > 0.475) {
        return vec3(0.015, 0.018, 0.024) + mainCol * smoothstep(0.488, 0.475, edge) * 0.5;
    }

    // Редкие горизонтальные глитч-сдвиги строки (эффект старого уличного LED-экрана под дождём)
    float glitchBand = step(0.92, hash(vec2(floor(uv.y * 18.0), floor(time * 5.0))));
    vec2 guv = uv + vec2((hash(vec2(floor(time * 9.0), floor(uv.y * 24.0))) - 0.5) * 0.08 * glitchBand, 0.0);

    // Сетка физических светодиодов (LED-пиксели) + сканлайны
    float scan = 0.84 + 0.16 * sin(uv.y * 140.0 - time * 14.0);
    float ledMask = (0.72 + 0.28 * step(0.18, fract(worldUV.x * 2.2)) * step(0.18, fract(worldUV.y * 2.2)));
    float sweep = exp(-pow((fract(time * 0.28) - uv.y) * 9.0, 2.0)) * 0.35;

    vec3 bg = mix(vec3(0.012, 0.015, 0.024), mainCol * 0.14, uv.y);
    vec3 secCol = (mainCol.r > mainCol.b) ? vec3(0.12, 0.85, 0.96) : vec3(0.98, 0.18, 0.28);
    vec3 pic = bg;

    if (style == 0) {
        // Стиль 0: Вертикальные бегущие иероглифы/глифы Нео-Токио + яркий заголовок корпорации
        vec2 cell = floor(vec2(guv.x * 9.0, guv.y * 18.0 + floor(time * 2.2)));
        vec2 f = fract(vec2(guv.x * 9.0, guv.y * 18.0 + floor(time * 2.2)));
        float activeCol = step(0.25, hash(vec2(cell.x, 3.1)));
        // Рисуем составные штрихи, похожие на японскую катакану
        float barH = step(0.2, f.x) * step(f.x, 0.8) * step(0.42, f.y) * step(f.y, 0.58) * step(0.35, hash(cell));
        float barV = step(0.42, f.x) * step(f.x, 0.58) * step(0.18, f.y) * step(f.y, 0.85) * step(0.45, hash(cell + 1.7));
        float barT = step(0.22, f.x) * step(f.x, 0.78) * step(0.72, f.y) * step(f.y, 0.86) * step(0.50, hash(cell + 5.2));
        float glyph = clamp(barH + barV + barT, 0.0, 1.0) * activeCol;
        pic += mainCol * glyph * 1.35;
        // Верхний рекламный блок и нижний штрихкод
        if (guv.y > 0.82) pic = mix(pic, secCol * 1.15, step(0.10, guv.x) * step(guv.x, 0.90) * 0.85);
        if (guv.y < 0.14) pic += vec3(0.95, 0.82, 0.25) * step(0.45, hash(vec2(floor(guv.x * 36.0), 1.0))) * 0.9;
    } else if (style == 1) {
        // Стиль 1: Гигантская капсула "АКИРА" (бело-голубая таблетка + пульсирующий радар)
        vec2 c = (guv - vec2(0.5, 0.54)) * vec2(1.0, 1.25);
        // Поворачиваем капсулу на 35 градусов
        vec2 rc = vec2(c.x * 0.82 - c.y * 0.57, c.x * 0.57 + c.y * 0.82);
        float pillDist = length(vec2(max(abs(rc.x) - 0.16, 0.0), rc.y));
        float pill = smoothstep(0.125, 0.112, pillDist);
        vec3 pillCol = (rc.x < 0.0) ? vec3(0.95, 0.97, 1.0) : vec3(0.12, 0.78, 1.0);
        float ring = smoothstep(0.022, 0.004, abs(length(c) - (0.34 + 0.02 * sin(time * 4.0))));
        pic += mainCol * ring * 1.35;
        pic = mix(pic, pillCol * 1.25, pill);
        // Бегущая строка предупреждения снизу
        float stripes = step(0.5, fract((guv.x + time * 0.25) * 10.0));
        if (guv.y < 0.18 && guv.y > 0.06) pic = mix(pic, mainCol * (0.4 + 0.8 * stripes), 0.85);
    } else if (style == 2) {
        // Стиль 2: Большой сканирующий кибер-глаз / биометрический монитор слежки
        vec2 c = (guv - vec2(0.5, 0.52)) * vec2(1.0, 1.2);
        float r = length(c);
        float outerRing = smoothstep(0.025, 0.005, abs(r - 0.34));
        float irisRing  = smoothstep(0.020, 0.005, abs(r - 0.20));
        float ang = atan(c.y, c.x);
        float ticks = step(0.70, fract(ang * 3.8 + time * 0.45)) * smoothstep(0.32, 0.21, r) * smoothstep(0.16, 0.21, r);
        float pupil = smoothstep(0.11, 0.08, length(c - vec2(0.04 * sin(time * 1.7), 0.02 * cos(time * 2.1))));
        pic += mainCol * (outerRing * 1.4 + irisRing * 1.2 + ticks * 0.9);
        pic += secCol * pupil * 1.6;
        // Боковые полосы телеметрии
        if (abs(guv.x - 0.5) > 0.36) {
            float bars = step(0.45, fract(guv.y * 14.0 - time * 1.5));
            pic += secCol * bars * 0.75;
        }
    } else {
        // Стиль 3: Неоновый эквалайзер и цифровой горизонт мегакорпорации
        float colIdx = floor(guv.x * 16.0);
        float barH = 0.22 + 0.58 * (0.5 + 0.5 * sin(colIdx * 1.7 + time * 4.5) * cos(colIdx * 0.9 - time * 2.8));
        float inBar = step(guv.y, barH) * step(0.16, fract(guv.x * 16.0)) * step(0.20, fract(guv.y * 22.0));
        vec3 eqCol = mix(secCol, mainCol, guv.y);
        pic += eqCol * inBar * 1.25;
        // Крупный логотип-ромб в верхней части экрана
        vec2 tc = (guv - vec2(0.5, 0.80)) * vec2(1.4, 2.2);
        float diamond = smoothstep(0.24, 0.20, abs(tc.x) + abs(tc.y)) - smoothstep(0.14, 0.10, abs(tc.x) + abs(tc.y));
        pic += mainCol * clamp(diamond, 0.0, 1.0) * 1.5;
    }

    // Добавляем свечение обновления, LED-сетку и лёгкий глитч-блик
    pic = (pic + mainCol * sweep) * scan * ledMask;
    pic += secCol * glitchBand * 0.28;
    return pic;
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 base = Params.rgb;
    vec3 col = vec3(0.012, 0.014, 0.022);

    float uCoord = abs(N.x) > 0.5 ? Local.z : Local.x;
    float spanW  = abs(N.x) > 0.5 ? Size.z  : Size.x;
    float u01    = clamp(uCoord / max(spanW, 0.01), 0.0, 1.0);
    float v01    = clamp(Local.y / max(Size.y, 0.01), 0.0, 1.0);

    if (Params.a < -6.5) {
        // Большие электронные киберпанк-билборды (Params.a = -7, -8, -9, -10 задают разные сюжеты)
        int style = int(floor(abs(Params.a) - 6.5)) % 4;
        col = renderCyberBillboard(vec2(u01, v01), vec2(uCoord, Local.y), base, style);
    } else if (Params.a < -5.5) {
        // Камень пирамид, кора и листва джунглей, марсианский грунт
        float grain = 0.86 + 0.14 * sin(FragPos.y * 1.4 + FragPos.z * 0.5) * cos(FragPos.x * 0.9);
        float stoneCourse = 0.92 + 0.08 * smoothstep(0.05, 0.15, fract(Local.y * 0.85));
        vec3 sunDir = normalize(vec3(-0.30, 0.72, -0.62));
        float diff = 0.34 + 0.66 * max(dot(N, sunDir), 0.0);
        col = base * grain * stoneCourse * diff;
        if (abs(N.y) > 0.5) col *= 1.12;
    } else if (Params.a < -4.5) {
        // Тёмные металлические и бетонные конструкции мостов, стен и опор
        float panelX = smoothstep(0.04, 0.09, fract(uCoord * 0.45)) * smoothstep(0.04, 0.09, 1.0 - fract(uCoord * 0.45));
        float panelY = smoothstep(0.04, 0.09, fract(Local.y * 0.45)) * smoothstep(0.04, 0.09, 1.0 - fract(Local.y * 0.45));
        float seam = mix(0.62, 1.0, panelX * panelY);
        vec3 lightDir = normalize(vec3(-0.3, 0.7, 0.6));
        float diff = 0.25 + 0.65 * max(dot(N, lightDir), 0.0);
        float spec = pow(max(dot(reflect(-lightDir, N), V), 0.0), 28.0);
        col = base * seam * diff + vec3(0.32, 0.42, 0.54) * spec * 0.24;
    } else if (Params.a < -3.5) {
        // Арктический ледник с бликом Френеля
        float fresnel = pow(1.0 - max(dot(N, V), 0.0), 2.5);
        float facets = 0.82 + 0.18 * sin(FragPos.y * 1.3 + FragPos.z * 0.4) * cos(FragPos.x * 0.7);
        vec3 sunDir = normalize(vec3(0.25, 0.65, -0.70));
        float diff = 0.40 + 0.60 * max(dot(N, sunDir), 0.0);
        float spec = pow(max(dot(reflect(-sunDir, N), V), 0.0), 42.0);
        col = base * facets * diff + vec3(0.55, 0.92, 1.0) * (fresnel * 0.55 + spec * 0.48);
    } else if (Params.a < -2.5) {
        // Компактные LED-экраны / солнечные панели
        float pulse = 0.58 + 0.42 * sin(u01 * 12.0 - time * 2.2 + v01 * 5.0);
        float ledDot = step(0.22, fract(uCoord * 1.4)) * step(0.22, fract(Local.y * 1.4));
        col = mix(vec3(0.02, 0.03, 0.07), base * 0.82, pulse * (0.55 + 0.45 * ledDot));
        float frame = smoothstep(0.46, 0.495, max(abs(u01 - 0.5), abs(v01 - 0.5)));
        col = mix(col, base * 1.05, frame);
    } else if (Params.a < -1.5) {
        // Неоновая вывеска с бегущей рамкой
        float border = max(abs(u01 - 0.5), abs(v01 - 0.5));
        float bulb = step(0.5, fract((u01 + v01) * 14.0 - time * 2.5));
        float frameGlow = smoothstep(0.42, 0.485, border);
        float bar1 = smoothstep(0.08, 0.02, abs(v01 - 0.36));
        float bar2 = smoothstep(0.08, 0.02, abs(v01 - 0.64));
        col = vec3(0.020, 0.015, 0.035) + base * (bar1 + bar2) * 0.65;
        col = mix(col, vec3(0.95, 0.80, 0.28) * (0.65 + 0.35 * bulb), frameGlow);
    } else if (Params.a < 0.0) {
        // Светящиеся элементы: маячки на крышах, поезда-пули, летающий трафик
        float pulse = 0.85 + 0.15 * sin(time * 4.0 + FragPos.z * 0.12);
        col = base * 1.05 * pulse;
    } else if (abs(N.y) > 0.5) {
        float roofGrid = step(0.92, fract(Local.x * 0.5)) + step(0.92, fract(Local.z * 0.5));
        col = vec3(0.012, 0.014, 0.022) + base * clamp(roofGrid, 0.0, 1.0) * 0.05;
    } else {
        // Мрачный дождливый фасад мегаполиса: тёмный бетон, редкие тёплые окна и сырое стекло
        vec2 g = vec2(uCoord, Local.y) / vec2(1.20, 1.65);
        vec2 id = floor(g);
        vec2 f = fract(g);
        float win = step(0.20, f.x) * step(f.x, 0.80) * step(0.24, f.y) * step(f.y, 0.80);
        float mullion = 1.0 - win;

        float r = hash(id + Params.a * 13.7);
        float floorHash = hash(vec2(id.y, Params.a * 7.1));
        float on = step(0.62, r) * step(0.20, floorHash); // меньше светлых окон -> мрачнее атмосфера

        vec3 warmGold  = vec3(0.90, 0.68, 0.34);
        vec3 warmAmber = vec3(0.92, 0.46, 0.18);
        vec3 coolGlass = mix(vec3(0.32, 0.62, 0.78), base, 0.38);
        vec3 wc = mix(warmGold, warmAmber, hash(id + 2.1));
        wc = mix(wc, coolGlass, step(0.74, hash(id + 5.3)));

        float ceilingGlow = 0.68 + 0.35 * smoothstep(0.50, 0.80, f.y);
        float diff = 0.55 + 0.45 * abs(N.x);

        float glassFresnel = pow(1.0 - max(dot(N, V), 0.0), 3.2);
        float rainStreak = 0.90 + 0.10 * sin(uCoord * 10.0 + sin(Local.y * 0.45 + time * 1.6));
        vec3 stoneFrame = mix(vec3(0.009, 0.011, 0.018), vec3(0.022, 0.025, 0.036), v01) * diff;
        vec3 unlitGlass = (mix(vec3(0.012, 0.016, 0.026), vec3(0.024, 0.032, 0.048) + base * 0.025, v01) + vec3(0.04, 0.07, 0.10) * glassFresnel) * rainStreak;

        col = stoneFrame * mullion + unlitGlass * win;
        col += win * on * wc * ceilingGlow * 0.40;

        float lobbyZone = (1.0 - step(3.1, Local.y)) * step(0.25, Local.y);
        float lobbyArch = step(0.22, fract(uCoord * 0.35)) * step(fract(uCoord * 0.35), 0.78);
        col += lobbyZone * lobbyArch * mix(warmAmber, base, 0.35) * 0.22;

        float midBand = smoothstep(0.018, 0.0, abs(v01 - 0.52));
        float cornerTrim = smoothstep(0.482, 0.496, abs(u01 - 0.5));
        float roofTrim   = smoothstep(0.980, 0.996, v01);
        col += base * (cornerTrim * 0.24 + roofTrim * 0.36 + midBand * 0.12);
    }

    float d = length(FragPos - viewPos);
    float fog = smoothstep(55.0, 185.0, d);
    col = mix(col, fogColor, fog);
    FragColor = vec4(col, 1.0);
}
