#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 camPos;
uniform vec3 carPos;
uniform bool headlightsOn;
uniform vec3 gridLineColor;
uniform vec3 roadColor;

void main()
{
    vec2 gridUV = TexCoord;
    vec2 gridD = abs(fract(gridUV - 0.5) - 0.5) / fwidth(gridUV);
    float line = 1.0 - min(min(gridD.x, gridD.y), 1.0);

    float x = FragPos.x;
    float roadHalfWidth = 4.8;
    bool onRoad = abs(x) < roadHalfWidth;

    vec3 groundBase = onRoad ? roadColor : (roadColor * 0.45);
    vec3 color = groundBase + gridLineColor * line * (onRoad ? 0.35 : 0.85);

    float roadBorderDist = abs(abs(x) - roadHalfWidth);
    float borderLine = 1.0 - smoothstep(0.04, 0.16, roadBorderDist);
    color += gridLineColor * borderLine * 2.5;

    float dashY = step(0.45, fract(TexCoord.y * 0.5));
    float laneDash1 = dashY * (1.0 - smoothstep(0.04, 0.14, abs(x - 1.2)));
    float laneDash2 = dashY * (1.0 - smoothstep(0.04, 0.14, abs(x + 1.2)));
    color += vec3(1.0, 0.9, 0.25) * (laneDash1 + laneDash2) * 2.2;

    if (headlightsOn) {
        vec3 lightPosL = carPos + vec3(-0.65, 0.2, -1.8);
        vec3 lightPosR = carPos + vec3( 0.65, 0.2, -1.8);
        vec3 spotDir = normalize(vec3(0.0, -0.15, -1.0));

        vec3 toFragL = normalize(FragPos - lightPosL);
        float thetaL = dot(toFragL, spotDir);
        float distL = length(FragPos - lightPosL);

        vec3 toFragR = normalize(FragPos - lightPosR);
        float thetaR = dot(toFragR, spotDir);
        float distR = length(FragPos - lightPosR);

        float cutoff = 0.86;
        float outerCutoff = 0.76;

        float intensityL = clamp((thetaL - outerCutoff) / (cutoff - outerCutoff), 0.0, 1.0) / (1.0 + 0.04 * distL * distL);
        float intensityR = clamp((thetaR - outerCutoff) / (cutoff - outerCutoff), 0.0, 1.0) / (1.0 + 0.04 * distR * distR);

        vec3 lightCol = vec3(0.2, 0.95, 1.0);
        color += (intensityL + intensityR) * lightCol * 3.5;
    }

    float dist = length(FragPos - camPos);
    float fogFactor = smoothstep(55.0, 150.0, dist);
    vec3 fogColor = vec3(0.95, 0.25, 0.55);
    color = mix(color, fogColor, fogFactor);

    FragColor = vec4(color, 1.0);
}
