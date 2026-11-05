#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

void main()
{
    vec2 grid = abs(fract(TexCoord - 0.5) - 0.5) / fwidth(TexCoord);
    float line = min(grid.x, grid.y);
    float c = 1.0 - min(line, 1.0);

    vec3 backgroundColor = vec3(0.05, 0.0, 0.15);
    vec3 gridColor = vec3(0.0, 1.0, 0.9);

    vec3 finalColor = mix(backgroundColor, gridColor, c);
    FragColor = vec4(finalColor, 1.0);
}
