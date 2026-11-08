#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 camPos;

void main()
{
    vec2 gridUV = TexCoord;
    vec2 gridD = abs(fract(gridUV - 0.5) - 0.5) / fwidth(gridUV);
    float line = 1.0 - min(min(gridD.x, gridD.y), 1.0);

    float x = FragPos.x;
    float roadHalfWidth = 5.0;
    bool onRoad = abs(x) < roadHalfWidth;

    vec3 groundBase = onRoad ? vec3(0.03, 0.01, 0.08) : vec3(0.05, 0.01, 0.10);

    float sideFactor = smoothstep(0.0, 30.0, abs(x));
    vec3 lineColor = mix(vec3(1.0, 0.1, 0.75), vec3(0.0, 0.9, 1.0), sideFactor);
    vec3 color = groundBase + lineColor * line * 1.5;

    float curbDist = abs(abs(x) - roadHalfWidth);
    float curbGlow = 1.0 - smoothstep(0.0, 0.25, curbDist);
    color += vec3(0.0, 1.0, 0.95) * curbGlow * 1.6;

    float centerDist = abs(x);
    float dash = step(0.5, fract(gridUV.y * 0.2)) * (1.0 - smoothstep(0.05, 0.18, centerDist));
    color += vec3(1.0, 0.85, 0.2) * dash * 1.8;

    float dist = length(FragPos - camPos);
    float fogFactor = smoothstep(60.0, 150.0, dist);
    vec3 fogColor = vec3(0.95, 0.25, 0.55);
    color = mix(color, fogColor, fogFactor);

    FragColor = vec4(color, 1.0);
}
