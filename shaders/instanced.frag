#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 VertColor;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.35 * VertColor;
    vec3 diffuse = diff * lightColor * VertColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor * 0.5;

    vec3 color = ambient + diffuse + specular;

    float dist = length(FragPos - viewPos);
    float fog = smoothstep(50.0, 150.0, dist);
    vec3 fogColor = vec3(0.95, 0.25, 0.55);
    color = mix(color, fogColor, fog);

    FragColor = vec4(color, 1.0);
}
