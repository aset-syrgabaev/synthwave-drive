#version 330 core
// Фрагментный шейдер для дальних гор и песчаных дюн.
// Разные биомы (era) получают свою геологию: песчаные барханы Сахары с ветровой рябью,
// слоистые скалы Марса, заснеженные альпийские ледники Арктики и тропические склоны джунглей.
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 viewPos;   // позиция камеры
uniform vec3 baseColor; // базовый цвет подножия
uniform vec3 peakColor; // цвет вершин
uniform vec3 fogColor;  // цвет атмосферной дымки
uniform int era;        // номер текущего биома (0..7)

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = (era == 5) ? normalize(vec3(0.45, 0.62, -0.64))
                  : (era == 7) ? normalize(vec3(-0.42, 0.54, -0.72))
                  : normalize(vec3(-0.35, 0.72, -0.60));
    float diff = max(dot(norm, lightDir), 0.0);
    float rim = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.0);

    vec3 color;
    if (era == 5) {
        // Сахара: реалистичные золотистые песчаные дюны с ветровой рябью и тёплыми тенями
        float ripple = 0.94 + 0.06 * sin(FragPos.z * 2.4 + FragPos.x * 1.1 + sin(FragPos.x * 0.25) * 3.0);
        float strata = 0.90 + 0.10 * sin(FragPos.y * 1.35 + FragPos.x * 0.08);
        vec3 shadowSand = vec3(0.32, 0.16, 0.06);
        vec3 warmSand   = vec3(0.86, 0.58, 0.24);
        vec3 sunlitGold = vec3(1.00, 0.80, 0.44);
        float h = smoothstep(2.0, 26.0, FragPos.y);
        vec3 sandCol = mix(warmSand, sunlitGold, h * 0.65) * ripple * strata;
        // Песчаные кристаллики на гребне слегка поблёскивают на солнце
        vec3 H = normalize(lightDir + viewDir);
        float sandSpec = pow(max(dot(norm, H), 0.0), 18.0) * 0.24;
        color = mix(shadowSand, sandCol, 0.28 + 0.72 * diff) + vec3(1.0, 0.82, 0.48) * (sandSpec + rim * 0.22);
    } else if (era == 7) {
        // Марс: слоистые террасированные каньоны из красного песчаника и базальта
        float strata = smoothstep(-0.2, 0.5, sin(FragPos.y * 1.85 + sin(FragPos.z * 0.08) * 1.5));
        vec3 darkBasalt = vec3(0.18, 0.06, 0.04);
        vec3 rustRock   = vec3(0.58, 0.21, 0.10);
        vec3 ochreDust  = vec3(0.76, 0.34, 0.16);
        vec3 rockCol = mix(darkBasalt, mix(rustRock, ochreDust, strata), smoothstep(1.0, 24.0, FragPos.y) * 0.85 + 0.15);
        color = rockCol * (0.34 + 0.82 * diff) + vec3(0.90, 0.40, 0.18) * rim * 0.22;
    } else if (era == 3) {
        // Арктика: ледяные скалы с густыми снежными шапками на пологих склонах
        float snowMask = smoothstep(0.54, 0.82, norm.y) * smoothstep(3.0, 14.0, FragPos.y);
        vec3 glacialBlue = vec3(0.14, 0.28, 0.46);
        vec3 pureSnow    = vec3(0.84, 0.93, 1.00);
        vec3 surf = mix(glacialBlue, pureSnow, snowMask);
        color = surf * (0.42 + 0.78 * diff) + vec3(0.45, 0.86, 1.0) * rim * 0.28;
    } else if (era == 6) {
        // Джунгли: изумрудные вулканические пики с влажной тропической листвой и туманом
        float canopy = smoothstep(0.45, 0.80, norm.y);
        vec3 wetCliff = vec3(0.05, 0.12, 0.10);
        vec3 lushMoss = vec3(0.08, 0.38, 0.22);
        vec3 surf = mix(wetCliff, lushMoss, canopy) * (0.86 + 0.14 * sin(FragPos.x * 0.35 + FragPos.z * 0.40));
        color = surf * (0.45 + 0.75 * diff) + vec3(0.14, 0.78, 0.56) * rim * 0.24;
    } else {
        // Ночной город, Лас-Вегас и Нео-Токио: тёмные горные хребты с атмосферным заревом города
        float heightFactor = smoothstep(4.0, 32.0, FragPos.y);
        float slopeSnow = smoothstep(0.62, 0.88, norm.y) * smoothstep(14.0, 28.0, FragPos.y);
        float rockGrain = 0.88 + 0.12 * sin(FragPos.x * 0.45 + FragPos.z * 0.35) * cos(FragPos.y * 0.75);
        vec3 surfaceColor = mix(baseColor, peakColor, clamp(heightFactor * 0.65 + slopeSnow * 0.55, 0.0, 1.0)) * rockGrain;
        color = baseColor * 0.55 + diff * surfaceColor * 0.85 + peakColor * rim * 0.24;
    }

    // Плавное растворение гор в атмосферном тумане выбранной локации
    float dist = length(FragPos - viewPos);
    float fog = smoothstep(75.0, 225.0, dist);
    vec3 effFog = (era == 5) ? vec3(0.48, 0.30, 0.16)
                : (era == 7) ? vec3(0.32, 0.12, 0.07)
                : (era == 3) ? vec3(0.18, 0.30, 0.44)
                : fogColor;
    color = mix(color, effFog, fog);

    FragColor = vec4(color, 1.0);
}
