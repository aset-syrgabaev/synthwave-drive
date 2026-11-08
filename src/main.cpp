#include <iostream>
#include <string>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#elif __has_include(<GL/glew.h>)
#include <GL/glew.h>
#elif __has_include(<glad/glad.h>)
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Sky.h"
#include "Car.h"
#include "Mountain.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 2.2f, 5.8f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Vehicle state
float currentSpeed = 25.0f;
float targetSteer = 0.0f;
float scrollDistance = 0.0f;
bool freeCameraMode = false;
bool cKeyPressed = false;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Car Steering (A / D or Left / Right)
    targetSteer = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        targetSteer -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        targetSteer += 1.0f;

    // Speed Control (W / S or Up / Down: Wow-moment acceleration)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        currentSpeed += 25.0f * deltaTime;
        if (currentSpeed > 75.0f) currentSpeed = 75.0f;
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        currentSpeed -= 25.0f * deltaTime;
        if (currentSpeed < 10.0f) currentSpeed = 10.0f;
    } else {
        // Natural speed settle towards 25.0
        currentSpeed += (25.0f - currentSpeed) * 1.5f * deltaTime;
    }

    // Toggle Camera Mode with C key
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cKeyPressed) {
            freeCameraMode = !freeCameraMode;
            cKeyPressed = true;
        }
    } else {
        cKeyPressed = false;
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Synthwave Drive | Team SE-2423", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

#if defined(__APPLE__)
#elif __has_include(<GL/glew.h>)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
#elif __has_include(<glad/glad.h>)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
#endif

    glEnable(GL_DEPTH_TEST);

    Shader skyShader("shaders/sky.vert", "shaders/sky.frag");
    Shader gridShader("shaders/grid.vert", "shaders/grid.frag");
    Shader carShader("shaders/car.vert", "shaders/car.frag");
    Shader mountainShader("shaders/mountain.vert", "shaders/mountain.frag");

    Sky sky;
    Car car;
    Mountain mountains;

    // Scrolling Road & Ground Plane
    float planeVertices[] = {
        -70.0f, 0.0f, -180.0f,   0.0f, 70.0f,
         70.0f, 0.0f, -180.0f,  35.0f, 70.0f,
         70.0f, 0.0f,   15.0f,  35.0f,  0.0f,

        -70.0f, 0.0f, -180.0f,   0.0f, 70.0f,
         70.0f, 0.0f,   15.0f,  35.0f,  0.0f,
        -70.0f, 0.0f,   15.0f,   0.0f,  0.0f
    };

    unsigned int gridVAO, gridVBO;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    int frameCount = 0;
    double prevTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Update car steering & rolling motion
        car.update(targetSteer, currentSpeed, deltaTime);

        // Advance texture scroll distance with speed
        scrollDistance += currentSpeed * deltaTime * 0.45f;

        // Camera follow
        glm::vec3 camPos;
        if (freeCameraMode) {
            camPos = glm::vec3(0.0f, 7.0f, 16.0f);
        } else {
            camPos = glm::vec3(car.posX * 0.55f, 2.2f, 5.8f);
        }
        camera.Position = camPos;
        glm::vec3 targetLook = glm::vec3(car.posX * 0.7f, 0.8f, -10.0f);
        glm::mat4 view = glm::lookAt(camera.Position, targetLook, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(55.0f), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 300.0f);

        // FPS & Dashboard counter in Title (R7)
        frameCount++;
        if (currentFrame - prevTime >= 0.5) {
            int fps = static_cast<int>(frameCount / (currentFrame - prevTime));
            std::string title = "Synthwave Drive [SE-2423] | FPS: " + std::to_string(fps) + 
                                " | Speed: " + std::to_string(static_cast<int>(currentSpeed * 3.6f)) + " km/h" +
                                " | [A/D: Steer, W/S: Speed, C: Cam]";
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            prevTime = currentFrame;
        }

        glClearColor(0.04f, 0.0f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Render Sky with Striped 80s Sun & Stars
        skyShader.use();
        skyShader.setFloat("time", currentFrame);
        skyShader.setVec2("resolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
        sky.draw();

        // 2. Render Scrolling Neon Grid & Highway
        gridShader.use();
        gridShader.setMat4("projection", projection);
        gridShader.setMat4("view", view);
        gridShader.setMat4("model", glm::mat4(1.0f));
        gridShader.setFloat("scrollOffset", scrollDistance);
        gridShader.setVec3("camPos", camera.Position);

        glBindVertexArray(gridVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // 3. Render Low-Poly Mountains on sides
        mountains.draw(mountainShader, view, projection, camera.Position);

        // 4. Render 3D Neon Supercar with Blinn-Phong lighting & glowing elements
        carShader.use();
        carShader.setMat4("projection", projection);
        carShader.setMat4("view", view);
        carShader.setVec3("viewPos", camera.Position);

        // Light 1: Sun / Sky ambient light from horizon
        carShader.setVec3("lightPos1", glm::vec3(0.0f, 15.0f, -60.0f));
        carShader.setVec3("lightColor1", glm::vec3(1.0f, 0.4f, 0.7f));

        // Light 2: Dynamic neon road reflection under car
        carShader.setVec3("lightPos2", glm::vec3(car.posX, 0.1f, 0.0f));
        carShader.setVec3("lightColor2", glm::vec3(0.0f, 0.9f, 1.0f));

        car.draw(carShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
    glfwTerminate();
    return 0;
}
