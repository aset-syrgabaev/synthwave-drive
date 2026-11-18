#version 330 core
in vec2 UV;
in vec4 Color;
out vec4 FragColor;

uniform sampler2D atlas;

void main()
{
    float a = texture(atlas, UV).r;
    FragColor = vec4(Color.rgb, Color.a * a);
}
