#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#elif __has_include(<GL/glew.h>)
#include <GL/glew.h>
#elif __has_include(<glad/glad.h>)
#include <glad/glad.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// ============================================================================
// МОДУЛЬ ШЕЙДЕРОВ (Shader.h) — отвечает за критерий R1 (свои GLSL 330 шейдеры)
// ВАЖНО ДЛЯ ЗАЩИТЫ:
//   1) Читает текст файлов .vert и .frag с диска через std::ifstream.
//   2) Создаёт шейдеры (glCreateShader), загружает код (glShaderSource) и компилирует (glCompileShader).
//   3) Собирает их в одну GPU-программу (glCreateProgram + glAttachShader + glLinkProgram).
//   4) Передаёт переменные (матрицы, свет, время) с CPU на видеокарту через uniform (glUniform*).
// ============================================================================
class Shader {
public:
    unsigned int ID; // номер скомпилированной шейдерной программы в OpenGL

    // Конструктор: принимает пути к файлам вершинного и фрагментного шейдеров
    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            // Считываем текст обоих шейдеров в строки
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure&) {
            std::cerr << "SHADER_ERROR::FILE_NOT_READ: " << vertexPath << " or " << fragmentPath << std::endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 1. Компиляция вершинного шейдера (обрабатывает координаты вершин)
        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        // 2. Компиляция фрагментного шейдера (считает итоговый цвет каждого пикселя)
        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // 3. Линковка обоих шейдеров в одну программу
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // После линковки отдельные объекты шейдеров больше не нужны
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // Активирует этот шейдер перед вызовом glDrawArrays / glDrawElements
    void use() const {
        glUseProgram(ID);
    }

    // Методы передачи uniform-переменных из C++ в активный GLSL шейдер
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string &name, const glm::vec2 &val) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(val));
    }

    void setVec3(const std::string &name, const glm::vec3 &val) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(val));
    }

    void setVec4(const std::string &name, const glm::vec4 &val) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(val));
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

private:
    // Проверяет ошибки компиляции GLSL и выводит лог в консоль, если где-то опечатка
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "SHADER_COMPILATION_ERROR::" << type << "\n" << infoLog << "\n";
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "PROGRAM_LINKING_ERROR::" << type << "\n" << infoLog << "\n";
            }
        }
    }
};

#endif
