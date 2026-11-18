#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

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
#include "Car.h"
#include "Terrain.h"
#include "Mountain.h"
#include "PalmTree.h"
#include "VegasCity.h"
#include "Track.h"
#include "TrackRenderer.h"
#include "TextRenderer.h"
#include "Sky.h"
#include "SpeedParticles.h"
#include "PostProcess.h"
#include "Theme.h"

unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 3.2f, 8.2f));
float lastX = 640.0f;
float lastY = 360.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

PostProcess* gPostProcess = nullptr;

enum GameState {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (width <= 0 || height <= 0) return;
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
    if (gPostProcess) {
        gPostProcess->resize(width, height);
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Synthwave Surfers | DeLorean 1985 | SE-2423", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

#if !defined(__APPLE__)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    Shader skyShader("shaders/sky.vert", "shaders/sky.frag");
    Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");
    Shader mountainShader("shaders/mountain.vert", "shaders/mountain.frag");
    Shader carShader("shaders/car.vert", "shaders/car.frag");
    Shader instancedShader("shaders/instanced.vert", "shaders/instanced.frag");
    Shader buildingShader("shaders/building.vert", "shaders/building.frag");
    Shader particleShader("shaders/particles.vert", "shaders/particles.frag");
    Shader postprocessShader("shaders/postprocess.vert", "shaders/postprocess.frag");

    Sky sky;
    Car delorean;
    Terrain terrain;
    Mountain mountains;
    PalmTree palmTrees;
    VegasCity vegasCity;
    Track track;
    TrackRenderer trackRenderer;
    TextRenderer textRenderer;
    SpeedParticles speedParticles(160);
    PostProcess postProcess(SCR_WIDTH, SCR_HEIGHT);
    gPostProcess = &postProcess;

    std::vector<Theme> themes = makeThemes();
    int themeIdx = 0;

    GameState state = STATE_MENU;

    int currentLane = 1;
    float scrollDistance = 0.0f;
    float baseSpeed = 28.0f;
    float currentSpeed = 28.0f;
    float boostFactor = 0.0f;

    int score = 0;
    float scoreDistAccum = 0.0f;
    int coins = 0;
    int lives = 3;
    int highScore = 4850;
    int scoreMultiplier = 1;

    bool wordCollected[5] = {false, false, false, false, false};
    const char wordTarget[6] = "SYNTH";

    float hoverTimer = 0.0f;
    float magnetTimer = 0.0f;
    float multiplierTimer = 0.0f;
    float invulnerableTimer = 0.0f;
    float shakeTimer = 0.0f;
    float flashTimer = 0.0f;
    float bannerTimer = 0.0f;
    std::string bannerText = "";

    bool keyCPressed = false;
    bool keyTPressed = false;
    bool keyLPressed = false;
    bool keyVPressed = false;
    bool keyFPressed = false;
    bool keySpacePressed = false;
    bool keyAPressed = false;
    bool keyDPressed = false;
    bool keyPPressed = false;
    bool keyEnterPressed = false;
    bool keyRPressed = false;

    double lastFpsTime = glfwGetTime();
    int frameCount = 0;

    auto resetGame = [&]() {
        currentLane = 1;
        scrollDistance = 0.0f;
        currentSpeed = baseSpeed;
        score = 0;
        scoreDistAccum = 0.0f;
        coins = 0;
        lives = 3;
        scoreMultiplier = 1;
        hoverTimer = 0.0f;
        magnetTimer = 0.0f;
        multiplierTimer = 0.0f;
        invulnerableTimer = 0.0f;
        shakeTimer = 0.0f;
        flashTimer = 0.0f;
        bannerTimer = 0.0f;
        bannerText = "";
        for (int i = 0; i < 5; ++i) wordCollected[i] = false;
        delorean.reset();
        track.reset(0.0f);
    };

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        lastFrame = currentFrame;

        frameCount++;
        if (currentFrame - lastFpsTime >= 0.25) {
            double fps = (double)frameCount / (currentFrame - lastFpsTime);
            std::string stStr = (state == STATE_MENU) ? "MENU" : (state == STATE_PLAYING ? (delorean.hover ? "HOVER FLIGHT" : "RUNNING") : (state == STATE_PAUSED ? "PAUSED" : "GAME OVER"));
            std::string title = "SYNTHWAVE SURFERS | SCORE: " + std::to_string(score) +
                                " | COINS: " + std::to_string(coins) +
                                " | LIVES: " + std::to_string(lives) +
                                " | STATE: " + stStr +
                                " | FPS: " + std::to_string((int)fps);
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsTime = currentFrame;
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            if (!keyCPressed) {
                postProcess.enabled = !postProcess.enabled;
                keyCPressed = true;
            }
        } else keyCPressed = false;

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            if (!keyTPressed) {
                themeIdx = (themeIdx + 1) % themes.size();
                keyTPressed = true;
            }
        } else keyTPressed = false;

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            if (!keyLPressed) {
                delorean.headlights = !delorean.headlights;
                keyLPressed = true;
            }
        } else keyLPressed = false;

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            camera.SetMode(CHASE_CAM);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            camera.SetMode(HOOD_CAM);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            camera.SetMode(FREE_CAM);
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
            if (!keyVPressed) {
                int nextMode = (camera.Mode + 1) % 3;
                camera.SetMode(static_cast<CameraMode>(nextMode));
                if (camera.Mode == FREE_CAM) {
                    firstMouse = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                } else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                keyVPressed = true;
            }
        } else keyVPressed = false;

        if (state == STATE_MENU) {
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                if (!keyEnterPressed && !keySpacePressed) {
                    resetGame();
                    state = STATE_PLAYING;
                    keyEnterPressed = true;
                    keySpacePressed = true;
                }
            } else {
                keyEnterPressed = false;
                keySpacePressed = false;
            }

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }

            scrollDistance += 12.0f * deltaTime;
            delorean.update(Track::laneX(1), 12.0f, false, deltaTime);
        }
        else if (state == STATE_PLAYING) {
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                if (!keyPPressed) {
                    state = STATE_PAUSED;
                    keyPPressed = true;
                }
            } else keyPPressed = false;

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                state = STATE_PAUSED;
            }

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                if (!keyAPressed) {
                    if (currentLane > 0) currentLane--;
                    keyAPressed = true;
                }
            } else keyAPressed = false;

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                if (!keyDPressed) {
                    if (currentLane < 2) currentLane++;
                    keyDPressed = true;
                }
            } else keyDPressed = false;

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                if (!keySpacePressed) {
                    delorean.jump();
                    keySpacePressed = true;
                }
            } else keySpacePressed = false;

            bool duckInput = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
            bool boostInput = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);

            if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
                if (!keyFPressed) {
                    delorean.hover = !delorean.hover;
                    if (delorean.hover) {
                        hoverTimer = 14.0f;
                        bannerText = "CYBER WINGS DEPLOYED! HOVER FLIGHT!";
                        bannerTimer = 2.2f;
                    }
                    keyFPressed = true;
                }
            } else keyFPressed = false;

            if (hoverTimer > 0.0f) {
                hoverTimer -= deltaTime;
                delorean.hover = true;
                if (hoverTimer <= 0.0f) {
                    delorean.hover = false;
                    track.endFlight();
                }
            }

            if (magnetTimer > 0.0f) {
                magnetTimer -= deltaTime;
            }

            if (multiplierTimer > 0.0f) {
                multiplierTimer -= deltaTime;
                scoreMultiplier = 2;
                if (multiplierTimer <= 0.0f) scoreMultiplier = 1;
            } else {
                scoreMultiplier = 1;
            }

            if (invulnerableTimer > 0.0f) {
                invulnerableTimer -= deltaTime;
                delorean.visible = (static_cast<int>(invulnerableTimer * 16.0f) % 2 == 0);
            } else {
                delorean.visible = true;
            }

            if (shakeTimer > 0.0f) shakeTimer -= deltaTime;
            if (flashTimer > 0.0f) flashTimer -= deltaTime;
            if (bannerTimer > 0.0f) bannerTimer -= deltaTime;

            float targetSpeed = baseSpeed;
            if (boostInput) targetSpeed = baseSpeed * 2.2f;
            if (delorean.hover) targetSpeed = baseSpeed * 1.55f;

            currentSpeed += (targetSpeed - currentSpeed) * 6.0f * deltaTime;
            float targetBoost = (boostInput || delorean.hover) ? 1.0f : 0.0f;
            boostFactor += (targetBoost - boostFactor) * 5.0f * deltaTime;

            float targetX = Track::laneX(currentLane);
            delorean.update(targetX, currentSpeed, duckInput, deltaTime);
            scrollDistance += currentSpeed * deltaTime;

            scoreDistAccum += currentSpeed * deltaTime;
            if (scoreDistAccum >= 2.8f) {
                score += 1 * scoreMultiplier;
                scoreDistAccum -= 2.8f;
            }

            int neededLetter = 0;
            for (int i = 0; i < 5; ++i) {
                if (!wordCollected[i]) {
                    neededLetter = i;
                    break;
                }
            }

            track.ensure(scrollDistance, delorean.hover, neededLetter, 1.0f);

            bool magnetActive = (magnetTimer > 0.0f);
            bool invuln = (invulnerableTimer > 0.0f);
            TrackEvents ev = track.update(scrollDistance, delorean.posX, delorean.wheelBottom(), delorean.hover, invuln, magnetActive, deltaTime);

            if (ev.coins > 0) {
                coins += ev.coins;
                score += ev.coins * 25 * scoreMultiplier;
            }

            if (ev.letterIndex >= 0 && ev.letterIndex < 5) {
                if (!wordCollected[ev.letterIndex]) {
                    wordCollected[ev.letterIndex] = true;
                    score += 250 * scoreMultiplier;
                    bannerText = std::string("FOUND LETTER [") + wordTarget[ev.letterIndex] + "]!";
                    bannerTimer = 1.8f;

                    bool all = true;
                    for (int k = 0; k < 5; ++k) if (!wordCollected[k]) all = false;
                    if (all) {
                        score += 2000;
                        delorean.hover = true;
                        hoverTimer = 16.0f;
                        track.startFlight(scrollDistance);
                        bannerText = "SYNTH COMPLETE! +2000 PTS! SUPER FLIGHT!";
                        bannerTimer = 3.0f;
                        for (int k = 0; k < 5; ++k) wordCollected[k] = false;
                    }
                }
            }

            if (ev.flightOrb) {
                delorean.hover = true;
                hoverTimer = 14.0f;
                track.startFlight(scrollDistance);
                bannerText = "FLIGHT ORB! WINGS DEPLOYED!";
                bannerTimer = 2.0f;
            }

            if (ev.magnetOrb) {
                magnetTimer = 12.0f;
                bannerText = "COIN MAGNET ACTIVATED!";
                bannerTimer = 2.0f;
            }

            if (ev.crash) {
                lives--;
                shakeTimer = 0.45f;
                flashTimer = 0.35f;
                invulnerableTimer = 1.8f;
                bannerText = "CRASH! -1 LIFE!";
                bannerTimer = 1.5f;
                currentSpeed *= 0.55f;

                if (lives <= 0) {
                    state = STATE_GAMEOVER;
                    delorean.visible = true;
                    if (score > highScore) highScore = score;
                }
            }
        }
        else if (state == STATE_PAUSED) {
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                if (!keyPPressed) {
                    state = STATE_PLAYING;
                    keyPPressed = true;
                }
            } else keyPPressed = false;

            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                resetGame();
                state = STATE_PLAYING;
            }

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                state = STATE_MENU;
            }
        }
        else if (state == STATE_GAMEOVER) {
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                if (!keyRPressed && !keySpacePressed) {
                    resetGame();
                    state = STATE_PLAYING;
                    keyRPressed = true;
                    keySpacePressed = true;
                }
            } else {
                keyRPressed = false;
                keySpacePressed = false;
            }

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                state = STATE_MENU;
            }

            scrollDistance += 8.0f * deltaTime;
            delorean.update(Track::laneX(1), 8.0f, false, deltaTime);
        }

        if (camera.Mode == FREE_CAM) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(0, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(1, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(2, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(3, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.ProcessKeyboard(4, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.ProcessKeyboard(5, deltaTime);
        } else {
            camera.UpdateChase(delorean.posX, delorean.posY, delorean.tilt, boostFactor, deltaTime);
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 320.0f);
        glm::mat4 view = camera.GetViewMatrix();

        if (shakeTimer > 0.0f) {
            float s = (shakeTimer / 0.45f) * 0.28f;
            float rX = ((float)rand() / (float)RAND_MAX - 0.5f) * s;
            float rY = ((float)rand() / (float)RAND_MAX - 0.5f) * s;
            view = glm::translate(glm::mat4(1.0f), glm::vec3(rX, rY, 0.0f)) * view;
        }

        const Theme& th = themes[themeIdx];
        glm::vec3 sunLightPos(0.0f, 18.0f, -70.0f);

        postProcess.begin();

        sky.draw(skyShader, currentFrame, glm::vec2((float)SCR_WIDTH, (float)SCR_HEIGHT),
                 th.skyTop, th.skyMid, th.skyHorizon);

        vegasCity.draw(buildingShader, view, projection, scrollDistance, camera.Position, currentFrame, th.mountainBase);

        mountains.draw(mountainShader, view, projection, camera.Position,
                       th.mountainBase, th.mountainPeak);

        terrain.draw(terrainShader, view, projection, scrollDistance, camera.Position,
                     glm::vec3(delorean.posX, delorean.posY, 0.0f), delorean.headlights,
                     th.gridLine, th.road);

        palmTrees.draw(instancedShader, view, projection, scrollDistance,
                       sunLightPos, th.accent, camera.Position);

        carShader.use();
        carShader.setMat4("projection", projection);
        carShader.setMat4("view", view);
        carShader.setVec3("viewPos", camera.Position);
        carShader.setVec3("lightPos1", sunLightPos);
        carShader.setVec3("lightColor1", th.skyHorizon);
        carShader.setVec3("lightPos2", glm::vec3(0.0f, 6.0f, 10.0f));
        carShader.setVec3("lightColor2", glm::vec3(0.4f, 0.4f, 0.5f));

        trackRenderer.draw(carShader, track, scrollDistance, currentFrame);

        delorean.draw(carShader, th.glow);

        speedParticles.updateAndDraw(particleShader, view, projection, currentSpeed, boostFactor, deltaTime);

        postProcess.end();
        postProcess.render(postprocessShader, currentFrame);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        textRenderer.begin(SCR_WIDTH, SCR_HEIGHT);

        float sw = static_cast<float>(SCR_WIDTH);
        float sh = static_cast<float>(SCR_HEIGHT);

        if (flashTimer > 0.0f) {
            float alpha = (flashTimer / 0.35f) * 0.45f;
            textRenderer.rect(0.0f, 0.0f, sw, sh, glm::vec4(1.0f, 0.0f, 0.1f, alpha));
        }

        if (state == STATE_MENU) {
            textRenderer.rect(sw * 0.12f, sh * 0.08f, sw * 0.76f, sh * 0.84f, glm::vec4(0.02f, 0.01f, 0.06f, 0.88f));
            textRenderer.frame(sw * 0.12f, sh * 0.08f, sw * 0.76f, sh * 0.84f, 4.0f, glm::vec4(0.10f, 0.95f, 1.0f, 0.95f));
            textRenderer.frame(sw * 0.12f + 6.0f, sh * 0.08f + 6.0f, sw * 0.76f - 12.0f, sh * 0.84f - 12.0f, 2.0f, glm::vec4(1.0f, 0.15f, 0.65f, 0.80f));

            textRenderer.textCentered("SYNTHWAVE SURFERS", sw * 0.5f, sh * 0.15f, 5.0f,
                                      glm::vec4(1.0f, 0.15f, 0.65f, 1.0f), glm::vec4(0.0f, 0.8f, 1.0f, 0.9f));
            textRenderer.textCentered("DELOREAN 1985 TIME MACHINE", sw * 0.5f, sh * 0.24f, 2.6f,
                                      glm::vec4(0.10f, 0.95f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.9f));
            textRenderer.textCentered("ASTANA IT UNIVERSITY | GROUP SE-2423", sw * 0.5f, sh * 0.30f, 1.8f,
                                      glm::vec4(1.0f, 0.85f, 0.2f, 0.9f), glm::vec4(0.0f, 0.0f, 0.0f, 0.8f));

            float pulse = 0.5f + 0.5f * std::sin(currentFrame * 5.0f);
            textRenderer.textCentered("PRESS SPACE OR ENTER TO DRIVE", sw * 0.5f, sh * 0.40f, 2.8f,
                                      glm::vec4(1.0f, 1.0f, 1.0f, 0.6f + 0.4f * pulse), glm::vec4(1.0f, 0.1f, 0.6f, 0.9f));

            float cardX = sw * 0.18f;
            float cardY = sh * 0.50f;
            textRenderer.rect(cardX, cardY, sw * 0.64f, sh * 0.36f, glm::vec4(0.05f, 0.03f, 0.12f, 0.75f));
            textRenderer.frame(cardX, cardY, sw * 0.64f, sh * 0.36f, 2.0f, glm::vec4(0.2f, 0.8f, 1.0f, 0.6f));

            textRenderer.text("CONTROLS (3 LANES):", cardX + 24.0f, cardY + 20.0f, 2.0f, glm::vec4(1.0f, 0.85f, 0.1f, 1.0f));
            textRenderer.text("A / D or ARROW KEYS : SWITCH BETWEEN 3 LANES", cardX + 24.0f, cardY + 50.0f, 1.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
            textRenderer.text("SPACE               : JUMP OVER LOW BARRIERS", cardX + 24.0f, cardY + 76.0f, 1.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
            textRenderer.text("S or DOWN ARROW     : DUCK UNDER LASER GATES", cardX + 24.0f, cardY + 102.0f, 1.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
            textRenderer.text("W or UP ARROW       : TURBO BOOST (SPEED X2.2)", cardX + 24.0f, cardY + 128.0f, 1.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
            textRenderer.text("F                   : TOGGLE HOVER FLIGHT MODE", cardX + 24.0f, cardY + 154.0f, 1.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
            textRenderer.text("C : CRT SCANLINES   |  T : THEME  |  1/2/3 : CAMERAS", cardX + 24.0f, cardY + 180.0f, 1.8f, glm::vec4(0.1f, 0.95f, 0.9f, 1.0f));
            textRenderer.text("P : PAUSE           |  V : CYCLE CAMERAS", cardX + 24.0f, cardY + 206.0f, 1.8f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
        }
        else if (state == STATE_PLAYING || state == STATE_PAUSED) {
            textRenderer.rect(16.0f, 16.0f, 310.0f, 96.0f, glm::vec4(0.04f, 0.02f, 0.09f, 0.78f));
            textRenderer.frame(16.0f, 16.0f, 310.0f, 96.0f, 2.0f, glm::vec4(0.10f, 0.90f, 1.0f, 0.7f));
            std::string scoreStr = "SCORE: " + std::to_string(score);
            textRenderer.text(scoreStr, 28.0f, 26.0f, 2.6f, glm::vec4(1.0f, 0.90f, 0.20f, 1.0f));

            if (scoreMultiplier > 1) {
                textRenderer.text("X2 MULTIPLIER", 28.0f, 54.0f, 1.8f, glm::vec4(1.0f, 0.20f, 0.70f, 1.0f));
            } else {
                textRenderer.text("HIGH: " + std::to_string(highScore), 28.0f, 54.0f, 1.8f, glm::vec4(0.6f, 0.6f, 0.7f, 1.0f));
            }

            int mph = static_cast<int>(currentSpeed * 2.85f);
            std::string spdStr = "SPEED: " + std::to_string(mph) + " MPH";
            textRenderer.text(spdStr, 28.0f, 78.0f, 1.8f, (mph >= 80) ? glm::vec4(1.0f, 0.3f, 0.1f, 1.0f) : glm::vec4(0.2f, 0.9f, 0.9f, 1.0f));

            float wordCardW = 280.0f;
            float wordCardX = sw * 0.5f - wordCardW * 0.5f;
            textRenderer.rect(wordCardX, 16.0f, wordCardW, 58.0f, glm::vec4(0.04f, 0.02f, 0.09f, 0.78f));
            textRenderer.frame(wordCardX, 16.0f, wordCardW, 58.0f, 2.0f, glm::vec4(1.0f, 0.15f, 0.65f, 0.7f));
            textRenderer.textCentered("COLLECT WORD:", sw * 0.5f, 24.0f, 1.6f, glm::vec4(0.8f, 0.8f, 0.9f, 1.0f), glm::vec4(0.0f));

            float badgeStartX = sw * 0.5f - 90.0f;
            for (int k = 0; k < 5; ++k) {
                float bx = badgeStartX + (float)k * 38.0f;
                bool got = wordCollected[k];
                textRenderer.rect(bx, 42.0f, 30.0f, 24.0f, got ? glm::vec4(0.95f, 0.75f, 0.1f, 0.9f) : glm::vec4(0.12f, 0.08f, 0.18f, 0.8f));
                textRenderer.frame(bx, 42.0f, 30.0f, 24.0f, 1.5f, got ? glm::vec4(1.0f, 1.0f, 0.4f, 1.0f) : glm::vec4(0.4f, 0.3f, 0.5f, 0.5f));
                std::string lStr(1, wordTarget[k]);
                textRenderer.textCentered(lStr, bx + 15.0f, 46.0f, 2.2f, got ? glm::vec4(0.05f, 0.02f, 0.10f, 1.0f) : glm::vec4(0.5f, 0.5f, 0.6f, 0.6f), glm::vec4(0.0f));
            }

            float rightCardW = 260.0f;
            float rightCardX = sw - rightCardW - 16.0f;
            textRenderer.rect(rightCardX, 16.0f, rightCardW, 96.0f, glm::vec4(0.04f, 0.02f, 0.09f, 0.78f));
            textRenderer.frame(rightCardX, 16.0f, rightCardW, 96.0f, 2.0f, glm::vec4(0.10f, 0.90f, 1.0f, 0.7f));
            textRenderer.text("COINS: " + std::to_string(coins), rightCardX + 16.0f, 26.0f, 2.4f, glm::vec4(1.0f, 0.85f, 0.15f, 1.0f));

            std::string heartStr = "LIVES: ";
            for (int i = 0; i < 3; ++i) heartStr += (i < lives) ? "<3 " : ".. ";
            textRenderer.text(heartStr, rightCardX + 16.0f, 56.0f, 2.0f, (lives > 1) ? glm::vec4(1.0f, 0.15f, 0.3f, 1.0f) : glm::vec4(1.0f, 0.8f, 0.1f, 1.0f));

            std::string modeLabel = delorean.hover ? "HOVER FLIGHT" : (delorean.grounded ? "GROUND" : "JUMPING");
            textRenderer.text("MODE: " + modeLabel, rightCardX + 16.0f, 82.0f, 1.6f, delorean.hover ? glm::vec4(0.1f, 0.95f, 1.0f, 1.0f) : glm::vec4(0.7f, 0.7f, 0.8f, 1.0f));

            if (hoverTimer > 0.0f) {
                float barW = 320.0f;
                float barX = sw * 0.5f - barW * 0.5f;
                float barY = sh - 68.0f;
                textRenderer.rect(barX, barY, barW, 36.0f, glm::vec4(0.02f, 0.04f, 0.10f, 0.85f));
                textRenderer.frame(barX, barY, barW, 36.0f, 2.0f, glm::vec4(0.10f, 0.95f, 1.0f, 0.9f));
                float fill = std::clamp(hoverTimer / 14.0f, 0.0f, 1.0f);
                textRenderer.rect(barX + 4.0f, barY + 4.0f, (barW - 8.0f) * fill, 28.0f, glm::vec4(0.10f, 0.85f, 1.0f, 0.85f));
                textRenderer.textCentered("HOVER FLIGHT " + std::to_string((int)hoverTimer + 1) + "s", sw * 0.5f, barY + 10.0f, 2.0f,
                                          glm::vec4(0.02f, 0.02f, 0.05f, 1.0f), glm::vec4(0.0f));
            } else if (magnetTimer > 0.0f) {
                float barW = 300.0f;
                float barX = sw * 0.5f - barW * 0.5f;
                float barY = sh - 68.0f;
                textRenderer.rect(barX, barY, barW, 36.0f, glm::vec4(0.05f, 0.04f, 0.02f, 0.85f));
                textRenderer.frame(barX, barY, barW, 36.0f, 2.0f, glm::vec4(1.0f, 0.85f, 0.15f, 0.9f));
                float fill = std::clamp(magnetTimer / 12.0f, 0.0f, 1.0f);
                textRenderer.rect(barX + 4.0f, barY + 4.0f, (barW - 8.0f) * fill, 28.0f, glm::vec4(1.0f, 0.80f, 0.10f, 0.85f));
                textRenderer.textCentered("COIN MAGNET " + std::to_string((int)magnetTimer + 1) + "s", sw * 0.5f, barY + 10.0f, 2.0f,
                                          glm::vec4(0.02f, 0.02f, 0.05f, 1.0f), glm::vec4(0.0f));
            }

            if (bannerTimer > 0.0f && !bannerText.empty()) {
                float bw = textRenderer.width(bannerText, 2.4f) + 40.0f;
                float bx = sw * 0.5f - bw * 0.5f;
                float by = sh * 0.28f;
                textRenderer.rect(bx, by, bw, 44.0f, glm::vec4(0.03f, 0.01f, 0.08f, 0.92f));
                textRenderer.frame(bx, by, bw, 44.0f, 2.0f, glm::vec4(1.0f, 0.85f, 0.15f, 0.95f));
                textRenderer.textCentered(bannerText, sw * 0.5f, by + 11.0f, 2.4f, glm::vec4(1.0f, 0.90f, 0.20f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.8f));
            }

            if (state == STATE_PAUSED) {
                textRenderer.rect(0.0f, 0.0f, sw, sh, glm::vec4(0.02f, 0.01f, 0.05f, 0.70f));
                textRenderer.textCentered("GAME PAUSED", sw * 0.5f, sh * 0.40f, 4.0f,
                                          glm::vec4(1.0f, 0.85f, 0.2f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.9f));
                textRenderer.textCentered("PRESS P TO RESUME | R TO RESTART | ESC FOR MENU", sw * 0.5f, sh * 0.52f, 2.0f,
                                          glm::vec4(0.9f, 0.9f, 0.95f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.8f));
            }
        }
        else if (state == STATE_GAMEOVER) {
            textRenderer.rect(0.0f, 0.0f, sw, sh, glm::vec4(0.04f, 0.01f, 0.06f, 0.75f));

            float cardW = sw * 0.54f;
            float cardH = sh * 0.64f;
            float cardX = sw * 0.5f - cardW * 0.5f;
            float cardY = sh * 0.18f;

            textRenderer.rect(cardX, cardY, cardW, cardH, glm::vec4(0.03f, 0.02f, 0.08f, 0.92f));
            textRenderer.frame(cardX, cardY, cardW, cardH, 4.0f, glm::vec4(1.0f, 0.10f, 0.25f, 0.95f));
            textRenderer.frame(cardX + 6.0f, cardY + 6.0f, cardW - 12.0f, cardH - 12.0f, 2.0f, glm::vec4(1.0f, 0.80f, 0.15f, 0.80f));

            textRenderer.textCentered("GAME OVER", sw * 0.5f, cardY + 30.0f, 4.5f,
                                      glm::vec4(1.0f, 0.15f, 0.30f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.9f));

            textRenderer.text("FINAL SCORE     : " + std::to_string(score), cardX + 48.0f, cardY + 110.0f, 2.4f, glm::vec4(1.0f, 0.90f, 0.20f, 1.0f));
            textRenderer.text("COINS COLLECTED : " + std::to_string(coins), cardX + 48.0f, cardY + 148.0f, 2.2f, glm::vec4(0.9f, 0.9f, 0.95f, 1.0f));
            textRenderer.text("HIGH SCORE      : " + std::to_string(highScore), cardX + 48.0f, cardY + 184.0f, 2.2f, glm::vec4(0.10f, 0.95f, 1.0f, 1.0f));

            float pulse = 0.5f + 0.5f * std::sin(currentFrame * 6.0f);
            textRenderer.textCentered("PRESS SPACE OR R TO RETRY", sw * 0.5f, cardY + cardH - 78.0f, 2.6f,
                                      glm::vec4(1.0f, 1.0f, 1.0f, 0.6f + 0.4f * pulse), glm::vec4(1.0f, 0.1f, 0.4f, 0.8f));
            textRenderer.textCentered("PRESS ESC FOR MAIN MENU", sw * 0.5f, cardY + cardH - 42.0f, 1.8f,
                                      glm::vec4(0.7f, 0.7f, 0.8f, 0.9f), glm::vec4(0.0f, 0.0f, 0.0f, 0.8f));
        }

        textRenderer.flush();

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
