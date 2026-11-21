#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <string>
#include <vector>
#include "GLHeaders.h"
#include <glm/glm.hpp>
#include "Shader.h"
#include "Font5x7.h"

// Отрисовщик 2D-интерфейса (меню, счёт, таймеры, настройки, справка).
// При старте собирает все буквы 5x7 в одну текстуру-атлас, а каждый кадр накапливает
// прямоугольники и символы в один общий массив batch и выводит их за 1 вызов glDrawArrays.
class TextRenderer {
private:
    static constexpr int kCols = 16; // число столбцов ячеек в атласе
    static constexpr int kRows = 6;  // число строк ячеек (16 * 6 = 96 символов ASCII 32..127)
    static constexpr int kCell = 8;  // размер одной ячейки в пикселях

    Shader shader;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int tex = 0;       // текстура-атлас шрифта
    std::vector<float> batch;   // накопленные вершины всех 2D-элементов за текущий кадр
    float screenW = 1.0f;
    float screenH = 1.0f;

    // Генерирует в памяти одноканальную текстуру GL_R8 со всеми символами из Font5x7.h.
    // Последняя ячейка (код 127) полностью белая — она используется для заливки обычных прямоугольников.
    void buildAtlas() {
        const int w = kCols * kCell;
        const int h = kRows * kCell;
        std::vector<unsigned char> px(w * h, 0);
        for (int c = 32; c <= 127; ++c) {
            int idx = c - 32;
            int cx = idx % kCols;
            int cy = idx / kCols;
            if (c == 127) {
                for (int y = 0; y < kCell; ++y)
                    for (int x = 0; x < kCell; ++x) px[(cy * kCell + y) * w + cx * kCell + x] = 255;
                continue;
            }
            const char* g = findGlyph(static_cast<char>(c));
            if (!g) continue;
            for (int r = 0; r < 7; ++r)
                for (int col = 0; col < 5; ++col)
                    if (glyphPixel(g, col, r)) px[(cy * kCell + r) * w + cx * kCell + 1 + col] = 255;
        }
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, px.data());
        // GL_NEAREST сохраняет чёткие пиксельные края букв без размытия
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Добавляет в буфер batch один прямоугольник (2 треугольника = 6 вершин по 8 float)
    void pushQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1, const glm::vec4& c) {
        float q[6][8] = {
            {x, y, u0, v0, c.r, c.g, c.b, c.a}, {x + w, y, u1, v0, c.r, c.g, c.b, c.a}, {x + w, y + h, u1, v1, c.r, c.g, c.b, c.a},
            {x, y, u0, v0, c.r, c.g, c.b, c.a}, {x + w, y + h, u1, v1, c.r, c.g, c.b, c.a}, {x, y + h, u0, v1, c.r, c.g, c.b, c.a}
        };
        for (auto& vtx : q) for (float f : vtx) batch.push_back(f);
    }

public:
    TextRenderer() : shader("shaders/text.vert", "shaders/text.frag") {
        buildAtlas();
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // Формат вершины: x, y (2) + u, v (2) + r, g, b, a (4) = 8 float
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    // Начинаем сборку 2D-элементов нового кадра (запоминаем текущие размеры окна)
    void begin(int w, int h) {
        screenW = (float)w;
        screenH = (float)h;
        batch.clear();
    }

    // Рисует закрашенный прямоугольник (фон панелей, кнопок, полосок таймеров)
    void rect(float x, float y, float w, float h, const glm::vec4& color) {
        float u = (15.0f * kCell + 4.0f) / (float)(kCols * kCell);
        float v = (5.0f * kCell + 4.0f) / (float)(kRows * kCell);
        pushQuad(x, y, w, h, u, v, u, v, color);
    }

    // Рисует контурную рамку толщиной t из четырёх прямоугольников
    void frame(float x, float y, float w, float h, float t, const glm::vec4& color) {
        rect(x, y, w, t, color);
        rect(x, y + h - t, w, t, color);
        rect(x, y, t, h, color);
        rect(x + w - t, y, t, h, color);
    }

    // Считает ширину строки в пикселях при заданном масштабе scale (нужно для центрирования)
    float width(const std::string& s, float scale) const {
        return s.empty() ? 0.0f : (float)s.size() * 6.0f * scale - scale;
    }

    // Добавляет в батч все символы строки s, начиная с точки (x, y)
    void text(const std::string& s, float x, float y, float scale, const glm::vec4& color) {
        float cursor = x;
        for (char ch : s) {
            const char* g = findGlyph(ch);
            if (g || ch == ' ') {
                if (g) {
                    int idx = static_cast<unsigned char>(ch) - 32;
                    char up = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                    idx = static_cast<unsigned char>(up) - 32;
                    if (ch == 'x') idx = 'x' - 32;
                    int cx = idx % kCols;
                    int cy = idx / kCols;
                    float u0 = (cx * kCell + 1.0f) / (float)(kCols * kCell);
                    float u1 = (cx * kCell + 6.0f) / (float)(kCols * kCell);
                    float v0 = (cy * kCell) / (float)(kRows * kCell);
                    float v1 = (cy * kCell + 7.0f) / (float)(kRows * kCell);
                    pushQuad(cursor, y, 5.0f * scale, 7.0f * scale, u0, v0, u1, v1, color);
                }
            }
            cursor += 6.0f * scale;
        }
    }

    // Выводит текст с небольшой тенью для лучшей читаемости
    void textShadow(const std::string& s, float x, float y, float scale, const glm::vec4& color, const glm::vec4& shadow) {
        text(s, x + scale, y + scale, scale, shadow);
        text(s, x, y, scale, color);
    }

    // Выводит текст по центру относительно координаты cx
    void textCentered(const std::string& s, float cx, float y, float scale, const glm::vec4& color, const glm::vec4& shadow) {
        textShadow(s, cx - width(s, scale) * 0.5f, y, scale, color, shadow);
    }

    // Отправляет весь накопленный 2D-интерфейс на видеокарту и рисует его поверх 3D-сцены
    void flush() {
        if (batch.empty()) return;
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader.use();
        shader.setVec2("screen", glm::vec2(screenW, screenH));
        shader.setInt("atlas", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, batch.size() * sizeof(float), batch.data(), GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(batch.size() / 8));
        glBindVertexArray(0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        batch.clear();
    }
};

#endif
