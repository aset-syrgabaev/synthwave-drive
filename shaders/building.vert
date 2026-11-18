#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in mat4 instanceMatrix;
layout (location = 7) in vec4 instParams;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 Size;
out vec3 Local;
out vec4 Params;

void main()
{
    vec4 world = instanceMatrix * vec4(aPos, 1.0);
    FragPos = world.xyz;
    Normal = normalize(mat3(instanceMatrix) * aNormal);
    Size = vec3(length(instanceMatrix[0].xyz), length(instanceMatrix[1].xyz), length(instanceMatrix[2].xyz));
    Local = (aPos + 0.5) * Size;
    Params = instParams;
    gl_Position = projection * view * world;
}
