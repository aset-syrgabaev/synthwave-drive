#version 330 core
// ============================================================================
// ФРАГМЕНТНЫЙ ШЕЙДЕР ПАЛЬМ (instanced.frag) — критерий R4 (Блинн-Фонг) + Stretch #1
// Вычисляет классическое пофрагментное освещение Блинна-Фонга:
//   I = Ambient (фоновый) + Diffuse (рассеянный max(N·L, 0)) + Specular (блик max(N·H, 0)^32)
// ============================================================================
in vec3 FragPos;   // позиция пикселя пальмы в мире
in vec3 Normal;    // интерполированная нормаль поверхности
in vec3 VertColor; // базовый цвет (ствол или листья)
out vec4 FragColor;

uniform vec3 lightPos;   // позиция основного источника света (солнце на горизонте)
uniform vec3 lightColor; // акцентный цвет освещения текущей локации
uniform vec3 viewPos;    // позиция камеры (нужна для расчёта блика Specular)

void main()
{
    // 1. Нормализуем нормаль N и направление на солнце L
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    // 2. Диффузный свет (закон Ламберта: грань ярче, если повёрнута к свету)
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.35 * VertColor;            // мягкая фоновая подсветка, чтобы тень не была чёрной
    vec3 diffuse = diff * lightColor * VertColor;

    // 3. Зеркальный блик по модели Блинна-Фонга через половинный вектор H = normalize(L + V)
    //    (быстрее и стабильнее классического Фонга, нет резкого обрыва блика при углах > 90°)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor * 0.5;

    vec3 color = ambient + diffuse + specular;

    // 4. Дальний атмосферный туман: плавно растворяем дальние пальмы (50..150 м), чтобы не появлялись резко
    float dist = length(FragPos - viewPos);
    float fog = smoothstep(50.0, 150.0, dist);
    vec3 fogColor = vec3(0.95, 0.25, 0.55);
    color = mix(color, fogColor, fog);

    FragColor = vec4(color, 1.0);
}
