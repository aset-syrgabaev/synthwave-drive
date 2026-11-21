#version 330 core
// ============================================================================
// ВЕРШИННЫЙ ШЕЙДЕР ГОР И БАРХАНОВ (mountain.vert) — ядро Темы #13 (R1, R3)
// Преобразует вершины процедурных гор (Mountain.h) по формуле MVP = P * V * M
// и передаёт мировые координаты FragPos и нормали Normal для освещения склонов.
// ============================================================================
layout (location = 0) in vec3 aPos;    // позиция вершины горы/дюны
layout (location = 1) in vec3 aNormal; // усреднённая нормаль склона

uniform mat4 model;      // матрица модели (единичная для статичных гор)
uniform mat4 view;       // матрица вида камеры
uniform mat4 projection; // матрица перспективной проекции

out vec3 FragPos; // позиция вершины в мире (для высоты снега/песка и тумана)
out vec3 Normal;  // мировая нормаль (для освещения и маски снега по наклону)

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
