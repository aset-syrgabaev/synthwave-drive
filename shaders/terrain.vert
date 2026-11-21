#version 330 core
// Вершинный шейдер дороги и земли.
// Отвечает за бесконечную прокрутку покрытия и плавное опускание дороги на -6м под землю в туннеле.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float scrollOffset; // сколько метров проехал игрок (для смещения текстуры и туннелей)

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

// Повторяет формулу из Track::tunnelDepthAt в C++, чтобы визуальный спуск совпадал с физикой машины.
// Каждые 460 м идёт цикл: 35м плавный спуск, 200м подземный участок, 35м плавный подъём.
float tunnelDepthAt(float trackDist)
{
    float c = mod(trackDist, 460.0);
    if (c < 0.0) c += 460.0;
    float down = smoothstep(150.0, 185.0, c);
    float up   = smoothstep(385.0, 420.0, c);
    return -6.0 * (down - up);
}

void main()
{
    vec3 pos = aPos;
    float trackDist = scrollOffset - aPos.z;
    // Опускаем только дорожную полосу (|x| <= 4.82), а обочины со зданиями оставляем на уровне 0
    float roadMask = 1.0 - smoothstep(4.82, 5.08, abs(aPos.x));
    pos.y = tunnelDepthAt(trackDist) * roadMask;

    // Для боковых стенок спуска поворачиваем нормаль внутрь дороги, чтобы на них падал свет фар
    vec3 nrm = aNormal;
    if (abs(aPos.x) > 4.80 && abs(aPos.x) < 5.10 && pos.y < -0.1) {
        nrm = normalize(vec3(-sign(aPos.x), 0.25, 0.0));
    }

    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * nrm;
    // Сдвигаем UV по вертикали, создавая иллюзию движения дороги под машиной
    TexCoord = vec2(aTexCoord.x, aTexCoord.y - scrollOffset * 0.4);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
