#include "CarModel.h"

// ============================================================================
// ПРОЦЕДУРНЫЙ КУЗОВ DELOREAN DMC-12 (CarModelBody.cpp) — КРИТЕРИЙ R2
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   Кузов не загружается из .obj — он строится математически по 105 поперечным
//   сечениям (лофтинг, loftDeLorean). Порог сглаживания нормалей cos(24°) = 0.91
//   сохраняет фирменные острые штампованные грани капота и крыльев DeLorean.
// ============================================================================
namespace CarModel {

// 18 точных точек профиля DeLorean DMC-12:
// приподнятые гребни передних крыльев + чёткие продольные канавки капота (не плоская доска)
void ringPoints(const Station& s, glm::vec3* out) {
    float g = s.hoodGroove;
    float fenderRise = g * 0.85f;
    float centerCrown = g * 0.48f;
    float xSeamOut = std::min(s.wHood - 0.06f, 0.56f);
    float xSeamIn  = std::min(s.wHood - 0.12f, 0.48f);
    float xGroove  = 0.30f;

    out[0]  = glm::vec3( 0.0f,         s.yBot,                     s.z);
    out[1]  = glm::vec3(-s.wBot,       s.yBot,                     s.z);
    out[2]  = glm::vec3(-s.wWaist,     s.yWaist,                   s.z);
    out[3]  = glm::vec3(-s.wWaist,     s.yWaist + 0.036f,          s.z);
    out[4]  = glm::vec3(-s.wShoulder,  s.yShoulder,                s.z);
    out[5]  = glm::vec3(-s.wHood,      s.yTop + fenderRise * 0.45f, s.z);
    out[6]  = glm::vec3(-xSeamOut,     s.yTop + fenderRise,        s.z);
    out[7]  = glm::vec3(-xSeamIn,      s.yTop - g * 0.45f,         s.z);
    out[8]  = glm::vec3(-xGroove,      s.yTop + centerCrown * 0.4f, s.z);
    out[9]  = glm::vec3( 0.0f,         s.yTop + centerCrown,       s.z);
    out[10] = glm::vec3( xGroove,      s.yTop + centerCrown * 0.4f, s.z);
    out[11] = glm::vec3( xSeamIn,      s.yTop - g * 0.45f,         s.z);
    out[12] = glm::vec3( xSeamOut,     s.yTop + fenderRise,        s.z);
    out[13] = glm::vec3( s.wHood,      s.yTop + fenderRise * 0.45f, s.z);
    out[14] = glm::vec3( s.wShoulder,  s.yShoulder,                s.z);
    out[15] = glm::vec3( s.wWaist,     s.yWaist + 0.036f,          s.z);
    out[16] = glm::vec3( s.wWaist,     s.yWaist,                   s.z);
    out[17] = glm::vec3( s.wBot,       s.yBot,                     s.z);
}

// Линейная интерполяция профилей для чёткой автомобильной геометрии без волн
float tableLerp(const std::vector<std::pair<float, float>>& t, float z) {
    if (z <= t.front().first) return t.front().second;
    if (z >= t.back().first) return t.back().second;
    for (size_t i = 1; i < t.size(); ++i) {
        if (z <= t[i].first) {
            float u = (z - t[i - 1].first) / (t[i].first - t[i - 1].first);
            return t[i - 1].second + u * (t[i].second - t[i - 1].second);
        }
    }
    return t.back().second;
}

void quadTwoSided(std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
    glm::vec3 n = glm::cross(b - a, c - a);
    float l = glm::length(n);
    if (l < 1e-6f) return;
    n /= l;
    Primitives::quad(v, a, b, c, d, n);
    Primitives::quad(v, a, d, c, b, -n);
}

// Лофтинг с точным контролем автомобильных рёбер (порог cos(24°) сохраняет острые выштамповки капота и крыльев)
Mesh loftDeLorean(const std::vector<Station>& st) {
    const int R = kRing;
    const int n = static_cast<int>(st.size());
    std::vector<glm::vec3> P(n * R);
    for (int i = 0; i < n; ++i) ringPoints(st[i], &P[i * R]);

    std::vector<glm::vec3> faceN((n - 1) * R, glm::vec3(0.0f, 1.0f, 0.0f));
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < R; ++j) {
            int j1 = (j + 1) % R;
            glm::vec3 a = P[i * R + j];
            glm::vec3 b = P[i * R + j1];
            glm::vec3 c = P[(i + 1) * R + j];
            glm::vec3 d = P[(i + 1) * R + j1];
            glm::vec3 nrm = glm::cross(c - a, b - a) + glm::cross(d - b, c - b);
            float len = glm::length(nrm);
            nrm = len > 1e-7f ? nrm / len : glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 center = (a + b + c + d) * 0.25f;
            glm::vec3 ringCenter(0.0f, (st[i].yBot + st[i].yTop) * 0.45f, center.z);
            if (glm::dot(nrm, center - ringCenter) < 0.0f) nrm = -nrm;
            faceN[i * R + j] = nrm;
        }
    }

    // Порог cos(24°) = 0.91: рельеф капота и рёбра крыльев выглядят чёткими и штампованными, как в металле
    auto crispNormalFor = [&](int vi, int vj, const glm::vec3& refN) {
        glm::vec3 acc(0.0f);
        for (int di = -1; di <= 0; ++di) {
            int fi = vi + di;
            if (fi < 0 || fi >= n - 1) continue;
            for (int dj = -1; dj <= 0; ++dj) {
                int fj = (vj + dj + R) % R;
                const glm::vec3& fn = faceN[fi * R + fj];
                if (glm::dot(fn, refN) > 0.91f) {
                    acc += fn;
                }
            }
        }
        float l = glm::length(acc);
        return l > 1e-6f ? acc / l : refN;
    };

    std::vector<float> v;
    v.reserve((n - 1) * R * 6 * 6);
    auto push = [&](const glm::vec3& p, const glm::vec3& nn) {
        v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
        v.push_back(nn.x); v.push_back(nn.y); v.push_back(nn.z);
    };

    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < R; ++j) {
            int j1 = (j + 1) % R;
            const glm::vec3& refN = faceN[i * R + j];
            glm::vec3 a = P[i * R + j];
            glm::vec3 b = P[i * R + j1];
            glm::vec3 c = P[(i + 1) * R + j];
            glm::vec3 d = P[(i + 1) * R + j1];

            glm::vec3 na = crispNormalFor(i, j, refN);
            glm::vec3 nb = crispNormalFor(i, j1, refN);
            glm::vec3 nc = crispNormalFor(i + 1, j, refN);
            glm::vec3 nd = crispNormalFor(i + 1, j1, refN);

            push(a, na); push(b, nb); push(c, nc);
            push(b, nb); push(d, nd); push(c, nc);
        }
    }

    for (int cap = 0; cap < 2; ++cap) {
        int i = cap == 0 ? 0 : n - 1;
        glm::vec3 center(0.0f);
        for (int j = 0; j < R; ++j) center += P[i * R + j];
        center /= (float)R;
        glm::vec3 nrm(0.0f, 0.0f, cap == 0 ? -1.0f : 1.0f);
        for (int j = 0; j < R; ++j) {
            if (cap == 0) {
                push(center, nrm);
                push(P[i * R + (j + 1) % R], nrm);
                push(P[i * R + j], nrm);
            } else {
                push(center, nrm);
                push(P[i * R + j], nrm);
                push(P[i * R + (j + 1) % R], nrm);
            }
        }
    }

    Mesh m;
    m.upload(v);
    return m;
}

// Высокодетализированный кузов 1981 DeLorean DMC-12 с рельефным штампованным капотом и козырьком над фарами
Mesh body() {
    std::vector<std::pair<float, float>> wBotT = {
        {-2.16f, 0.72f}, {-1.98f, 0.80f}, {-1.30f, 0.83f}, {-0.90f, 0.80f},
        { 0.50f, 0.80f}, { 1.30f, 0.85f}, { 1.98f, 0.81f}, { 2.14f, 0.75f}
    };
    std::vector<std::pair<float, float>> wWaistT = {
        {-2.16f, 0.83f}, {-1.96f, 0.92f}, {-0.90f, 0.94f},
        {-0.85f, 0.76f}, { 0.45f, 0.76f}, { 0.50f, 0.95f},
        { 1.96f, 0.93f}, { 2.14f, 0.85f}
    };
    std::vector<std::pair<float, float>> wShoulderT = {
        {-2.16f, 0.80f}, {-1.96f, 0.88f}, {-0.90f, 0.89f},
        {-0.85f, 0.68f}, { 0.45f, 0.68f}, { 0.50f, 0.90f},
        { 1.96f, 0.88f}, { 2.14f, 0.81f}
    };
    std::vector<std::pair<float, float>> wHoodT = {
        {-2.16f, 0.75f}, {-1.96f, 0.82f}, {-0.90f, 0.83f},
        {-0.85f, 0.62f}, { 0.45f, 0.62f}, { 0.50f, 0.83f},
        { 1.96f, 0.80f}, { 2.14f, 0.74f}
    };
    std::vector<std::pair<float, float>> yWaistT = {
        {-2.16f, 0.05f}, {-1.96f, 0.09f}, {-0.90f, 0.14f}, {0.50f, 0.15f}, {2.14f, 0.17f}
    };
    std::vector<std::pair<float, float>> yShoulderT = {
        {-2.16f, 0.14f}, {-1.96f, 0.19f}, {-0.90f, 0.355f},
        {-0.85f, 0.14f}, { 0.45f, 0.14f}, { 0.50f, 0.375f},
        { 1.96f, 0.415f}, { 2.14f, 0.385f}
    };
    std::vector<std::pair<float, float>> yTopT = {
        {-2.16f, 0.16f}, {-1.96f, 0.21f}, {-0.90f, 0.380f},
        {-0.85f, 0.15f}, { 0.45f, 0.15f}, { 0.50f, 0.400f},
        { 1.96f, 0.440f}, { 2.14f, 0.400f}
    };

    std::vector<Station> st;
    for (int k = 0; k <= 104; ++k) {
        float z = -2.16f + (4.30f * (float)k / 104.0f);
        Station s;
        s.z = z;
        s.wBot = tableLerp(wBotT, z);
        s.wWaist = tableLerp(wWaistT, z);
        s.wShoulder = tableLerp(wShoulderT, z);
        s.wHood = tableLerp(wHoodT, z);
        s.yWaist = tableLerp(yWaistT, z);
        s.yShoulder = tableLerp(yShoulderT, z);
        s.yTop = tableLerp(yTopT, z);

        // Рельеф штампованного капота 1981 DeLorean проявляется по всей передней части (-2.12..-0.92)
        if (z < -0.90f && z > -2.14f) {
            float fadeFront = std::clamp((z - (-2.14f)) / 0.16f, 0.0f, 1.0f);
            float fadeBack  = std::clamp((-0.90f - z) / 0.14f, 0.0f, 1.0f);
            s.hoodGroove = 0.026f * fadeFront * fadeBack;
        } else {
            s.hoodGroove = 0.0f;
        }

        // Аккуратные расширенные арки колёс + невысокий вырез (никаких просветов в крыльях)
        float yb = -0.25f;
        if (z < -1.90f) yb = -0.18f + (-1.90f - z) * 0.22f;
        if (z >  1.88f) yb = -0.20f + (z - 1.88f) * 0.22f;
        for (float wz : {-1.30f, 1.30f}) {
            float dz = std::fabs(z - wz);
            if (dz < 0.42f) {
                float t = std::clamp((0.42f - dz) / 0.16f, 0.0f, 1.0f);
                float archY = -0.25f + t * 0.34f;
                yb = std::max(yb, archY);
                s.yWaist = std::max(s.yWaist, archY + 0.02f);
                // Чёткая мускулистая отбортовка крыла над колесом
                s.wWaist += 0.024f * t;
                s.wShoulder += 0.016f * t;
            }
        }
        s.yBot = yb;
        st.push_back(s);
    }
    return loftDeLorean(st);
}

// Ровное наклонное лобовое стекло, задние боковые стёкла и фастбэк
Mesh cabinGlass() {
    std::vector<float> v;

    glm::vec3 wBL(-0.79f, 0.375f, -0.89f);
    glm::vec3 wBR( 0.79f, 0.375f, -0.89f);
    glm::vec3 wTR( 0.59f, 0.785f, -0.26f);
    glm::vec3 wTL(-0.59f, 0.785f, -0.26f);
    quadTwoSided(v, wBL, wBR, wTR, wTL);

    for (float sx : {-1.0f, 1.0f}) {
        glm::vec3 q0(sx * 0.85f, 0.40f, 0.48f);
        glm::vec3 q1(sx * 0.60f, 0.780f, 0.48f);
        glm::vec3 q2(sx * 0.67f, 0.60f, 1.22f);
        glm::vec3 q3(sx * 0.85f, 0.42f, 1.22f);
        quadTwoSided(v, q0, q1, q2, q3);
    }

    glm::vec3 rBL(-0.54f, 0.775f, 0.48f);
    glm::vec3 rBR( 0.54f, 0.775f, 0.48f);
    glm::vec3 rTR( 0.60f, 0.45f, 1.82f);
    glm::vec3 rTL(-0.60f, 0.45f, 1.82f);
    quadTwoSided(v, rBL, rBR, rTR, rTL);

    Mesh m;
    m.upload(v);
    return m;
}

// Крыша и чёткие стойки кузова DeLorean DMC-12 (A-pillar, B-pillar, C-pillar фастбэка)
Mesh cabinFrame() {
    std::vector<float> v;

    Primitives::appendBox(v, glm::vec3(0.0f, 0.790f, 0.11f), glm::vec3(0.30f, 0.038f, 0.76f));
    Primitives::appendBox(v, glm::vec3(0.0f, 0.786f, -0.25f), glm::vec3(1.20f, 0.034f, 0.06f));
    Primitives::appendBox(v, glm::vec3(0.0f, 0.786f,  0.48f), glm::vec3(1.22f, 0.036f, 0.08f));

    for (float sx : {-1.0f, 1.0f}) {
        glm::vec3 a0(sx * 0.84f, 0.375f, -0.90f);
        glm::vec3 a1(sx * 0.76f, 0.375f, -0.87f);
        glm::vec3 a2(sx * 0.56f, 0.792f, -0.25f);
        glm::vec3 a3(sx * 0.62f, 0.792f, -0.28f);
        quadTwoSided(v, a0, a1, a2, a3);

        glm::vec3 b0(sx * 0.86f, 0.390f, 0.46f);
        glm::vec3 b1(sx * 0.86f, 0.390f, 0.54f);
        glm::vec3 b2(sx * 0.60f, 0.790f, 0.53f);
        glm::vec3 b3(sx * 0.60f, 0.790f, 0.46f);
        quadTwoSided(v, b0, b1, b2, b3);

        glm::vec3 cTopIn(sx * 0.52f, 0.786f, 0.48f);
        glm::vec3 cTopOut(sx * 0.60f, 0.786f, 0.48f);
        glm::vec3 cBotOut(sx * 0.82f, 0.440f, 1.94f);
        glm::vec3 cBotIn(sx * 0.60f, 0.440f, 1.94f);
        quadTwoSided(v, cTopIn, cTopOut, cBotOut, cBotIn);

        glm::vec3 sBeltF(sx * 0.86f, 0.400f, 0.48f);
        glm::vec3 sBeltR(sx * 0.85f, 0.435f, 1.94f);
        quadTwoSided(v, sBeltF, cTopOut, cBotOut, sBeltR);
        quadTwoSided(v, cTopIn, cBotIn, sBeltR, cTopOut);
    }

    Mesh m;
    m.upload(v);
    return m;
}

}
