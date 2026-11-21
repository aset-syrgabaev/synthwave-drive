#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#elif __has_include(<GL/glew.h>)
#include <GL/glew.h>
#elif __has_include(<glad/glad.h>)
#include <glad/glad.h>
#endif

#include <glm/glm.hpp>
#include "Shader.h"

// ============================================================================
// ПОСТ-ОБРАБОТКА КАДРА ЧЕРЕЗ FBO (PostProcess.h) — STRETCH GOALS #2 (Bloom) И #3 (CRT)
// ВАЖНО ДЛЯ ЗАЩИТЫ — КАК ЭТО РАБОТАЕТ В 3 ШАГА:
//   1) begin(): переключаем вывод OpenGL не на экран, а в оффскрин-буфер msaaFBO
//      формата GL_RGBA16F (16-битный HDR float, сохраняет яркость > 1.0 для свечения) с 4x MSAA.
//   2) end(): функция glBlitFramebuffer сглаживает 4x MSAA и копирует картинку в 2D-текстуру resolveFBO.
//   3) render(): рисуем прямоугольник на весь экран (Screen Quad) и в postprocess.frag
//      добавляем двухрадиусный Bloom, ACES Tonemapping и ретро CRT-сканлайны (клавиша C).
// ============================================================================
class PostProcess {
private:
    unsigned int msaaFBO = 0;            // буфер кадра с аппаратным сглаживанием 4x MSAA
    unsigned int msaaColorRBO = 0;       // цветовой рендер-буфер (16-битный float HDR)
    unsigned int msaaDepthRBO = 0;       // буфер глубины и трафарета с 4x MSAA

    unsigned int resolveFBO = 0;         // промежуточный FBO для сглаженной 2D-текстуры
    unsigned int textureColorBuffer = 0; // итоговая текстура сцены, которую читает шейдер
    unsigned int quadVAO = 0, quadVBO = 0;
    int width, height;
    int samples = 4;                     // число сэмплов сглаживания MSAA

    // Создаёт оба FBO и прикрепляет к ним рендер-буферы и текстуру
    void createTargets() {
        glGenFramebuffers(1, &msaaFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

        glGenRenderbuffers(1, &msaaColorRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, msaaColorRBO);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA16F, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorRBO);

        glGenRenderbuffers(1, &msaaDepthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthRBO);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaDepthRBO);

        // Если старая видеокарта не поддерживает 16-битный HDR MSAA, переключаемся на обычный RGBA8
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glBindRenderbuffer(GL_RENDERBUFFER, msaaColorRBO);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
        }

        glGenFramebuffers(1, &resolveFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);

        glGenTextures(1, &textureColorBuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Освобождает ресурсы видеопамяти перед пересозданием буферов (например, при ресайзе окна)
    void destroyTargets() {
        if (msaaFBO) glDeleteFramebuffers(1, &msaaFBO);
        if (msaaColorRBO) glDeleteRenderbuffers(1, &msaaColorRBO);
        if (msaaDepthRBO) glDeleteRenderbuffers(1, &msaaDepthRBO);
        if (resolveFBO) glDeleteFramebuffers(1, &resolveFBO);
        if (textureColorBuffer) glDeleteTextures(1, &textureColorBuffer);
        msaaFBO = msaaColorRBO = msaaDepthRBO = resolveFBO = textureColorBuffer = 0;
    }

public:
    bool enabled;        // включает ретро-фильтр CRT (клавиша C)
    float bloomStrength; // сила свечения ярких объектов Bloom (меняется в настройках G)

    PostProcess(int w, int h) : width(w), height(h), enabled(false), bloomStrength(0.24f) {
        createTargets();
        initQuad();
    }

    ~PostProcess() {
        destroyTargets();
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }

    // Создаёт прямоугольник на весь экран для финального вывода текстуры
    void initQuad() {
        float quadVertices[] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    // Вызывается из framebuffer_size_callback при изменении размера окна
    void resize(int w, int h) {
        if (w <= 0 || h <= 0) return;
        width = w;
        height = h;
        destroyTargets();
        createTargets();
    }

    // Шаг 1: переключаем вывод OpenGL в наш MSAA-буфер и очищаем его перед рисованием 3D-сцены
    void begin() {
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glViewport(0, 0, width, height);
        glClearColor(0.02f, 0.01f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Шаг 2: копируем сглаженный результат из мультисемплового FBO в обычную текстуру (resolve)
    void end() {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Шаг 3: рисуем получившуюся текстуру на экран с шейдером пост-обработки
    void render(const Shader& shader, float time) {
        glDisable(GL_DEPTH_TEST);
        shader.use();
        shader.setInt("screenTexture", 0);
        shader.setBool("crtEnabled", enabled);
        shader.setFloat("bloomStrength", bloomStrength);
        shader.setFloat("time", time);
        shader.setVec2("resolution", glm::vec2((float)width, (float)height));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }
};

#endif
