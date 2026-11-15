#version 330 core
out vec4 FragColor;
in vec2 UV;

uniform float time;
uniform vec2 resolution;

void main()
{
    vec2 p = UV;
    
    // Sky gradient from deep night purple to warm magenta/orange at horizon
    vec3 colTop = vec3(0.04, 0.0, 0.12);
    vec3 colMid = vec3(0.35, 0.02, 0.45);
    vec3 colHorizon = vec3(0.95, 0.25, 0.55);
    
    vec3 sky = mix(colHorizon, colMid, smoothstep(0.2, 0.6, p.y));
    sky = mix(sky, colTop, smoothstep(0.6, 1.0, p.y));

    // Stars in upper sky
    float n = sin(p.x * 210.0) * cos(p.y * 330.0);
    if (n > 0.985 && p.y > 0.45) {
        sky += vec3(0.8, 0.8, 1.0) * (0.5 + 0.5 * sin(time * 3.0 + p.x * 50.0));
    }

    // Striped 80s Sun
    vec2 sunPos = vec2(0.5, 0.38);
    vec2 aspect = vec2(resolution.x / resolution.y, 1.0);
    vec2 d = (p - sunPos) * aspect;
    float dist = length(d);
    float sunRadius = 0.22;

    if (dist < sunRadius) {
        float sunY = (p.y - (sunPos.y - sunRadius)) / (2.0 * sunRadius);
        vec3 sunTopColor = vec3(1.0, 0.92, 0.25);
        vec3 sunBottomColor = vec3(1.0, 0.05, 0.45);
        vec3 sunColor = mix(sunBottomColor, sunTopColor, sunY);

        // Horizontal synthwave stripes (progressively wider cuts towards bottom)
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

    // Sun outer glow
    float glow = exp(-dist * 5.0) * 0.55;
    sky += vec3(1.0, 0.25, 0.5) * glow;

    FragColor = vec4(sky, 1.0);
}
