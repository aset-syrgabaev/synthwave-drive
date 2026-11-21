#include "AkiraBike.h"

// Этот файл строит процедурную 3D-геометрию деталей мотоцикла Акиры:
// шины с протектором, ступичные колпаки, капсулу и плоские наклейки на крылья.
namespace AkiraGeo {

// Тело вращения вокруг оси X: берём 2D-профиль (x, радиус) и проворачиваем его на 360 градусов (seg шагов).
// Так делаются все круглые детали (колпаки, половинки таблетки) со сглаженными нормалями.
Mesh revolveX(const std::vector<glm::vec2>& prof, int seg) {
    int n = static_cast<int>(prof.size());
    std::vector<glm::vec3> P(n * seg);
    std::vector<glm::vec3> N(n * seg, glm::vec3(0.0f));
    glm::vec3 cen(0.0f);
    for (int k = 0; k < n; ++k) {
        for (int j = 0; j < seg; ++j) {
            float a = 2.0f * kPi * (float)j / (float)seg;
            P[k * seg + j] = glm::vec3(prof[k].x, prof[k].y * std::cos(a), prof[k].y * std::sin(a));
            cen += P[k * seg + j];
        }
    }
    cen /= (float)(n * seg);
    float orient = 0.0f;
    for (int k = 0; k < n - 1; ++k) {
        for (int j = 0; j < seg; ++j) {
            int j1 = (j + 1) % seg;
            glm::vec3 a = P[k * seg + j], b = P[k * seg + j1];
            glm::vec3 c = P[(k + 1) * seg + j], d = P[(k + 1) * seg + j1];
            glm::vec3 fn = glm::cross(b - a, c - a) + glm::cross(d - b, c - b);
            orient += glm::dot(fn, (a + b + c + d) * 0.25f - cen);
            N[k * seg + j] += fn;       N[k * seg + j1] += fn;
            N[(k + 1) * seg + j] += fn; N[(k + 1) * seg + j1] += fn;
        }
    }
    float flip = (orient < 0.0f) ? -1.0f : 1.0f;
    for (int k = 0; k < n; ++k) {
        for (int j = 0; j < seg; ++j) {
            glm::vec3& nn = N[k * seg + j];
            float l = glm::length(nn);
            if (l > 1e-9f) nn = nn / l * flip;
            else nn = glm::vec3((k == 0) ? -1.0f : 1.0f, 0.0f, 0.0f);
        }
    }
    std::vector<float> v;
    for (int k = 0; k < n - 1; ++k) {
        for (int j = 0; j < seg; ++j) {
            int j1 = (j + 1) % seg;
            int ia = k * seg + j, ib = k * seg + j1, ic = (k + 1) * seg + j, id = (k + 1) * seg + j1;
            Primitives::vert(v, P[ia], N[ia]); Primitives::vert(v, P[ib], N[ib]); Primitives::vert(v, P[ic], N[ic]);
            Primitives::vert(v, P[ib], N[ib]); Primitives::vert(v, P[id], N[id]); Primitives::vert(v, P[ic], N[ic]);
        }
    }
    Mesh m;
    m.upload(v);
    return m;
}

float hubDomeX(float r, float R, float depth) {
    float q = std::min(r / (R * 0.955f), 1.0f);
    return 0.016f + (depth - 0.016f) * (1.0f - q * 0.82f);
}

// Плоско-гранёный ступенчатый передний колпак колеса (без излишней выпуклости)
Mesh hubcapFront(float R, float depth) {
    std::vector<glm::vec2> p = {
        {-0.025f, 0.0f}, {-0.025f, R}, {0.000f, R}, {0.014f, R * 0.96f},
        {0.022f, R * 0.68f}, {0.016f, R * 0.65f}, {0.024f, R * 0.62f},
        {depth * 0.85f, R * 0.28f}, {depth, R * 0.22f}, {depth, 0.0f}
    };
    return revolveX(p, 56);
}

// Ступенчатый металлический диск + крышка ступицы заднего колеса
Mesh hubcapRear(float R, float depth) {
    std::vector<glm::vec2> p = {
        {-0.025f, 0.0f}, {-0.025f, R}, {0.000f, R}, {0.016f, R * 0.95f},
        {0.024f, R * 0.72f}, {0.042f, R * 0.66f}, {depth * 0.88f, R * 0.56f},
        {depth, R * 0.44f}, {depth, 0.0f}
    };
    return revolveX(p, 56);
}

// Выпуклая шина мотоцикла с рельефными блоками протектора и центральной канавкой (суперэллипс по сечению)
Mesh tire(float R, float ri, float W, int seg, int prof) {
    float rc = (R + ri) * 0.5f, h = (R - ri) * 0.5f, hw = W * 0.5f;
    std::vector<glm::vec3> P(prof * seg), Cc(prof * seg), N(prof * seg, glm::vec3(0.0f));
    for (int k = 0; k < prof; ++k) {
        float t = 2.0f * kPi * (float)k / (float)prof;
        float cx = std::cos(t), sy = std::sin(t);
        float px = hw * sgnPow(cx, 2.0f / 3.4f);
        float pr = rc + h * sgnPow(sy, 2.0f / 3.4f);
        float outer = std::pow(std::max(0.0f, sy), 6.0f);
        for (int j = 0; j < seg; ++j) {
            float a = 2.0f * kPi * (float)j / (float)seg;
            float block = smooth01(0.30f, 0.62f, 0.5f + 0.5f * std::sin(a * 36.0f));
            float groove = std::exp(-std::pow(px / (hw * 0.07f), 2.0f));
            float rr = pr + outer * (0.014f * block - 0.012f * groove);
            P[k * seg + j] = glm::vec3(px, rr * std::cos(a), rr * std::sin(a));
            Cc[k * seg + j] = glm::vec3(0.0f, rc * std::cos(a), rc * std::sin(a));
        }
    }
    for (int k = 0; k < prof; ++k) {
        int k1 = (k + 1) % prof;
        for (int j = 0; j < seg; ++j) {
            int j1 = (j + 1) % seg;
            glm::vec3 a = P[k * seg + j], b = P[k * seg + j1];
            glm::vec3 c = P[k1 * seg + j], d = P[k1 * seg + j1];
            glm::vec3 fn = glm::cross(b - a, c - a) + glm::cross(d - b, c - b);
            glm::vec3 ctr = (a + b + c + d) * 0.25f;
            if (glm::dot(fn, ctr - Cc[k * seg + j]) < 0.0f) fn = -fn;
            N[k * seg + j] += fn;  N[k * seg + j1] += fn;
            N[k1 * seg + j] += fn; N[k1 * seg + j1] += fn;
        }
    }
    for (auto& nn : N) {
        float l = glm::length(nn);
        nn = (l > 1e-9f) ? nn / l : glm::vec3(0, 1, 0);
    }
    std::vector<float> v;
    for (int k = 0; k < prof; ++k) {
        int k1 = (k + 1) % prof;
        for (int j = 0; j < seg; ++j) {
            int j1 = (j + 1) % seg;
            int ia = k * seg + j, ib = k * seg + j1, ic = k1 * seg + j, id = k1 * seg + j1;
            Primitives::vert(v, P[ia], N[ia]); Primitives::vert(v, P[ib], N[ib]); Primitives::vert(v, P[ic], N[ic]);
            Primitives::vert(v, P[ib], N[ib]); Primitives::vert(v, P[id], N[id]); Primitives::vert(v, P[ic], N[ic]);
        }
    }
    Mesh m;
    m.upload(v);
    return m;
}

// Половинка декоративной капсулы-таблетки на заднем крыле (цилиндр + полусфера вращением)
Mesh pillHalf(float R, float L) {
    std::vector<glm::vec2> p;
    p.push_back({0.0f, 0.0f});
    p.push_back({0.0f, R});
    p.push_back({L, R});
    const int K = 10;
    for (int k = 1; k <= K; ++k) {
        float phi = (float)k / (float)K * (kPi * 0.5f);
        p.push_back({L + R * std::sin(phi), R * std::cos(phi)});
    }
    return revolveX(p, 32);
}

// Плоский круг для круглых логотипов и наклеек на боковых обтекателях
Mesh flatDisc(int seg) {
    std::vector<float> v;
    glm::vec3 n(0, 0, 1);
    for (int i = 0; i < seg; ++i) {
        float a0 = 2.0f * kPi * (float)i / (float)seg, a1 = 2.0f * kPi * (float)(i + 1) / (float)seg;
        Primitives::vert(v, glm::vec3(0, 0, 0), n);
        Primitives::vert(v, glm::vec3(std::cos(a0), std::sin(a0), 0), n);
        Primitives::vert(v, glm::vec3(std::cos(a1), std::sin(a1), 0), n);
    }
    Mesh m;
    m.upload(v);
    return m;
}

// Плоская пятиконечная звезда для наклейки на мотоцикл
Mesh flatStar(float rInner) {
    std::vector<float> v;
    glm::vec3 n(0, 0, 1);
    for (int i = 0; i < 10; ++i) {
        float a0 = kPi * 0.5f + kPi * 0.2f * (float)i, a1 = kPi * 0.5f + kPi * 0.2f * (float)(i + 1);
        float r0 = (i % 2 == 0) ? 1.0f : rInner, r1 = ((i + 1) % 2 == 0) ? 1.0f : rInner;
        Primitives::vert(v, glm::vec3(0, 0, 0), n);
        Primitives::vert(v, glm::vec3(r0 * std::cos(a0), r0 * std::sin(a0), 0), n);
        Primitives::vert(v, glm::vec3(r1 * std::cos(a1), r1 * std::sin(a1), 0), n);
    }
    Mesh m;
    m.upload(v);
    return m;
}

// Плоский треугольник-декаль
Mesh flatTriangle() {
    std::vector<float> v;
    glm::vec3 n(0, 0, 1);
    for (int i = 0; i < 3; ++i) {
        float a0 = kPi * 0.5f + 2.0f * kPi * (float)i / 3.0f, a1 = kPi * 0.5f + 2.0f * kPi * (float)(i + 1) / 3.0f;
        Primitives::vert(v, glm::vec3(0, 0, 0), n);
        Primitives::vert(v, glm::vec3(std::cos(a0), std::sin(a0), 0), n);
        Primitives::vert(v, glm::vec3(std::cos(a1), std::sin(a1), 0), n);
    }
    Mesh m;
    m.upload(v);
    return m;
}

// Строит 3D-надпись на корпусе (CITIZEN, CANON, SHOEI) из пикселей встроенного шрифта Font5x7
Mesh flatText(const std::string& str) {
    std::vector<float> v;
    glm::vec3 n(0, 0, 1);
    int len = static_cast<int>(str.size());
    float totalW = (float)(len * 6 - 1);
    float x0 = -totalW * 0.5f, y0 = 3.5f;
    for (int ci = 0; ci < len; ++ci) {
        const char* rows = findGlyph(str[ci]);
        if (!rows) continue;
        for (int r = 0; r < 7; ++r) {
            for (int c = 0; c < 5; ++c) {
                if (!glyphPixel(rows, c, r)) continue;
                float px = x0 + (float)(ci * 6 + c);
                float py = y0 - (float)r;
                Primitives::quad(v, glm::vec3(px, py - 1.0f, 0), glm::vec3(px + 1.0f, py - 1.0f, 0),
                                 glm::vec3(px + 1.0f, py, 0), glm::vec3(px, py, 0), n);
            }
        }
    }
    Mesh m;
    m.upload(v);
    return m;
}

// Строит ортонормированную матрицу поворота так, чтобы плоская наклейка легла ровно по нормали поверхности
glm::mat4 faceMatrix(const glm::vec3& pos, const glm::vec3& normalIn, const glm::vec3& upHint) {
    glm::vec3 n = glm::normalize(normalIn);
    glm::vec3 r = glm::cross(upHint, n);
    if (glm::length(r) < 1e-4f) r = glm::cross(glm::vec3(0, 0, 1), n);
    r = glm::normalize(r);
    glm::vec3 u = glm::cross(n, r);
    glm::mat4 m(1.0f);
    m[0] = glm::vec4(r, 0.0f);
    m[1] = glm::vec4(u, 0.0f);
    m[2] = glm::vec4(n, 0.0f);
    m[3] = glm::vec4(pos, 1.0f);
    return m;
}

// Позиционирование наклеек точно на внешней плоской грани переднего крыла-обтекателя (xOutTop)
glm::mat4 wingDecal(float side, float z, float y, float lift) {
    static const float zs[7] = {-1.44f, -1.30f, -1.12f, -0.90f, -0.68f, -0.46f, -0.30f};
    static const float xs[7] = { 0.26f,  0.36f,  0.42f,  0.45f,  0.45f,  0.43f,  0.38f};
    auto evalX = [&](float zz) {
        if (zz <= zs[0]) return xs[0];
        if (zz >= zs[6]) return xs[6];
        for (int i = 0; i < 6; ++i) {
            if (zz <= zs[i + 1]) {
                float t = (zz - zs[i]) / (zs[i + 1] - zs[i]);
                return xs[i] + (xs[i + 1] - xs[i]) * t;
            }
        }
        return xs[6];
    };
    float x = evalX(z);
    float dxdz = (evalX(z + 0.02f) - evalX(z - 0.02f)) / 0.04f;
    glm::vec3 n = glm::normalize(glm::vec3(side, 0.0f, -dxdz));
    return faceMatrix(glm::vec3(side * x, y, z) + n * lift, n);
}

}
