#version 330 core
// ============================================================================
// ВЕРШИННЫЙ ШЕЙДЕР ИНСТАНСИНГА ПАЛЬМ (instanced.vert) — STRETCH GOAL #1 + B4
// ВАЖНО ДЛЯ ЗАЩИТЫ: вместо 40 отдельных вызовов glDrawArrays (по одному на дерево),
// мы грузим геометрию ОДНОЙ пальмы и массив из 40 матриц instanceMatrix в отдельный VBO.
// Тип mat4 занимает 4 слота атрибутов подряд (location 3, 4, 5, 6), и в PalmTree.h
// вызвано glVertexAttribDivisor(3+i, 1) — матрица обновляется 1 раз на целую пальму!
// ============================================================================
layout (location = 0) in vec3 aPos;             // вершина ствола или листа (из базового VBO)
layout (location = 1) in vec3 aNormal;          // нормаль вершины для освещения Блинна-Фонга
layout (location = 2) in vec3 aColor;           // цвет вершины (фиолетовый ствол / бирюзовая крона)
layout (location = 3) in mat4 instanceMatrix;   // индивидуальная матрица Model (позиция, поворот, масштаб)

uniform mat4 view;       // матрица вида камеры
uniform mat4 projection; // матрица перспективной проекции

out vec3 FragPos;   // мировая координата точки
out vec3 Normal;    // мировая нормаль
out vec3 VertColor; // цвет вершины

void main()
{
    // Каждая копия пальмы умножается на СВОЮ матрицу instanceMatrix прямо на видеокарте
    FragPos = vec3(instanceMatrix * vec4(aPos, 1.0));
    // Корректное преобразование нормали через обратную транспонированную матрицу
    Normal = mat3(transpose(inverse(instanceMatrix))) * aNormal;
    VertColor = aColor;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
