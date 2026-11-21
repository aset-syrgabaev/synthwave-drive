#version 330 core
// Вершинный шейдер для зданий и электронных билбордов города.
// Использует инстансинг: один и тот же куб (aPos, aNormal) рисуется много раз
// с разными матрицами (instanceMatrix) и параметрами оформления (instParams).
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in mat4 instanceMatrix; // матрица каждого отдельного здания
layout (location = 7) in vec4 instParams;     // цвет, seed для окон и тип объекта

uniform mat4 view;       // матрица камеры
uniform mat4 projection; // матрица перспективы

out vec3 FragPos; // мировые координаты вершины
out vec3 Normal;  // нормаль в мировом пространстве
out vec3 Size;    // реальные размеры (ширина, высота, глубина) здания в метрах
out vec3 Local;   // координата внутри фасада в метрах (от 0 до Size) для сетки окон
out vec4 Params;  // передаём параметры здания дальше во фрагментный шейдер

void main()
{
    vec4 world = instanceMatrix * vec4(aPos, 1.0);
    FragPos = world.xyz;
    Normal = normalize(mat3(instanceMatrix) * aNormal);
    // Длина первых трёх столбцов матрицы даёт масштаб (размер) коробки по осям X, Y, Z
    Size = vec3(length(instanceMatrix[0].xyz), length(instanceMatrix[1].xyz), length(instanceMatrix[2].xyz));
    Local = (aPos + 0.5) * Size;
    Params = instParams;
    gl_Position = projection * view * world;
}
