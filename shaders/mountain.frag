#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 viewPos;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.0, 1.0, -1.0));
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 darkPurple = vec3(0.12, 0.02, 0.22);
    vec3 neonPink = vec3(0.95, 0.1, 0.65);
    vec3 color = mix(darkPurple, neonPink, diff);

    float dist = length(FragPos - viewPos);
    float fog = smoothstep(50.0, 140.0, dist);
    vec3 fogColor = vec3(0.95, 0.25, 0.55);
    color = mix(color, fogColor, fog);

    FragColor = vec4(color, 1.0);
}
