#version 330 core
// Шейдер рисует процедурный фон неба на весь экран (полноэкранный прямоугольник).
// Для каждой из 8 локаций (uniform int era) здесь своя атмосфера: тучи, смог, солнце или космос.
out vec4 FragColor;
in vec2 UV; // координаты пикселя на экране от (0,0) внизу слева до (1,1) наверху справа

uniform float time;            // текущее время сцены для анимации туч, прожекторов и звёзд
uniform vec2 resolution;       // размер окна, нужен чтобы круглые солнце/луна не растягивались
uniform int era;               // номер текущей локации (0..7)
uniform vec3 skyTopColor;      // цвет верха неба
uniform vec3 skyMidColor;      // средний цвет неба
uniform vec3 skyHorizonColor;  // цвет у линии горизонта

// Простая хеш-функция: выдаёт псевдослучайное число от 0 до 1 для координат сетки (для звёзд)
float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Сглаженный шум для объёмных грозовых облаков и смога
float cloudNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main()
{
    vec2 p = UV;
    vec2 aspect = vec2(resolution.x / resolution.y, 1.0);

    // Базовый вертикальный градиент неба: горизонт -> середина -> зенит
    vec3 sky = mix(skyHorizonColor, skyMidColor, smoothstep(0.10, 0.52, p.y));
    sky = mix(sky, skyTopColor, smoothstep(0.52, 1.0, p.y));

    // Звёзды рисуем только в чистых мирах (в пасмурном Night City и Neo-Tokyo небо закрыто тяжёлым смогом)
    if (era != 0 && era != 2) {
        vec2 starGrid = floor(p * vec2(260.0, 150.0));
        float st = hash21(starGrid);
        float starThresh = (era == 4) ? 0.972 : ((era == 3 || era == 6) ? 0.984 : 0.990);
        if (st > starThresh && p.y > (era == 4 ? 0.08 : 0.36)) {
            float twinkle = 0.55 + 0.45 * sin(time * 3.0 + st * 90.0);
            vec3 starCol = mix(vec3(0.75, 0.85, 1.0), vec3(1.0, 0.92, 0.78), fract(st * 17.0));
            sky += starCol * twinkle * ((era == 4) ? 1.0 : 0.72);
        }
    }

    if (era == 0) {
        // 0: 2077 NIGHT CITY — тяжёлые грозовые тучи, мрачный смог мегаполиса, красное зарево и лучи прожекторов
        float c1 = cloudNoise(vec2(p.x * 5.0 + time * 0.025, p.y * 8.0));
        float c2 = cloudNoise(vec2(p.x * 11.0 - time * 0.018, p.y * 15.0 + 3.1));
        float clouds = c1 * 0.65 + c2 * 0.35;
        sky += vec3(0.025, 0.035, 0.055) * clouds * smoothstep(0.12, 0.85, p.y);

        // Давящий промышленный смог у горизонта (холодный серо-синий + отблески красных и янтарных билбордов)
        float smogHaze = exp(-max(p.y - 0.10, 0.0) * 6.8);
        vec3 smogCol = mix(vec3(0.09, 0.025, 0.035), vec3(0.04, 0.07, 0.09), 0.5 + 0.5 * sin(p.x * 4.5));
        sky += smogCol * smogHaze * 0.85;

        // Два медленных мрачных луча полицейских прожекторов сквозь дождь и облака
        float sw1 = 0.28 + 0.16 * sin(time * 0.42);
        float sw2 = 0.72 + 0.16 * cos(time * 0.36);
        float b1 = exp(-pow((p.x - sw1 - (p.y - 0.12) * 0.48) * 24.0, 2.0)) * smoothstep(0.10, 0.38, p.y) * (1.0 - smoothstep(0.65, 0.96, p.y));
        float b2 = exp(-pow((p.x - sw2 + (p.y - 0.12) * 0.44) * 24.0, 2.0)) * smoothstep(0.10, 0.38, p.y) * (1.0 - smoothstep(0.65, 0.96, p.y));
        sky += vec3(0.28, 0.42, 0.52) * (b1 + b2) * 0.26;

        // Редкая глухая вспышка далёкой грозы внутри туч
        float flash = pow(max(0.0, sin(time * 0.9) * sin(time * 3.7)), 32.0);
        sky += vec3(0.18, 0.32, 0.48) * flash * clouds * smoothstep(0.25, 0.78, p.y) * 0.55;
    } else if (era == 1) {
        // ВАЖНО ДЛЯ ЗАЩИТЫ (ядро Темы #13 — полосатое 80s неоновое солнце в 1985 LAS VEGAS):
        // 1. Умножаем смещение (p - sunPos) на aspect (ширина/высота), чтобы круг солнца не стал овалом на 16:9!
        // 2. Градиент mix(розовый, золотой, sunY) красит диск сверху вниз.
        // 3. В нижней части солнца функция mod(p.y + time*0.008, 0.036) вырезает бегущие горизонтальные полосы;
        //    чем ниже по диску (normY больше), тем шире вырез — классический эффект Synthwave 1980-х!
        vec2 sunPos = vec2(0.5, 0.68);
        float dist = length((p - sunPos) * aspect);
        float sunRadius = 0.22;
        if (dist < sunRadius) {
            float sunY = (p.y - (sunPos.y - sunRadius)) / (2.0 * sunRadius);
            vec3 sunColor = mix(vec3(0.95, 0.08, 0.46), vec3(1.0, 0.84, 0.18), clamp(sunY, 0.0, 1.0));
            float cut = 1.0;
            if (p.y < sunPos.y + sunRadius * 0.35) {
                float normY = clamp((sunPos.y + sunRadius * 0.35 - p.y) / (sunRadius * 1.35), 0.0, 1.0);
                float stripePhase = mod(p.y + time * 0.008, 0.036) / 0.036;
                if (stripePhase < mix(0.22, 0.58, normY)) cut = 0.0;
            }
            sky = mix(sky, sunColor, cut);
        }
        // Экспоненциальный ореол вокруг солнца exp(-dist * 4.8) — мягкое свечение атмосферы
        sky += vec3(0.68, 0.12, 0.36) * exp(-dist * 4.8) * 0.36;
    } else if (era == 2) {
        // 2: NEO-TOKYO AKIRA — мрачное багровое небо Нео-Токио 2019, густые тучи и скрещенные прожекторы
        float c1 = cloudNoise(vec2(p.x * 6.0 + time * 0.03, p.y * 10.0));
        float c2 = cloudNoise(vec2(p.x * 13.0 - time * 0.02, p.y * 16.0));
        sky += vec3(0.06, 0.025, 0.045) * (c1 * 0.6 + c2 * 0.4) * smoothstep(0.12, 0.80, p.y) * 0.9;
        sky += vec3(0.24, 0.055, 0.065) * exp(-max(p.y - 0.10, 0.0) * 5.8) * 0.75;

        // Лучи прожекторов ПВО над ночным городом
        float sw1 = 0.34 + 0.18 * sin(time * 0.55);
        float sw2 = 0.66 + 0.18 * cos(time * 0.48);
        float beam1 = exp(-pow((p.x - sw1 - (p.y - 0.15) * 0.42) * 26.0, 2.0)) * smoothstep(0.12, 0.42, p.y) * (1.0 - smoothstep(0.65, 0.98, p.y));
        float beam2 = exp(-pow((p.x - sw2 + (p.y - 0.15) * 0.38) * 26.0, 2.0)) * smoothstep(0.12, 0.42, p.y) * (1.0 - smoothstep(0.65, 0.98, p.y));
        sky += vec3(0.42, 0.58, 0.74) * (beam1 + beam2) * 0.30;

        // Тусклая кровавая луна за дымкой смога
        vec2 moonPos = vec2(0.5, 0.54);
        float dist = length((p - moonPos) * aspect);
        if (dist < 0.085) sky = mix(sky, vec3(0.72, 0.24, 0.20), smoothstep(0.085, 0.062, dist) * 0.65);
        sky += vec3(0.45, 0.10, 0.10) * exp(-dist * 6.5) * 0.28;
    } else if (era == 3) {
        // 3: ARCTIC AURORA — северное сияние из двух волнистых лент
        float band1 = sin(p.x * 6.5 + time * 0.45 + sin(p.y * 5.0) * 1.4);
        float band2 = sin(p.x * 11.0 - time * 0.32 + cos(p.x * 3.0) * 2.0);
        float curtain = exp(-pow((p.y - 0.58 - 0.09 * band1) * 7.5, 2.0));
        float rays = 0.55 + 0.45 * sin(p.x * 38.0 + band2 * 3.0 + time * 0.9);
        vec3 auroraCol = mix(vec3(0.08, 0.92, 0.58), vec3(0.45, 0.22, 0.95), smoothstep(0.46, 0.76, p.y));
        sky += auroraCol * curtain * rays * 0.75;
        float curtain2 = exp(-pow((p.y - 0.44 + 0.06 * band2) * 9.0, 2.0));
        sky += vec3(0.10, 0.75, 0.92) * curtain2 * (0.6 + 0.4 * rays) * 0.48;
    } else if (era == 4) {
        // 4: ORBITAL STATION — туманность и планета с кольцами
        float neb = 0.5 + 0.5 * sin(p.x * 5.5 + p.y * 4.0) * cos(p.x * 9.0 - p.y * 7.0);
        sky += mix(vec3(0.04, 0.14, 0.34), vec3(0.22, 0.06, 0.32), neb) * smoothstep(0.12, 0.85, p.y) * 0.35;
        vec2 planetPos = vec2(0.5, 0.52);
        vec2 dp = (p - planetPos) * aspect;
        float dist = length(dp);
        float pR = 0.145;
        vec2 rp = vec2(dp.x * 0.92 + dp.y * 0.38, -dp.x * 0.38 + dp.y * 0.92);
        float ringD = length(vec2(rp.x * 0.52, rp.y * 2.75));
        float ringMask = smoothstep(0.135, 0.148, ringD) * (1.0 - smoothstep(0.215, 0.232, ringD));
        float ringBands = 0.65 + 0.35 * sin(ringD * 180.0);
        bool inFront = (rp.y < 0.0);
        if (!inFront && ringMask > 0.01 && dist >= pR) sky = mix(sky, vec3(0.45, 0.68, 0.92) * ringBands, ringMask * 0.72);
        if (dist < pR) {
            float shade = clamp(dot(normalize(vec3(dp / pR, sqrt(max(0.0, 1.0 - dot(dp / pR, dp / pR))))), normalize(vec3(-0.55, 0.45, 0.65))), 0.0, 1.0);
            float bands = 0.75 + 0.25 * sin(dp.y * 55.0 + sin(dp.x * 18.0) * 1.2);
            vec3 planetCol = mix(vec3(0.08, 0.16, 0.36), vec3(0.32, 0.68, 0.95), bands) * (0.14 + 0.86 * shade);
            planetCol += vec3(0.28, 0.75, 1.0) * pow(clamp(dist / pR, 0.0, 1.0), 4.0) * 0.55;
            sky = mix(sky, planetCol, smoothstep(pR, pR - 0.005, dist));
        }
        if (inFront && ringMask > 0.01) sky = mix(sky, vec3(0.55, 0.78, 1.0) * ringBands, ringMask * 0.80);
        sky += vec3(0.14, 0.38, 0.78) * exp(-dist * 5.0) * 0.30;
    } else if (era == 5) {
        // 5: SAHARA PYRAMIDS — тёплый закат над пустыней
        float haze = exp(-max(p.y - 0.10, 0.0) * 5.2);
        sky += vec3(0.42, 0.22, 0.06) * haze * 0.55;
        vec2 sunPos = vec2(0.5, 0.34);
        float dist = length((p - sunPos) * aspect);
        float sunR = 0.17;
        if (dist < sunR) {
            float sy = (p.y - (sunPos.y - sunR)) / (2.0 * sunR);
            vec3 sCol = mix(vec3(0.95, 0.42, 0.10), vec3(1.0, 0.92, 0.58), sy);
            sky = mix(sky, sCol, smoothstep(sunR, sunR - 0.012, dist));
        }
        sky += vec3(0.85, 0.48, 0.14) * exp(-dist * 4.8) * 0.45;
    } else if (era == 6) {
        // 6: NEON JUNGLE — ночные джунгли с туманом и серпом луны
        float mist = exp(-max(p.y - 0.12, 0.0) * 5.5);
        sky += vec3(0.06, 0.28, 0.18) * mist * 0.65;
        float glowWisp = 0.5 + 0.5 * sin(p.x * 8.0 + time * 0.25) * cos(p.y * 10.0 - time * 0.12);
        sky += vec3(0.05, 0.24, 0.20) * glowWisp * smoothstep(0.15, 0.70, p.y) * 0.38;
        vec2 mPos = vec2(0.56, 0.58);
        float d1 = length((p - mPos) * aspect);
        float d2 = length((p - (mPos + vec2(0.032, 0.018))) * aspect);
        float cres = smoothstep(0.095, 0.088, d1) * (1.0 - smoothstep(0.085, 0.078, d2));
        sky = mix(sky, vec3(0.75, 0.98, 0.88), cres * 0.88);
        sky += vec3(0.10, 0.45, 0.32) * exp(-d1 * 6.5) * 0.32;
    } else {
        // 7: MARS COLONY — пыльная буря и два спутника Марса (Фобос и Деймос)
        float dust = exp(-max(p.y - 0.10, 0.0) * 4.8);
        sky += vec3(0.45, 0.16, 0.08) * dust * 0.58;
        vec2 sunPos = vec2(0.50, 0.28);
        float dSun = length((p - sunPos) * aspect);
        sky += vec3(0.45, 0.65, 0.82) * exp(-dSun * 6.5) * 0.45;
        if (dSun < 0.055) sky = mix(sky, vec3(0.88, 0.94, 1.0), smoothstep(0.055, 0.042, dSun));
        float dPhobos = length((p - vec2(0.36, 0.58)) * aspect);
        if (dPhobos < 0.042) sky = mix(sky, vec3(0.68, 0.54, 0.46), smoothstep(0.042, 0.035, dPhobos) * 0.9);
        float dDeimos = length((p - vec2(0.68, 0.64)) * aspect);
        if (dDeimos < 0.022) sky = mix(sky, vec3(0.78, 0.66, 0.58), smoothstep(0.022, 0.017, dDeimos) * 0.9);
    }

    FragColor = vec4(sky, 1.0);
}
