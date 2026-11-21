#ifndef GL_HEADERS_H
#define GL_HEADERS_H

// ============================================================================
// ПОДКЛЮЧЕНИЕ ЗАГОЛОВКОВ OPENGL 3.3 CORE (GLHeaders.h) — критерий R1 / R8
// Кросс-платформенная обёртка: на macOS подключается системный OpenGL/gl3.h,
// а на Windows и Linux — загрузчик расширений GLEW (или GLAD), который получает
// адреса всех современных функций OpenGL 3.3 (glGenVertexArrays, glCreateShader и др.).
// ============================================================================
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#elif __has_include(<GL/glew.h>)
#include <GL/glew.h>
#elif __has_include(<glad/glad.h>)
#include <glad/glad.h>
#endif

#endif
