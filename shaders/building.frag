#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 Size;
in vec3 Local;
in vec4 Params;
out vec4 FragColor;

uniform vec3 viewPos;
uniform float time;
uniform vec3 fogColor;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 base = Params.rgb;
    vec3 col = vec3(0.02, 0.015, 0.05);

    if (Params.a < 0.0) {
        float pulse = 0.8 + 0.2 * sin(time * 3.0 + FragPos.z * 0.2);
        col = base * 2.4 * pulse;
    } else if (abs(N.y) > 0.5) {
        col = vec3(0.03, 0.02, 0.06) + base * 0.12;
    } else {
        float u = abs(N.x) > 0.5 ? Local.z : Local.x;
        vec2 g = vec2(u, Local.y) / vec2(1.8, 2.6);
        vec2 id = floor(g);
        vec2 f = fract(g);
        float win = step(0.22, f.x) * step(f.x, 0.78) * step(0.25, f.y) * step(f.y, 0.80);
        float r = hash(id + Params.a * 17.0);
        float on = step(0.42, r);
        vec3 wc = mix(base, vec3(1.0, 0.85, 0.5), step(0.72, hash(id + 3.1)));
        float flicker = 1.0 + 0.35 * sin(time * 2.0 + r * 30.0) * step(0.9, r);
        col += win * on * wc * 1.2 * flicker;

        float band = step(0.47, abs(fract(Local.y / 9.0) - 0.5));
        col += base * band * 2.0;
        col += base * step(Size.y - 0.5, Local.y) * 2.6;
    }

    float dist = length(FragPos - viewPos);
    float fog = smoothstep(70.0, 240.0, dist);
    col = mix(col, fogColor, fog);
    FragColor = vec4(col, 1.0);
}
