#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool crtEnabled;
uniform float time;
uniform vec2 resolution;

void main()
{
    if (!crtEnabled) {
        FragColor = texture(screenTexture, TexCoords);
        return;
    }

    vec2 uv = TexCoords;

    vec2 uvDist = uv - 0.5;
    uv = uv + uvDist * (dot(uvDist, uvDist) * 0.06);

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        FragColor = vec4(0.02, 0.0, 0.05, 1.0);
        return;
    }

    float shift = 0.0016;
    float r = texture(screenTexture, uv + vec2(shift, 0.0)).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - vec2(shift, 0.0)).b;
    vec3 col = vec3(r, g, b);

    float scanline = sin(uv.y * resolution.y * 1.8) * 0.04;
    col -= scanline;

    float vig = uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);
    vig = clamp(pow(16.0 * vig, 0.18), 0.0, 1.0);
    col *= vig;

    col = col * 1.05;

    FragColor = vec4(col, 1.0);
}
