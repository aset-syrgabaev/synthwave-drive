#include "CarModel.h"

// Генерация дверей крыло чайки, обтекателей и лент световых шлейфов
namespace CarModel {

// Трапециевидная дверь "крыло чайки" DeLorean DMC-12 — герметичная, со скосом под лобовое стекло
Mesh gullwingDoorMetal(float sx) {
    std::vector<float> v;
    float zTopF = -0.37f;
    float zTopR =  0.37f;
    float zBotF = -0.98f;
    float zBotR =  0.37f;

    Primitives::appendBox(v, glm::vec3(sx * 0.23f, -0.012f, 0.0f), glm::vec3(0.46f, 0.036f, 0.74f));

    glm::vec3 rOutF(sx * 0.46f,  0.00f, zTopF);
    glm::vec3 rOutR(sx * 0.46f,  0.00f, zTopR);
    glm::vec3 wF   (sx * 0.71f, -0.41f, zBotF);
    glm::vec3 wR   (sx * 0.72f, -0.40f, zBotR);
    glm::vec3 mF   (sx * 0.79f, -0.63f, zBotF);
    glm::vec3 mR   (sx * 0.80f, -0.62f, zBotR);
    glm::vec3 bF   (sx * 0.67f, -1.01f, zBotF);
    glm::vec3 bR   (sx * 0.67f, -1.01f, zBotR);

    glm::vec3 pF_in0(sx * 0.46f,  0.00f, zTopF + 0.06f);
    glm::vec3 pF_in1(sx * 0.71f, -0.41f, zBotF + 0.09f);
    quadTwoSided(v, rOutF, wF, pF_in1, pF_in0);

    glm::vec3 pR_in0(sx * 0.46f,  0.00f, zTopR - 0.06f);
    glm::vec3 pR_in1(sx * 0.72f, -0.40f, zBotR - 0.06f);
    quadTwoSided(v, pR_in0, pR_in1, wR, rOutR);

    glm::vec3 tInF(sx * 0.49f, -0.045f, zTopF + 0.05f);
    glm::vec3 tInR(sx * 0.49f, -0.045f, zTopR - 0.05f);
    quadTwoSided(v, rOutF, tInF, tInR, rOutR);

    quadTwoSided(v, wF, mF, mR, wR);
    quadTwoSided(v, mF, bF, bR, mR);

    glm::vec3 inWF(sx * 0.62f, -0.41f, zBotF);
    glm::vec3 inWR(sx * 0.62f, -0.40f, zBotR);
    glm::vec3 inBF(sx * 0.60f, -1.01f, zBotF);
    glm::vec3 inBR(sx * 0.60f, -1.01f, zBotR);
    quadTwoSided(v, inWF, inWR, inBR, inBF);
    quadTwoSided(v, wF, wR, inWR, inWF);
    quadTwoSided(v, bF, inBF, inBR, bR);
    quadTwoSided(v, wF, inWF, inBF, bF);
    quadTwoSided(v, wR, bR, inBR, inWR);

    Mesh m;
    m.upload(v);
    return m;
}

Mesh gullwingDoorGlass(float sx) {
    std::vector<float> v;
    float zTopF = -0.32f;
    float zTopR =  0.32f;
    float zBotF = -0.91f;
    float zBotR =  0.32f;

    glm::vec3 gTF(sx * 0.47f, -0.02f, zTopF);
    glm::vec3 gTR(sx * 0.47f, -0.02f, zTopR);
    glm::vec3 gBR(sx * 0.71f, -0.40f, zBotR);
    glm::vec3 gBF(sx * 0.705f, -0.41f, zBotF);
    quadTwoSided(v, gTF, gBF, gBR, gTR);

    Mesh m;
    m.upload(v);
    return m;
}

// Детализация двери: чёрный молдинг, перемычка фирменной маленькой форточки DeLorean и утопленная ручка двери
Mesh gullwingDoorTrim(float sx) {
    std::vector<float> v;
    float zBotF = -0.97f;
    float zBotR =  0.36f;

    // Чёрный боковой резиновый молдинг
    glm::vec3 r0(sx * 0.796f, -0.610f, zBotF);
    glm::vec3 r1(sx * 0.806f, -0.600f, zBotR);
    glm::vec3 r2(sx * 0.806f, -0.652f, zBotR);
    glm::vec3 r3(sx * 0.796f, -0.662f, zBotF);
    quadTwoSided(v, r0, r1, r2, r3);

    // Фирменная горизонтальная и вертикальная рамка маленькой форточки "toll-booth window"
    glm::vec3 bar0(sx * 0.625f, -0.265f, -0.67f);
    glm::vec3 bar1(sx * 0.630f, -0.260f, 0.33f);
    glm::vec3 bar2(sx * 0.645f, -0.290f, 0.33f);
    glm::vec3 bar3(sx * 0.640f, -0.295f, -0.69f);
    quadTwoSided(v, bar0, bar1, bar2, bar3);

    glm::vec3 vbar0(sx * 0.640f, -0.285f, -0.06f);
    glm::vec3 vbar1(sx * 0.640f, -0.285f, -0.03f);
    glm::vec3 vbar2(sx * 0.715f, -0.405f, -0.03f);
    glm::vec3 vbar3(sx * 0.715f, -0.405f, -0.06f);
    quadTwoSided(v, vbar0, vbar1, vbar2, vbar3);

    // Утопленная чёрная ручка двери DeLorean
    Primitives::appendBox(v, glm::vec3(sx * 0.755f, -0.495f, 0.20f), glm::vec3(0.028f, 0.055f, 0.145f));

    Mesh m;
    m.upload(v);
    return m;
}

// ============================================================================

// Гладкий сглаженный лофтинг для обтекаемых красных кузовных панелей мотоцикла Акиры
Mesh loftSmoothBike(const std::vector<BikeSection>& sec) {
    const int R = 12;
    const int n = static_cast<int>(sec.size());
    std::vector<glm::vec3> P(n * R);
    for (int i = 0; i < n; ++i) {
        const auto& s = sec[i];
        float ym2 = (s.yMid + s.yTop) * 0.5f;
        float wm2 = (s.wMid + s.wTop) * 0.52f;
        P[i * R + 0]  = glm::vec3( 0.0f,    s.yBot,              s.z);
        P[i * R + 1]  = glm::vec3(-s.wBot,  s.yBot + 0.02f,      s.z);
        P[i * R + 2]  = glm::vec3(-s.wMid,  s.yMid,              s.z);
        P[i * R + 3]  = glm::vec3(-wm2,     ym2,                 s.z);
        P[i * R + 4]  = glm::vec3(-s.wTop,  s.yTop - 0.015f,     s.z);
        P[i * R + 5]  = glm::vec3( 0.0f,    s.yTop,              s.z);
        P[i * R + 6]  = glm::vec3( s.wTop,  s.yTop - 0.015f,     s.z);
        P[i * R + 7]  = glm::vec3( wm2,     ym2,                 s.z);
        P[i * R + 8]  = glm::vec3( s.wMid,  s.yMid,              s.z);
        P[i * R + 9]  = glm::vec3( s.wBot,  s.yBot + 0.02f,      s.z);
        P[i * R + 10] = glm::vec3( s.wBot * 0.5f, s.yBot,        s.z);
        P[i * R + 11] = glm::vec3(-s.wBot * 0.5f, s.yBot,        s.z);
    }

    std::vector<glm::vec3> N(n * R, glm::vec3(0.0f));
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < R; ++j) {
            int j1 = (j + 1) % R;
            glm::vec3 a = P[i * R + j], b = P[i * R + j1];
            glm::vec3 c = P[(i + 1) * R + j], d = P[(i + 1) * R + j1];
            glm::vec3 fn = glm::cross(c - a, b - a) + glm::cross(d - b, c - b);
            glm::vec3 ctr = (a + b + c + d) * 0.25f;
            glm::vec3 rCtr(0.0f, (sec[i].yBot + sec[i].yTop) * 0.5f, ctr.z);
            if (glm::dot(fn, ctr - rCtr) < 0.0f) fn = -fn;
            N[i * R + j] += fn; N[i * R + j1] += fn;
            N[(i + 1) * R + j] += fn; N[(i + 1) * R + j1] += fn;
        }
    }
    for (auto& nn : N) {
        float l = glm::length(nn);
        nn = (l > 1e-6f) ? (nn / l) : glm::vec3(0.0f, 1.0f, 0.0f);
    }

    std::vector<float> v;
    v.reserve((n - 1) * R * 36);
    auto push = [&](const glm::vec3& p, const glm::vec3& nn) {
        v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
        v.push_back(nn.x); v.push_back(nn.y); v.push_back(nn.z);
    };

    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < R; ++j) {
            int j1 = (j + 1) % R;
            push(P[i * R + j], N[i * R + j]);
            push(P[i * R + j1], N[i * R + j1]);
            push(P[(i + 1) * R + j], N[(i + 1) * R + j]);

            push(P[i * R + j1], N[i * R + j1]);
            push(P[(i + 1) * R + j1], N[(i + 1) * R + j1]);
            push(P[(i + 1) * R + j], N[(i + 1) * R + j]);
        }
    }

    // Торцевые крышки
    for (int cap = 0; cap < 2; ++cap) {
        int i = (cap == 0) ? 0 : n - 1;
        glm::vec3 ctr(0.0f);
        for (int j = 0; j < R; ++j) ctr += P[i * R + j];
        ctr /= (float)R;
        glm::vec3 nrm(0.0f, 0.0f, (cap == 0) ? -1.0f : 1.0f);
        for (int j = 0; j < R; ++j) {
            int j1 = (j + 1) % R;
            if (cap == 0) { push(ctr, nrm); push(P[i * R + j1], nrm); push(P[i * R + j], nrm); }
            else          { push(ctr, nrm); push(P[i * R + j], nrm);  push(P[i * R + j1], nrm); }
        }
    }

    Mesh m;
    m.upload(v);
    return m;
}

// 1) Точный массивный наклонный передний обтекатель мотоцикла Канэды (с наклонным носом и боковыми щеками вилки)
Mesh akiraFrontCowl() {
    std::vector<BikeSection> s = {
        {-1.50f, 0.14f, 0.24f, 0.16f, 0.30f, 0.37f, 0.44f},
        {-1.36f, 0.22f, 0.34f, 0.24f, 0.22f, 0.40f, 0.51f},
        {-1.12f, 0.28f, 0.42f, 0.30f, 0.14f, 0.44f, 0.58f},
        {-0.82f, 0.32f, 0.45f, 0.31f, 0.11f, 0.48f, 0.64f},
        {-0.52f, 0.32f, 0.41f, 0.28f, 0.14f, 0.49f, 0.67f},
        {-0.30f, 0.26f, 0.33f, 0.22f, 0.22f, 0.46f, 0.62f}
    };
    return loftSmoothBike(s);
}

// 2) Низкая обтекаемая рама-капсула мотоцикла Акиры (под сиденьем Канэды)
Mesh akiraLowerTub() {
    std::vector<BikeSection> s = {
        {-0.76f, 0.22f, 0.29f, 0.22f, -0.22f, -0.04f, 0.14f},
        {-0.40f, 0.28f, 0.36f, 0.27f, -0.29f, -0.10f, 0.06f},
        { 0.12f, 0.31f, 0.38f, 0.29f, -0.30f, -0.11f, 0.04f},
        { 0.56f, 0.30f, 0.37f, 0.28f, -0.28f, -0.06f, 0.16f},
        { 0.90f, 0.23f, 0.31f, 0.23f, -0.20f,  0.02f, 0.24f}
    };
    return loftSmoothBike(s);
}

// 3) Характерный массивный округлый задний капот-обтекатель за спиной Канэды над задним колесом
Mesh akiraRearCowl() {
    std::vector<BikeSection> s = {
        { 0.46f, 0.29f, 0.37f, 0.26f, 0.14f, 0.46f, 0.68f},
        { 0.72f, 0.32f, 0.41f, 0.30f, 0.18f, 0.53f, 0.74f},
        { 1.04f, 0.30f, 0.39f, 0.28f, 0.26f, 0.52f, 0.70f},
        { 1.32f, 0.25f, 0.33f, 0.22f, 0.35f, 0.48f, 0.59f},
        { 1.54f, 0.16f, 0.23f, 0.13f, 0.36f, 0.42f, 0.47f}
    };
    return loftSmoothBike(s);
}

// 4) Большой выгнутый прозрачный панорамный ветровик-колпак мотоцикла Акиры
Mesh akiraWindscreen() {
    std::vector<float> v;
    const int nz = 12;
    const int na = 12;
    auto pt = [&](float u, float ang) -> glm::vec3 {
        float z = -1.26f + u * 1.02f;
        float arcLift = std::sin(u * 2.10f) * 0.39f + u * 0.08f;
        float baseY = 0.54f + arcLift;
        float halfW = 0.21f + u * 0.11f;
        float domeH = 0.09f + u * 0.09f;
        float x = std::sin(ang) * halfW;
        float y = baseY + std::cos(ang) * domeH - domeH * 0.35f;
        return glm::vec3(x, y, z);
    };

    for (int iz = 0; iz < nz; ++iz) {
        float u0 = (float)iz / (float)nz;
        float u1 = (float)(iz + 1) / (float)nz;
        for (int ia = 0; ia < na; ++ia) {
            float a0 = -1.16f + 2.32f * ((float)ia / (float)na);
            float a1 = -1.16f + 2.32f * ((float)(ia + 1) / (float)na);
            quadTwoSided(v, pt(u0, a0), pt(u0, a1), pt(u1, a1), pt(u1, a0));
        }
    }
    Mesh m;
    m.upload(v);
    return m;
}

// 5) Плавная 3-лучевая звезда-лента для объёмного светового шлейфа фар (0°, 60°, 120° вокруг оси Z)
// Выглядит корректно объёмно и плавно со ВСЕХ ракурсов (сзади, сбоку, сверху, в орбите) — никаких плоских прямоугольников
Mesh lightTrailRibbon() {
    std::vector<float> v;
    const int segs = 22;
    const float angles[3] = {0.0f, 1.0471975f, 2.0943951f}; // 0°, 60°, 120°
    for (int p = 0; p < 3; ++p) {
        float ca = std::cos(angles[p]);
        float sa = std::sin(angles[p]);
        glm::vec3 nrm(-sa, ca, 0.0f);
        for (int i = 0; i < segs; ++i) {
            float z0 = (float)i / (float)segs;
            float z1 = (float)(i + 1) / (float)segs;
            float w0 = 1.0f - z0 * 0.70f;
            float w1 = 1.0f - z1 * 0.70f;
            glm::vec3 a(-ca * w0, -sa * w0, z0);
            glm::vec3 b( ca * w0,  sa * w0, z0);
            glm::vec3 c( ca * w1,  sa * w1, z1);
            glm::vec3 d(-ca * w1, -sa * w1, z1);
            Primitives::quad(v, a, b, c, d, nrm);
            Primitives::quad(v, a, d, c, b, -nrm);
        }
    }
    Mesh m;
    m.upload(v);
    return m;
}

}
