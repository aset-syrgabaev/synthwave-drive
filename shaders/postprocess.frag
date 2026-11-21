#version 330 core
// Финальная пост-обработка HDR-кадра:
// - двухрадиусный Bloom для свечения билбордов, фар и неона;
// - тональная компрессия ACES (делает глубокие тени и сочные блики как в кино);
// - лёгкое киноплёночное зерно, виньетка по краям и опциональный CRT-фильтр (клавиша C).
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture; // отрендеренная 3D-сцена из HDR фреймбуфера
uniform bool crtEnabled;         // включён ли ретро-эффект старого монитора
uniform float bloomStrength;     // сила свечения (настраивается в меню G)
uniform float time;              // время для анимации лёгкого шума плёнки
uniform vec2 resolution;         // разрешение экрана в пикселях

// Отбираем только самые яркие участки кадра (яркость > 1.05), чтобы обычные объекты не расплывались
vec3 sampleBright(vec2 uv)
{
    vec3 c = texture(screenTexture, clamp(uv, 0.001, 0.999)).rgb;
    float lum = max(c.r, max(c.g, c.b));
    return c * smoothstep(1.05, 1.85, lum);
}

// Считаем мягкий ореол свечения: смешиваем узкий и широкий радиус размытия
vec3 computeBloom(vec2 uv)
{
    vec2 texel = 1.0 / max(resolution, vec2(1.0));
    vec3 acc = vec3(0.0);
    float total = 0.0;
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            float w = 1.0 / (1.0 + float(x * x + y * y) * 0.7);
            acc += sampleBright(uv + vec2(float(x), float(y)) * texel * 2.2) * w;
            acc += sampleBright(uv + vec2(float(x), float(y)) * texel * 5.4) * w * 0.58;
            total += w * 1.58;
        }
    }
    return acc / max(total, 1.0);
}

// Стандартная кино-кривая ACES Filmic Tonemapping (переводит HDR-цвета в диапазон [0..1])
vec3 acesToneMap(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Быстрый шум для еле заметного зерна киноплёнки
float filmGrain(vec2 uv)
{
    return fract(sin(dot(uv + fract(time * 0.17), vec2(12.9898, 78.233))) * 43758.5453) - 0.5;
}

void main()
{
    vec2 uv = TexCoords;

    // STRETCH GOAL #3 (клавиша C): бочкообразное искривление экрана старого кинескопного CRT-монитора.
    // Чем дальше пиксель от центра (0.5, 0.5), тем сильнее сдвигаются его UV-координаты наружу.
    if (crtEnabled) {
        vec2 uvDist = uv - 0.5;
        uv = uv + uvDist * (dot(uvDist, uvDist) * 0.045);
        // Пиксели, вышедшие за изогнутую границу кинескопа, закрашиваем тёмной рамкой
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
            FragColor = vec4(0.006, 0.006, 0.014, 1.0);
            return;
        }
    }

    // При включённом CRT добавляем хроматическую аберрацию (сдвиг красного и синего каналов на ±0.0014)
    vec3 col;
    if (crtEnabled) {
        float shift = 0.0014;
        float r = texture(screenTexture, uv + vec2(shift, 0.0)).r;
        float g = texture(screenTexture, uv).g;
        float b = texture(screenTexture, uv - vec2(shift, 0.0)).b;
        col = vec3(r, g, b);
    } else {
        col = texture(screenTexture, uv).rgb;
    }

    // STRETCH GOAL #2 (свечение вокруг неоновых линий и фар):
    // Прибавляем размытые яркие пиксели (HDR > 1.05) из computeBloom к основной картинке.
    if (bloomStrength > 0.01) {
        vec3 bloom = computeBloom(uv);
        col += bloom * bloomStrength;
    }

    // Кинематографичная цветокоррекция ACES: мягко сжимает яркость из HDR (>1.0) в диапазон монитора [0..1]
    col = acesToneMap(col * 1.18);
    col = pow(max(col, vec3(0.0)), vec3(0.99));

    // Лёгкое плёночное зерно (убирает «ступеньки» градиента на тёмном небе)
    col += filmGrain(uv * resolution * 0.5) * 0.012;

    // STRETCH GOAL #3 (сканлайны CRT): горизонтальные тёмные полосы по синусоиде от высоты экрана в пикселях
    if (crtEnabled) {
        float scanline = sin(uv.y * resolution.y * 1.8) * 0.028;
        col -= scanline;
    }

    // Мягкая виньетка по краям экрана
    float vig = uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);
    vig = clamp(pow(16.0 * vig, 0.20), 0.0, 1.0);
    col *= mix(0.78, 1.0, vig);

    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
