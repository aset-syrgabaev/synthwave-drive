#version 330 core
out vec4 FragColor;
in vec2 UV;

uniform float time;
uniform vec2 resolution;
uniform vec3 skyTopColor;
uniform vec3 skyMidColor;
uniform vec3 skyHorizonColor;

void main()
{
    vec2 p = UV;

    vec3 sky = mix(skyHorizonColor, skyMidColor, smoothstep(0.15, 0.55, p.y));
    sky = mix(sky, skyTopColor, smoothstep(0.55, 1.0, p.y));

    float starNoise = sin(p.x * 240.0) * cos(p.y * 360.0);
    if (starNoise > 0.982 && p.y > 0.40) {
        float twinkle = 0.5 + 0.5 * sin(time * 3.5 + p.x * 70.0);
        sky += vec3(0.85, 0.85, 1.0) * twinkle;
    }

    vec2 sunPos = vec2(0.5, 0.38);
    vec2 aspect = vec2(resolution.x / resolution.y, 1.0);
    vec2 d = (p - sunPos) * aspect;
    float dist = length(d);
    float sunRadius = 0.23;

    if (dist < sunRadius) {
        float sunY = (p.y - (sunPos.y - sunRadius)) / (2.0 * sunRadius);
        vec3 sunTopColor = vec3(1.0, 0.92, 0.25);
        vec3 sunBottomColor = vec3(1.0, 0.05, 0.45);
        vec3 sunColor = mix(sunBottomColor, sunTopColor, sunY);

        float cut = 1.0;
        if (p.y < sunPos.y) {
            float normY = (sunPos.y - p.y) / sunRadius;
            float stripePeriod = 0.035;
            float stripePhase = mod(p.y + time * 0.015, stripePeriod) / stripePeriod;
            float barWidth = mix(0.15, 0.65, normY);
            if (stripePhase < barWidth) {
                cut = 0.0;
            }
        }
        sky = mix(sky, sunColor, cut);
    }

    float glow = exp(-dist * 4.5) * 0.55;
    sky += vec3(1.0, 0.25, 0.5) * glow;

    FragColor = vec4(sky, 1.0);
}
