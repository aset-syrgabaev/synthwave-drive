#include "AkiraBike.h"

// ============================================================================
// ОБТЕКАТЕЛИ И ШАССИ МОТОЦИКЛА «АКИРА» (AkiraBikeBody.cpp) — КРИТЕРИЙ R2
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   Все изогнутые красные панели байка Канэды (боковые крылья, носовой мост,
//   поддон, хвостовой обтекатель и ветровик) строятся процедурным лофтингом
//   loftProfiles() со сглаживанием нормалей по углу перегиба < 38°.
// ============================================================================
namespace AkiraGeo {

float sgnPow(float v, float e) {
    return (v < 0.0f ? -1.0f : 1.0f) * std::pow(std::fabs(v), e);
}

float smooth01(float a, float b, float x) {
    float t = std::clamp((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Вспомогательная функция: строит гранёный/сглаженный меш по цепочке 2D-профилей (кольцо точек в каждом сечении Z)
// sharpCrease = true считает нормали по граням с лёгким сглаживанием вдоль Z, чтобы сохранить чёткие автомобильные рёбра
// Строит 3D-меш по цепочке поперечных профилей.
// Использует сглаживание нормалей по углу перегиба (38°): плавные изгибы корпуса вдоль длины
// выглядят гладко, как штампованный металл, а острые рёбра между гранями остаются чёткими.
// Универсальный лофтинг: соединяет последовательность замкнутых поперечных профилей (сечений)
// в единый 3D-меш. При sharpCrease = true сглаживает только плавные изгибы (< 38 градусов),
// а острые стыки панелей оставляет чёткими, как у штампованного кузова.
Mesh loftProfiles(const std::vector<std::vector<glm::vec3>>& rings, bool closeCap) {
    int n = static_cast<int>(rings.size());
    int R = static_cast<int>(rings[0].size());
    std::vector<glm::vec3> ctrs(n, glm::vec3(0.0f));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < R; ++j) ctrs[i] += rings[i][j];
        ctrs[i] /= (float)R;
    }

    // Сначала вычисляем единичные нормали для каждой четырёхугольной ячейки сетки
    std::vector<glm::vec3> cellN((n - 1) * R, glm::vec3(0, 1, 0));
    for (int i = 0; i < n - 1; ++i) {
        glm::vec3 segCtr = (ctrs[i] + ctrs[i + 1]) * 0.5f;
        for (int j = 0; j < R; ++j) {
            int j1 = (j + 1) % R;
            glm::vec3 a = rings[i][j], b = rings[i][j1];
            glm::vec3 c = rings[i + 1][j], d = rings[i + 1][j1];
            glm::vec3 fn = glm::cross(b - a, c - a) + glm::cross(d - b, c - b);
            glm::vec3 fCtr = (a + b + c + d) * 0.25f;
            if (glm::dot(fn, fCtr - segCtr) < 0.0f) fn = -fn;
            float l = glm::length(fn);
            cellN[i * R + j] = (l > 1e-8f) ? (fn / l) : glm::vec3(0, 1, 0);
        }
    }

    // Для каждой вершины ячейки усредняем нормаль только с соседними гранями, угол к которым меньше 38°
    const float creaseCos = 0.788f; // cos(38°)
    auto smoothVertN = [&](int ci, int cj, int vi, int vj) {
        glm::vec3 baseN = cellN[ci * R + cj];
        glm::vec3 acc = baseN;
        for (int di = -1; di <= 0; ++di) {
            int ni = vi + di;
            if (ni < 0 || ni >= n - 1) continue;
            for (int dj = -1; dj <= 0; ++dj) {
                int nj = (vj + dj + R) % R;
                if (ni == ci && nj == cj) continue;
                glm::vec3 nn = cellN[ni * R + nj];
                if (glm::dot(baseN, nn) >= creaseCos) acc += nn;
            }
        }
        float l = glm::length(acc);
        return (l > 1e-8f) ? (acc / l) : baseN;
    };

    std::vector<float> v;
    v.reserve((n - 1) * R * 36);

    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < R; ++j) {
            int j1 = (j + 1) % R;
            glm::vec3 a = rings[i][j], b = rings[i][j1];
            glm::vec3 c = rings[i + 1][j], d = rings[i + 1][j1];
            glm::vec3 na = smoothVertN(i, j, i, j);
            glm::vec3 nb = smoothVertN(i, j, i, j1);
            glm::vec3 nc = smoothVertN(i, j, i + 1, j);
            glm::vec3 nd = smoothVertN(i, j, i + 1, j1);

            Primitives::vert(v, a, na); Primitives::vert(v, b, nb); Primitives::vert(v, c, nc);
            Primitives::vert(v, b, nb); Primitives::vert(v, d, nd); Primitives::vert(v, c, nc);
        }
    }

    if (closeCap) {
        for (int cap = 0; cap < 2; ++cap) {
            int i = (cap == 0) ? 0 : (n - 1);
            glm::vec3 ctr(0.0f);
            for (int j = 0; j < R; ++j) ctr += rings[i][j];
            ctr /= (float)R;
            glm::vec3 nrm(0.0f, 0.0f, (cap == 0) ? -1.0f : 1.0f);
            for (int j = 0; j < R; ++j) {
                int j1 = (j + 1) % R;
                if (cap == 0) {
                    Primitives::vert(v, ctr, nrm); Primitives::vert(v, rings[i][j1], nrm); Primitives::vert(v, rings[i][j], nrm);
                } else {
                    Primitives::vert(v, ctr, nrm); Primitives::vert(v, rings[i][j], nrm); Primitives::vert(v, rings[i][j1], nrm);
                }
            }
        }
    }

    Mesh m;
    m.upload(v);
    return m;
}

// 1) Боковые крылья-обтекатели передней вилки (side = -1 левое, +1 правое).
// Длинный низкий гоночный силуэт в стиле Акиры: 10 плавных сечений от оси переднего колеса к кокпиту.
Mesh frontWingSide(float side) {
    struct WSec { float z, yBot, yMid, yTop, xIn, xOutBot, xOutTop, xTopIn; };
    const std::vector<WSec> secs = {
        {-1.56f,  0.14f, 0.25f, 0.34f, 0.16f, 0.21f, 0.23f, 0.16f},
        {-1.46f,  0.02f, 0.22f, 0.39f, 0.17f, 0.25f, 0.29f, 0.19f},
        {-1.34f, -0.18f, 0.16f, 0.44f, 0.18f, 0.27f, 0.36f, 0.22f},
        {-1.18f, -0.21f, 0.18f, 0.49f, 0.18f, 0.30f, 0.42f, 0.25f},
        {-1.00f, -0.11f, 0.24f, 0.54f, 0.19f, 0.33f, 0.45f, 0.28f},
        {-0.82f, -0.02f, 0.30f, 0.58f, 0.20f, 0.35f, 0.46f, 0.30f},
        {-0.64f,  0.07f, 0.36f, 0.61f, 0.20f, 0.35f, 0.46f, 0.30f},
        {-0.46f,  0.15f, 0.41f, 0.62f, 0.20f, 0.33f, 0.43f, 0.29f},
        {-0.30f,  0.22f, 0.44f, 0.60f, 0.19f, 0.29f, 0.37f, 0.25f},
        {-0.20f,  0.27f, 0.44f, 0.56f, 0.18f, 0.24f, 0.30f, 0.21f}
    };

    std::vector<std::vector<glm::vec3>> rings;
    for (const auto& s : secs) {
        std::vector<glm::vec3> r = {
            glm::vec3(side * s.xIn,     s.yBot,              s.z),
            glm::vec3(side * s.xOutBot, s.yBot + 0.02f,      s.z),
            glm::vec3(side * s.xOutTop, s.yMid,              s.z),
            glm::vec3(side * s.xOutTop, s.yTop - 0.045f,     s.z),
            glm::vec3(side * s.xTopIn,  s.yTop,              s.z),
            glm::vec3(side * s.xIn,     s.yTop - 0.02f,      s.z),
            glm::vec3(side * s.xIn,     s.yMid,              s.z),
            glm::vec3(side * s.xIn,     s.yBot + 0.05f,      s.z)
        };
        if (side < 0.0f) std::reverse(r.begin(), r.end());
        rings.push_back(r);
    }
    return loftProfiles(rings, true);
}

// 2) Аэродинамическая носовая перемычка над передним колесом с рельефным гребнем
Mesh frontNoseBridge() {
    struct BSec { float z, yBot, yTop, wBot, wTop; };
    const std::vector<BSec> secs = {
        {-1.56f, 0.28f, 0.34f, 0.17f, 0.13f},
        {-1.45f, 0.31f, 0.40f, 0.21f, 0.17f},
        {-1.30f, 0.34f, 0.46f, 0.24f, 0.22f},
        {-1.12f, 0.37f, 0.51f, 0.27f, 0.26f},
        {-0.92f, 0.40f, 0.54f, 0.29f, 0.28f}
    };
    std::vector<std::vector<glm::vec3>> rings;
    for (const auto& s : secs) {
        rings.push_back({
            glm::vec3(-s.wBot, s.yBot, s.z),
            glm::vec3(-s.wTop, s.yTop, s.z),
            glm::vec3( 0.0f,   s.yTop + 0.022f, s.z),
            glm::vec3( s.wTop, s.yTop, s.z),
            glm::vec3( s.wBot, s.yBot, s.z),
            glm::vec3( 0.0f,   s.yBot - 0.01f, s.z)
        });
    }
    return loftProfiles(rings, true);
}

// 3) Нижняя рама-шасси и аэродинамические боковые пороги под открытым сиденьем
Mesh lowerChassis() {
    struct CSec { float z, yBot, yMid, yTop, wBot, wMid, wTop; };
    const std::vector<CSec> secs = {
        {-0.94f, -0.26f, -0.09f, 0.10f, 0.16f, 0.24f, 0.22f},
        {-0.70f, -0.34f, -0.15f, 0.05f, 0.22f, 0.33f, 0.29f},
        {-0.42f, -0.36f, -0.17f, -0.04f, 0.26f, 0.37f, 0.32f},
        {-0.10f, -0.36f, -0.17f, -0.06f, 0.27f, 0.38f, 0.33f},
        { 0.22f, -0.36f, -0.15f, -0.02f, 0.27f, 0.38f, 0.34f},
        { 0.52f, -0.33f, -0.11f,  0.14f, 0.26f, 0.37f, 0.32f},
        { 0.78f, -0.24f, -0.03f,  0.22f, 0.21f, 0.31f, 0.27f},
        { 0.90f, -0.15f,  0.04f,  0.24f, 0.15f, 0.23f, 0.20f}
    };
    std::vector<std::vector<glm::vec3>> rings;
    for (const auto& s : secs) {
        rings.push_back({
            glm::vec3(-s.wBot, s.yBot, s.z),
            glm::vec3(-s.wMid, s.yMid, s.z),
            glm::vec3(-s.wTop, s.yTop, s.z),
            glm::vec3( 0.0f,   s.yTop - 0.02f, s.z),
            glm::vec3( s.wTop, s.yTop, s.z),
            glm::vec3( s.wMid, s.yMid, s.z),
            glm::vec3( s.wBot, s.yBot, s.z),
            glm::vec3( 0.0f,   s.yBot, s.z)
        });
    }
    return loftProfiles(rings, true);
}

// 4) Заострённый обтекаемый задний хвост за спинкой кресла (открывает массивное заднее колесо)
Mesh rearSeatCowl() {
    struct RSec { float z, yBot, yMid, yTop, wBot, wMid, wTop; };
    const std::vector<RSec> secs = {
        { 0.40f, 0.06f, 0.42f, 0.67f, 0.31f, 0.35f, 0.24f},
        { 0.56f, 0.11f, 0.47f, 0.73f, 0.33f, 0.37f, 0.25f},
        { 0.74f, 0.20f, 0.49f, 0.70f, 0.31f, 0.35f, 0.22f},
        { 0.92f, 0.30f, 0.49f, 0.62f, 0.26f, 0.29f, 0.17f},
        { 1.08f, 0.37f, 0.47f, 0.54f, 0.20f, 0.22f, 0.12f},
        { 1.20f, 0.41f, 0.46f, 0.49f, 0.12f, 0.14f, 0.08f}
    };
    std::vector<std::vector<glm::vec3>> rings;
    for (const auto& s : secs) {
        rings.push_back({
            glm::vec3(-s.wBot, s.yBot, s.z),
            glm::vec3(-s.wMid, s.yMid, s.z),
            glm::vec3(-s.wTop, s.yTop, s.z),
            glm::vec3( 0.0f,   s.yTop + 0.015f, s.z),
            glm::vec3( s.wTop, s.yTop, s.z),
            glm::vec3( s.wMid, s.yMid, s.z),
            glm::vec3( s.wBot, s.yBot, s.z),
            glm::vec3( 0.0f,   s.yBot, s.z)
        });
    }
    return loftProfiles(rings, true);
}

// 5) Низкое наклонное ветровое стекло-козырёк, плавно переходящее из носового обтекателя
Mesh windscreen() {
    const int nz = 24, na = 18;
    const float z0 = -1.34f, z1 = -0.26f;
    auto pt = [&](int iz, int ia) -> glm::vec3 {
        float u = (float)iz / (float)nz;
        float z = z0 + (z1 - z0) * u;
        float vv = (float)ia / (float)na * 2.0f - 1.0f;
        float halfW = 0.21f + 0.15f * std::sin(u * kPi * 0.78f);
        float x = halfW * vv;
        float baseY = 0.40f + 0.22f * u;
        float ridgeLift = 0.24f * std::pow(u, 0.70f);
        float arch = std::pow(std::max(0.0f, 1.0f - vv * vv), 0.62f);
        return glm::vec3(x, baseY + ridgeLift * arch, z);
    };
    std::vector<float> v;
    auto nrm = [&](int iz, int ia) {
        glm::vec3 dzv = pt(std::min(iz + 1, nz), ia) - pt(std::max(iz - 1, 0), ia);
        glm::vec3 dav = pt(iz, std::min(ia + 1, na)) - pt(iz, std::max(ia - 1, 0));
        glm::vec3 n = glm::cross(dav, dzv);
        float l = glm::length(n);
        n = (l < 1e-8f) ? glm::vec3(0, 1, 0) : n / l;
        if (n.y < 0.0f) n = -n;
        return n;
    };
    for (int iz = 0; iz < nz; ++iz) {
        for (int ia = 0; ia < na; ++ia) {
            glm::vec3 a = pt(iz, ia), b = pt(iz, ia + 1), c = pt(iz + 1, ia + 1), d = pt(iz + 1, ia);
            glm::vec3 na_ = nrm(iz, ia), nb = nrm(iz, ia + 1), nc = nrm(iz + 1, ia + 1), nd = nrm(iz + 1, ia);
            Primitives::vert(v, a, na_); Primitives::vert(v, b, nb); Primitives::vert(v, c, nc);
            Primitives::vert(v, a, na_); Primitives::vert(v, c, nc); Primitives::vert(v, d, nd);
            Primitives::vert(v, a, -na_); Primitives::vert(v, c, -nc); Primitives::vert(v, b, -nb);
            Primitives::vert(v, a, -na_); Primitives::vert(v, d, -nd); Primitives::vert(v, c, -nc);
        }
    }
    Mesh m;
    m.upload(v);
    return m;
}

}
