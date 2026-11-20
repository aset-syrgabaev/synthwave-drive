#version 330 core
// Основной PBR-шейдер для 3D-моделей: DeLorean, мотоцикла Акиры, поездов, эстакад ЛРТ, монет и эффектов.
// Использует физически корректную модель освещения Cook-Torrance (GGX + Geometry Smith + Fresnel)
// плюс спецрежимы для стекла, световых шлейфов, щита и 8 уникальных скинов (включая невидимый).
in vec3 FragPos;   // мировые координаты текущей точки поверхности
in vec3 Normal;    // нормаль поверхности в мировых координатах
in vec3 LocalPos;  // локальные координаты внутри модели (нужны для узоров, карбона и шлейфов)
out vec4 FragColor;

uniform vec3 viewPos;        // позиция камеры
uniform vec3 objectColor;    // основной цвет материала (albedo)
uniform vec3 emissiveColor;  // собственное свечение (фары, неон, экраны)
uniform float shininess;     // гладкость материала (чем выше, тем острее блик)
uniform float reflectivity;  // отражающая способность / металличность (0..1)
uniform float shieldAlpha;   // управляющий параметр прозрачных режимов (0 = обычный непрозрачный объект)
uniform int skinMode;        // номер скина игрока (0..7), для окружения всегда 0
uniform float time;          // время для анимации эффектов

uniform vec3 lightPos1;      // основной источник света (небо/горизонт)
uniform vec3 lightColor1;
uniform vec3 lightPos2;      // контровой/городской источник света
uniform vec3 lightColor2;

uniform vec3 carPos;         // позиция машины (для расчёта луча фар на препятствиях и стенах)
uniform bool headlightsOn;   // включены ли фары

const float PI = 3.14159265359;

// Распределение микрограней GGX: определяет форму и резкость зеркального блика
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 0.0001);
}

// Геометрическое самозатенение микрограней (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

// Приближение Френеля: под скользящим углом любая поверхность отражает свет сильнее
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Процедурная карта отражений мрачного ночного киберпанк-города (отражается на краске, хроме и стекле)
vec3 sampleMoodyCyberEnv(vec3 R, float underground)
{
    vec3 dir = normalize(R);
    float y = dir.y;
    vec3 skyZenith = vec3(0.015, 0.022, 0.042);
    vec3 skyMid    = vec3(0.065, 0.088, 0.135);
    vec3 skyHoriz  = vec3(0.22, 0.18, 0.24);
    vec3 groundCol = vec3(0.015, 0.018, 0.026);

    vec3 env;
    if (y >= 0.0) {
        env = mix(skyHoriz, skyMid, smoothstep(0.0, 0.22, y));
        env = mix(env, skyZenith, smoothstep(0.22, 0.82, y));
        // Верхняя студийная полоса уличного света и красно-бирюзовые отблески билбордов
        float studioBand = exp(-pow(dir.x * 2.2, 2.0)) * smoothstep(0.10, 0.55, y) * (1.0 - smoothstep(0.55, 0.95, y));
        env += vec3(0.24, 0.42, 0.56) * studioBand * 0.65;
        float sideBill = exp(-pow((abs(dir.x) - 0.72) * 3.2, 2.0)) * smoothstep(0.02, 0.42, y);
        env += mix(vec3(0.52, 0.08, 0.12), vec3(0.10, 0.48, 0.62), step(0.0, dir.x)) * sideBill * 0.52;
    } else {
        env = mix(vec3(0.06, 0.07, 0.10), groundCol, smoothstep(0.0, 0.40, -y));
    }
    return env * mix(1.0, 0.05, underground);
}

// Расчёт прямого освещения от одного точечного источника по модели Cook-Torrance
vec3 calcPBRLight(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    vec3 specular = numerator / denom;

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // Режим 1: Объёмные световые шлейфы фар и турбин (shieldAlpha < -1.5)
    // Использует мягкое гауссово затухание по ширине и длине, чтобы не было жёстких граней полигонов
    if (shieldAlpha < -1.5) {
        float intensity = clamp((-shieldAlpha - 2.0), 0.05, 2.0);
        float along  = clamp(LocalPos.z, 0.0, 1.0);
        float localW = max(0.08, 1.0 - along * 0.70);
        float across = clamp(length(LocalPos.xy) / localW, 0.0, 1.0);
        float headSoft = smoothstep(0.0, 0.04, along);
        float lenFade  = pow(1.0 - along, 1.85) * headSoft;
        float edgeMask = smoothstep(1.0, 0.06, across);
        float edgeFade = exp(-across * across * 4.2) * edgeMask;
        float shimmer  = 0.92 + 0.08 * sin(along * 28.0 - time * 46.0);
        float hotCore  = exp(-across * across * 14.0) * pow(1.0 - along, 2.2) * edgeMask;
        vec3 col = mix(objectColor * 1.55, vec3(1.0, 0.94, 0.86) * 2.4, hotCore);
        float alpha = clamp(lenFade * edgeFade * shimmer * intensity, 0.0, 0.92);
        FragColor = vec4(col * alpha, alpha);
        return;
    }

    // Режим 2: Невидимый скин — оптический камуфляж (shieldAlpha в диапазоне -1.45 .. -0.80)
    // Центр корпуса почти полностью прозрачен, видны только преломлённые контуры и лёгкая рябь камуфляжа
    if (shieldAlpha < -0.80) {
        float ndotv = abs(dot(N, V));
        float rim = pow(1.0 - ndotv, 3.6);
        float ripple = 0.5 + 0.5 * sin(LocalPos.z * 26.0 - time * 7.5 + LocalPos.y * 16.0);
        float scan = smoothstep(0.92, 0.99, sin(LocalPos.y * 32.0 + time * 4.0));
        vec3 envRefl = sampleMoodyCyberEnv(reflect(-V, N), 0.0);
        vec3 cloakCol = mix(envRefl * 1.4, vec3(0.32, 0.85, 1.0), rim * 0.72) + vec3(0.45, 0.95, 1.0) * scan * 0.35;
        float alpha = clamp(0.035 + rim * 0.52 + ripple * rim * 0.22 + scan * 0.08, 0.03, 0.62);
        FragColor = vec4(cloakCol, alpha);
        return;
    }

    // Режим 3: Прозрачное гнутое стекло ветровика мотоцикла (shieldAlpha в диапазоне -0.80 .. -0.01)
    if (shieldAlpha < -0.01) {
        float glassAmt = clamp(-shieldAlpha, 0.05, 0.78);
        float undergroundG = smoothstep(-0.8, -5.2, FragPos.y);
        float ndotv = abs(dot(N, V));
        float fresnel = pow(1.0 - ndotv, 3.2);
        vec3 envRefl = sampleMoodyCyberEnv(reflect(-V, N), undergroundG);
        vec3 L1g = normalize(lightPos1 - FragPos);
        vec3 H1g = normalize(V + L1g);
        float specG = DistributionGGX(N, H1g, 0.05) * 0.45 * max(abs(dot(N, L1g)), 0.0);
        vec3 glassCol = objectColor * 0.20 + envRefl * (0.48 + 1.25 * fresnel) + vec3(0.95, 0.98, 1.0) * specG;
        float alpha = clamp(0.10 + fresnel * 0.60 * glassAmt + specG * 0.35, 0.08, 0.76);
        FragColor = vec4(glassCol, alpha);
        return;
    }

    // Режим 4: Энергетический щит, голографический скин, Phase Shift, Sandevistan, EMP (shieldAlpha > 0.01)
    // Цвет щита и сетки полностью следует за objectColor (меняется вместе с выбранным скином)
    if (shieldAlpha > 0.01) {
        float ndotv = abs(dot(N, V));
        float fresnel = pow(1.0 - ndotv, 2.8);
        float wave = 0.5 + 0.5 * sin(LocalPos.y * 22.0 - time * 5.6 + LocalPos.z * 10.0);
        float latLine = smoothstep(0.94, 0.995, abs(sin(LocalPos.y * 24.0 - time * 3.0)));
        float lonLine = smoothstep(0.94, 0.995, abs(sin(LocalPos.z * 16.0 + atan(LocalPos.x, LocalPos.z) * 8.0 + time * 1.6)));
        float grid = clamp(latLine + lonLine, 0.0, 1.0);

        vec3 coreCol = mix(objectColor, objectColor * 1.45 + vec3(0.20), fresnel * 0.65);
        vec3 finalShield = coreCol * (0.45 + 1.20 * fresnel + 0.65 * grid * wave);
        float alpha = clamp((0.04 + fresnel * 0.46 + grid * 0.22 * wave) * shieldAlpha, 0.0, 0.72);
        FragColor = vec4(finalShield, alpha);
        return;
    }

    // ---- Обычный непрозрачный PBR-расчёт ----
    float underground = smoothstep(-0.8, -5.2, FragPos.y);
    float lightScale = mix(1.0, 0.015, underground);

    float metallic  = clamp((reflectivity - 0.15) * 1.12, 0.0, 0.94);
    float roughness = clamp(1.0 - (shininess / 260.0), 0.12, 0.88);
    vec3 albedo = objectColor;
    vec3 extraEmissive = vec3(0.0);

    // Модификаторы 8 уникальных скинов (skinMode 3..7 полностью меняют материал и узор кузова)
    bool isGlass = (objectColor.r < 0.06 && objectColor.g < 0.08 && objectColor.b > 0.07);
    if (!isGlass && skinMode == 3 && reflectivity > 0.45) {
        // Скин 3 (INFERNO CARBON): тёмное углеволокно с пульсирующими раскалёнными трещинами лавы
        float w1 = sin((LocalPos.x + LocalPos.z) * 95.0) * sin((LocalPos.x - LocalPos.z) * 95.0);
        float weave = smoothstep(-0.2, 0.4, w1);
        albedo = mix(vec3(0.04, 0.04, 0.048), vec3(0.11, 0.10, 0.115), weave);
        roughness = mix(0.30, 0.16, weave);
        float crackWave = sin(LocalPos.z * 9.5 + sin(LocalPos.x * 13.0) * 1.8 + LocalPos.y * 6.0);
        float lavaCrack = smoothstep(0.955, 0.996, abs(crackWave));
        float pulse = 0.72 + 0.28 * sin(time * 4.5 - LocalPos.z * 3.0);
        extraEmissive += mix(vec3(2.2, 0.32, 0.02), vec3(2.8, 1.35, 0.12), lavaCrack) * lavaCrack * pulse;
    } else if (!isGlass && skinMode == 4 && reflectivity > 0.45) {
        // Скин 4 (HYPER CHAMELEON): зеркальный хамелеон, переливающийся от ультрафиолета к бирюзе и мадженте
        float angle = 1.0 - max(dot(N, V), 0.0);
        float shift = fract(angle * 1.25 + 0.12 * sin(LocalPos.z * 2.2 - time * 1.8));
        vec3 cA = vec3(0.32, 0.04, 0.78);
        vec3 cB = vec3(0.04, 0.92, 0.96);
        vec3 cC = vec3(1.00, 0.10, 0.62);
        albedo = mix(cA, cB, smoothstep(0.05, 0.45, shift));
        albedo = mix(albedo, cC, smoothstep(0.45, 0.88, shift));
        metallic = 0.94;
        roughness = 0.10;
        extraEmissive += mix(cB, cC, angle) * pow(angle, 3.0) * 0.45;
    } else if (!isGlass && skinMode == 5 && reflectivity > 0.45) {
        // Скин 5 (TOXIC BIOHAZARD): матовая тёмная тактическая броня с пульсирующими кислотными жилами
        albedo = vec3(0.06, 0.08, 0.055);
        metallic = 0.30;
        roughness = 0.56;
        float vein = sin(LocalPos.z * 12.0 - time * 4.2 + sin(LocalPos.x * 16.0 + LocalPos.y * 10.0) * 1.6);
        float glowMask = smoothstep(0.90, 0.988, vein);
        extraEmissive += vec3(0.28, 1.95, 0.08) * glowMask;
    } else if (!isGlass && skinMode == 6 && reflectivity > 0.45) {
        // Скин 6 (ROYAL 24K GOLD): зеркальное жидкое золото с бегущим солнечным бликом по кузову
        float gleam = smoothstep(0.86, 0.99, sin(LocalPos.z * 4.2 - time * 3.4));
        albedo = mix(vec3(0.96, 0.76, 0.20), vec3(1.0, 0.94, 0.65), gleam * 0.65);
        metallic = 0.98;
        roughness = 0.08;
        extraEmissive += vec3(1.1, 0.78, 0.15) * gleam * 0.28;
    } else if (!isGlass && skinMode == 7 && reflectivity > 0.45) {
        // Скин 7 (ARCTIC FROST): кристаллический ледяной хром с морозными гранями и голубым свечением
        float f1 = abs(sin(LocalPos.x * 18.0 + LocalPos.z * 14.0) * cos(LocalPos.y * 22.0 - LocalPos.z * 11.0));
        float crystal = smoothstep(0.25, 0.78, f1);
        float edgeIce = pow(1.0 - max(dot(N, V), 0.0), 2.5);
        albedo = mix(vec3(0.14, 0.58, 0.95), vec3(0.88, 0.97, 1.00), crystal);
        metallic = mix(0.65, 0.92, crystal);
        roughness = mix(0.22, 0.08, crystal);
        float frostLine = smoothstep(0.94, 0.995, abs(sin(LocalPos.z * 15.0 + LocalPos.x * 12.0)));
        extraEmissive += vec3(0.35, 0.88, 1.45) * (edgeIce * 0.42 + frostLine * 0.45);
    } else if (metallic > 0.65) {
        // Стандартная мелкая шлифовка нержавеющей стали 304 (волосные штрихи на DeLorean)
        float g1 = sin(LocalPos.x * 420.0 + LocalPos.y * 190.0) * 0.016;
        float g2 = sin(LocalPos.x * 890.0 - LocalPos.y * 310.0) * 0.008;
        albedo = clamp(albedo + vec3(g1 + g2), 0.0, 1.0);
        roughness = clamp(roughness + g1 * 1.4, 0.12, 0.42);
    }

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Контактное затенение под днищем транспорта
    float isCarPart = step(-2.6, FragPos.z - carPos.z) * step(FragPos.z - carPos.z, 2.6);
    float ao = mix(1.0, smoothstep(-0.52, 0.14, FragPos.y - carPos.y), isCarPart * 0.44);

    vec3 R = reflect(-V, N);
    vec3 envColor = sampleMoodyCyberEnv(R, underground);
    vec3 F_env = FresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD_env = (vec3(1.0) - F_env) * (1.0 - metallic);
    float hemi = N.y * 0.5 + 0.5;

    // Сбалансированное ночное освещение: атмосфера остаётся киберпанковой, но рельеф кузова хорошо читается
    vec3 ambientDiffuse = mix(vec3(0.055, 0.070, 0.105), vec3(0.22, 0.26, 0.34), hemi) * albedo * kD_env * lightScale;
    vec3 ambientSpec = envColor * F_env * mix(0.24, 1.15, reflectivity) * (1.0 - roughness * 0.42);

    vec3 L1 = normalize(lightPos1 - FragPos);
    vec3 L2 = normalize(lightPos2 - FragPos);
    vec3 col1 = mix(lightColor1, vec3(0.50, 0.64, 0.78), 0.52) * 1.65 * lightScale;
    vec3 col2 = mix(lightColor2, vec3(0.26, 0.48, 0.68), 0.45) * 1.10 * lightScale;

    vec3 Lo = calcPBRLight(N, V, L1, col1, albedo, metallic, roughness, F0) +
              calcPBRLight(N, V, L2, col2, albedo, metallic, roughness, F0);

    // Автомобильный двойной лак (Clearcoat): даёт реалистичный влажный глянец на краске под дождём
    if (reflectivity > 0.45 && skinMode != 5 && skinMode != 7) {
        vec3 H1 = normalize(V + L1);
        vec3 H2 = normalize(V + L2);
        float ccSpec1 = DistributionGGX(N, H1, 0.068) * 0.25 * max(dot(N, L1), 0.0);
        float ccSpec2 = DistributionGGX(N, H2, 0.078) * 0.16 * max(dot(N, L2), 0.0);
        float ccFresnel = 0.04 + 0.96 * pow(1.0 - max(dot(N, V), 0.0), 5.0);
        Lo += vec3(0.94, 0.97, 1.0) * (ccSpec1 + ccSpec2) * lightScale;
        ambientSpec += envColor * ccFresnel * 0.34 * smoothstep(0.45, 0.95, reflectivity);
    }

    vec3 baseLit = (ambientDiffuse + ambientSpec + Lo) * ao;

    // Прожекторы передних фар на объектах впереди (поездах, барьерах, опорах ЛРТ и стенах туннеля)
    float headlightHit = 0.0;
    if (headlightsOn && FragPos.z < carPos.z - 1.2) {
        vec3 hPosL = carPos + vec3(-0.68, 0.16, -2.0);
        vec3 hPosR = carPos + vec3( 0.68, 0.16, -2.0);
        vec3 spotDir = normalize(vec3(0.0, -0.04, -1.0));

        vec3 toL = FragPos - hPosL;
        float dL = length(toL);
        float thL = dot(toL / max(dL, 0.01), spotDir);

        vec3 toR = FragPos - hPosR;
        float dR = length(toR);
        float thR = dot(toR / max(dR, 0.01), spotDir);

        float linK  = mix(0.040, 0.014, underground);
        float quadK = mix(0.0062, 0.0014, underground);
        float coneL = clamp((thL - 0.68) / (0.88 - 0.68), 0.0, 1.0) / (1.0 + linK * dL + quadK * dL * dL);
        float coneR = clamp((thR - 0.68) / (0.88 - 0.68), 0.0, 1.0) / (1.0 + linK * dR + quadK * dR * dR);
        headlightHit = clamp(coneL + coneR, 0.0, 1.5);
        float hDiff = max(dot(N, -spotDir), 0.35);
        vec3 beamCol = vec3(1.0, 0.95, 0.86) * mix(0.42, 3.0, underground);
        baseLit += (coneL + coneR) * hDiff * beamCol * albedo;
    }

    // Контровой свет по краю силуэта (выделяет тёмные детали на тёмном фоне)
    float rim = pow(1.0 - max(dot(N, V), 0.0), 4.2);
    baseLit += vec3(0.16, 0.45, 0.65) * rim * 0.22 * lightScale;

    float emScale = mix(1.0, mix(0.10 + 0.90 * clamp(headlightHit, 0.0, 1.0), 1.0, isCarPart), underground);
    vec3 result = baseLit + (emissiveColor + extraEmissive) * emScale;

    // Дальний ночной туман
    float dist = length(FragPos - viewPos);
    float fogFactor = smoothstep(60.0, 175.0, dist);
    vec3 fogColor = mix(vec3(0.012, 0.015, 0.024), vec3(0.005, 0.005, 0.010), underground);
    result = mix(result, fogColor, fogFactor);

    FragColor = vec4(result, 1.0);
}
