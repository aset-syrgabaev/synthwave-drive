#version 330 core
// ============================================================================
// ВЕРШИННЫЙ ШЕЙДЕР ПОСТ-ОБРАБОТКИ (postprocess.vert) — Stretch Goals #2 и #3
// Рисует полноэкранный квадрат (Screen Quad), на который натягивается готовая
// HDR-текстура 3D-сцены из FBO (PostProcess.h) для наложения Bloom, ACES и CRT.
// ============================================================================
layout (location = 0) in vec2 aPos;       // экранная позиция угла (-1..+1 в NDC)
layout (location = 1) in vec2 aTexCoords; // текстурные координаты (0..1)

out vec2 TexCoords; // передаём UV в postprocess.frag

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
