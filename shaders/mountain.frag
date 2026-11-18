#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 viewPos;
uniform vec3 baseColor;
uniform vec3 peakColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.0, 0.9, -1.0));
    float diff = max(dot(norm, lightDir), 0.0);

    float heightFactor = clamp(FragPos.y / 15.0, 0.0, 1.0);
    vec3 surfaceColor = mix(baseColor, peakColor, heightFactor);

    vec3 ambient = baseColor * 0.45;
    vec3 diffuse = diff * surfaceColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * peakColor * 0.4;

    vec3 color = ambient + diffuse + specular;

    float dist = length(FragPos - viewPos);
    float fog = smoothstep(45.0, 140.0, dist);
    vec3 fogColor = vec3(0.95, 0.25, 0.55);
    color = mix(color, fogColor, fog);

    FragColor = vec4(color, 1.0);
}
