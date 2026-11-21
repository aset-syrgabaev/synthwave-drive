#version 330 core
// Вершинный шейдер для 2D-интерфейса (текст, кнопки, рамки HUD).
layout (location = 0) in vec2 aPos;   // позиция угла в пикселях окна
layout (location = 1) in vec2 aUV;    // координаты символа в текстуре-атласе шрифта
layout (location = 2) in vec4 aColor; // цвет RGBA

uniform vec2 screen; // текущие ширина и высота окна в пикселях

out vec2 UV;
out vec4 Color;

void main()
{
    // Переводим координаты из пикселей (0..screen) в OpenGL NDC (-1..1).
    // Ось Y инвертируем, так как в интерфейсе Y=0 сверху, а в OpenGL снизу.
    vec2 ndc = vec2(aPos.x / screen.x * 2.0 - 1.0, 1.0 - aPos.y / screen.y * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    UV = aUV;
    Color = aColor;
}
