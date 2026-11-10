#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 emissiveColor;
uniform float shininess;
uniform vec3 viewPos;
uniform vec3 lightPos1;
uniform vec3 lightColor1;
uniform vec3 lightPos2;
uniform vec3 lightColor2;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 ambient = 0.25 * objectColor;

    // Light 1 (Sun light)
    vec3 lightDir1 = normalize(lightPos1 - FragPos);
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse1 = diff1 * lightColor1;
    vec3 halfwayDir1 = normalize(lightDir1 + viewDir);
    float spec1 = pow(max(dot(norm, halfwayDir1), 0.0), shininess);
    vec3 specular1 = spec1 * lightColor1;

    // Light 2 (Dynamic light)
    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * lightColor2;
    vec3 halfwayDir2 = normalize(lightDir2 + viewDir);
    float spec2 = pow(max(dot(norm, halfwayDir2), 0.0), shininess);
    vec3 specular2 = spec2 * lightColor2;

    vec3 result = ambient + (diffuse1 + specular1 + diffuse2 + specular2) * objectColor + emissiveColor;
    FragColor = vec4(result, 1.0);
}
