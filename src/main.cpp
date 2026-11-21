#include <iostream>
#include <string>
#include <algorithm>
#include "GLHeaders.h"
#include <GLFW/glfw3.h>
#include "GameApp.h"

// Указатель на главную структуру игры, чтобы функции-коллбэки GLFW могли менять камеру и размеры окна
static GameApp* gApp = nullptr;
// Прошлая позиция мыши, нужна чтобы считать на сколько пикселей сдвинулся курсор
static float gLastX = 640.0f;
static float gLastY = 360.0f;
static bool gFirstMouse = true;

// GLFW сам вызывает эту функцию, когда пользователь растягивает или уменьшает окно
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    if (width <= 0 || height <= 0 || !gApp) return;
    gApp->scrWidth = static_cast<unsigned int>(width);
    gApp->scrHeight = static_cast<unsigned int>(height);
    // Говорим OpenGL рисовать на всю новую область окна
    glViewport(0, 0, width, height);
    // Пересоздаём текстуры фреймбуфера (PostProcess) под новый размер, иначе картинка растянется
    gApp->postProcess.resize(width, height);
}

// GLFW вызывает это при каждом движении мыши внутри окна
static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!gApp) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    // Сохраняем координаты курсора — они нужны для кликов по меню и настройкам G
    gApp->mouseX = xpos;
    gApp->mouseY = ypos;

    // На самом первом кадре просто запоминаем точку, чтобы камера резко не дёрнулась
    if (gFirstMouse) {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    // Считаем смещение мыши по X и Y (по Y инвертируем, т.к. в окне Y идёт сверху вниз)
    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos;
    gLastX = xpos;
    gLastY = ypos;

    // Если зажата левая или правая кнопка мыши — крутим 3D-камеру вокруг машины
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        gApp->camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// Отслеживаем нажатие левой кнопки мыши (для нажатия кнопок в главном меню и панели G)
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;
    if (gApp && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        gApp->mouseLeftClicked = true;
    }
}

// Главная функция программы: создаём окно OpenGL 3.3 Core, грузим сцену и крутим игровой цикл
int main(int argc, char** argv) {
    // Стартовый размер окна 1280x720 (как требуется в критерии R7/B3)
    unsigned int initW = 1280;
    unsigned int initH = 720;

    // Флаги для замера реального FPS через консоль (--bench ширина высота crt кадры)
    bool benchMode = false;
    int benchCrt = 1;
    int benchTargetFrames = 240;

    if (argc >= 5 && std::string(argv[1]) == "--bench") {
        benchMode = true;
        initW = static_cast<unsigned int>(std::stoi(argv[2]));
        initH = static_cast<unsigned int>(std::stoi(argv[3]));
        benchCrt = std::stoi(argv[4]);
        if (argc > 5) benchTargetFrames = std::stoi(argv[5]);
    }

    // Шаг 1: запускаем библиотеку GLFW и просим современный OpenGL 3.3 Core Profile
    // Core Profile запрещает старые функции вроде glBegin/glVertex (требование R1)
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Включаем 4x MSAA сглаживание краёв прямо на уровне контекста
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Создаём окно программы
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(initW), static_cast<int>(initH),
                                          "SYNTHWAVE SURFERS | DELOREAN DMC-12 & AKIRA BIKE | SE-2423", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Делаем OpenGL-контекст этого окна активным и подключаем обработчики ресайза и мыши
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // В режиме бенчмарка отключаем VSync, чтобы замерить честный максимальный FPS видеокарты
    if (benchMode) glfwSwapInterval(0);

    // Шаг 2: через GLEW загружаем адреса всех функций OpenGL 3.3 из видеодрайвера
#if !defined(__APPLE__)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
#endif

    // Включаем буфер глубины (Z-buffer), чтобы ближние 3D-объекты перекрывали дальние
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);

    // Шаг 3: создаём объект GameApp — внутри компилируются все 9 пар шейдеров и строятся 3D-меши
    GameApp app(initW, initH);
    gApp = &app;

    // Режим --shot нужен только нам для быстрого снятия скриншотов в отчёт и слайды
    // Формат: --shot out.bmp yaw pitch dist bike theme skin play crt hover extra
    int shotExtra = 0;
    if (argc >= 3 && std::string(argv[1]) == "--shot") {
        app.shotMode = true;
        app.shotPath = argv[2];
        if (argc > 3)  app.shotYaw   = std::stof(argv[3]);
        if (argc > 4)  app.shotPitch = std::stof(argv[4]);
        if (argc > 5)  app.shotDist  = std::stof(argv[5]);
        if (argc > 6)  app.shotBike  = std::stoi(argv[6]);
        if (argc > 7)  app.shotTheme = std::stoi(argv[7]);
        if (argc > 8)  app.shotSkin  = std::stoi(argv[8]);
        if (argc > 9)  app.shotPlay  = std::stoi(argv[9]);
        if (argc > 10) app.shotCrt   = std::stoi(argv[10]);
        if (argc > 11) app.shotHover = std::stoi(argv[11]);
        shotExtra = (argc > 12) ? std::stoi(argv[12]) : 0;

        app.themeIdx = std::clamp(app.shotTheme, 0, (int)app.themes.size() - 1);
        if (app.shotBike != 0) app.delorean.toggleVehicleMode();
        app.delorean.paintIdx = std::clamp(app.shotSkin, 0, 7);
        app.delorean.morphFlash = 0.0f;
        app.postProcess.enabled = (app.shotCrt != 0);
        app.camera.OrbitPitch = app.shotPitch;
        app.camera.OrbitDist = app.shotDist;

        if (app.shotPlay != 0) {
            app.gameMode = (app.shotPlay == 2) ? MODE_CLASSIC : MODE_FREE_PLAY;
            app.resetGame();
            app.state = STATE_PLAYING;
            app.delorean.paintIdx = std::clamp(app.shotSkin, 0, 7);
            if (app.shotHover != 0) {
                app.delorean.hover = true;
                app.hoverMaxTimer = 16.0f;
                app.hoverTimer = 14.0f;
                app.track.startFlight(0.0f, app.gameMode == MODE_CLASSIC);
            }
            app.track.ensure(0.0f, app.delorean.hover, 4.53f, 'N', 2.0f, app.gameMode == MODE_CLASSIC);
        }

        // Дополнительные состояния для скринов всех функций (фары выкл, меню G/H, разные камеры)
        if (shotExtra == 1) app.delorean.headlights = false;
        else if (shotExtra == 2) app.showSettings = true;
        else if (shotExtra == 3) app.showHelpOverlay = true;
        else if (shotExtra == 4) app.camera.SetMode(HOOD_CAM);
        else if (shotExtra == 5) app.camera.SetMode(AKIRA_CAM);
        else if (shotExtra == 6) {
            app.camera.SetMode(ORBIT_CAM);
            app.camera.OrbitYaw = app.shotYaw;
            app.camera.OrbitPitch = app.shotPitch;
            app.camera.OrbitDist = app.shotDist;
        } else if (shotExtra == 7) {
            // Прокручиваем чуть вперёд по трассе, чтобы в кадре был виден виадук ЛРТ и препятствия
            app.scrollDistance = 42.0f;
            app.track.ensure(app.scrollDistance, false, 4.53f, 'N', 3.0f, false);
        } else if (shotExtra >= 8 && shotExtra <= 11) {
            // Тест-кадры живого прыжка для проверки всех камер (8=Chase, 9=Action, 10=Hood, 11=Akira)
            app.scrollDistance = 12.0f;
            app.track.ensure(app.scrollDistance, false, 4.53f, 'N', 3.0f, false);
            if (shotExtra == 8)  app.camera.SetMode(CHASE_CAM);
            if (shotExtra == 9)  app.camera.SetMode(ACTION_CAM);
            if (shotExtra == 10) app.camera.SetMode(HOOD_CAM);
            if (shotExtra == 11) app.camera.SetMode(AKIRA_CAM);
        }
    }

    // Настройка режима бенчмарка --bench (сразу запускаем 1985 Las Vegas с пальмами и городом)
    if (benchMode) {
        app.themeIdx = 1;
        app.gameMode = MODE_FREE_PLAY;
        app.resetGame();
        app.state = STATE_PLAYING;
        app.postProcess.enabled = (benchCrt != 0);
        app.bloomLevel = (benchCrt != 0) ? 1 : 0;
        app.track.ensure(0.0f, false, 4.53f, 'N', 2.0f, false);
    }

    // Таймеры для расчёта deltaTime (длительность 1 кадра) и счётчика FPS
    double lastFrame = glfwGetTime();
    double lastFpsTime = lastFrame;
    int frameCount = 0;
    int benchFrames = 0;
    double benchStart = 0.0;

    // Шаг 4: главный цикл игры — выполняется каждый кадр, пока не закроют окно
    while (!glfwWindowShouldClose(window)) {
        // Считаем сколько секунд прошло с прошлого кадра (если окно зависало, режем до 0.05с)
        double now = glfwGetTime();
        app.deltaTime = static_cast<float>(now - lastFrame);
        if (app.deltaTime > 0.05f) app.deltaTime = 0.05f;
        lastFrame = now;

        // Плавно гасим вспышки экрана и тряску камеры
        if (app.flashTimer > 0.0f) app.flashTimer = std::max(0.0f, app.flashTimer - app.deltaTime);
        if (app.shakeTimer > 0.0f) app.shakeTimer = std::max(0.0f, app.shakeTimer - app.deltaTime);
        if (app.state == STATE_GAMEOVER || app.state == STATE_PAUSED) app.shakeTimer = 0.0f;

        // Забираем флаг клика мыши на этот кадр и сразу сбрасываем
        app.clickNow = app.mouseLeftClicked;
        app.mouseLeftClicked = false;
        app.postProcess.bloomStrength = app.bloomValues[app.bloomLevel];

        // Раз в 0.25 секунды пересчитываем реальный FPS и обновляем заголовок окна + HUD
        frameCount++;
        if (now - lastFpsTime >= 0.25) {
            double fps = static_cast<double>(frameCount) / (now - lastFpsTime);
            app.currentFps = std::max(1, static_cast<int>(fps + 0.5));
            std::string title = "SYNTHWAVE SURFERS | " + app.modeShort[(int)app.gameMode] +
                                " | " + app.themes[app.themeIdx].name +
                                " | SCORE: " + std::to_string(app.score) +
                                " | FPS: " + std::to_string(app.currentFps);
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsTime = now;
        }

        // Проверяем глобальные клавиши (G, H, ESC, T, B, K, L, O, C, камеры 1-5)
        if (!benchMode) app.handleGlobalKeys(window);
        bool overlayOpen = app.showHelpOverlay || app.showSettings;

        // Для тест-скринов прыжка запускаем реальный прыжок на 36-м и двойной на 44-м кадре
        if (app.shotMode && shotExtra >= 8 && shotExtra <= 11 && app.state == STATE_PLAYING) {
            if (app.shotFrames == 36) app.delorean.jump(1.0f, true);
            if (app.shotFrames == 44) app.delorean.jump(1.0f, true);
        }

        // Обновляем логику в зависимости от текущего экрана (меню / езда / пауза / проигрыш)
        if (app.state == STATE_MENU) {
            app.updateMenu(window);
        } else if (app.state == STATE_PLAYING && !overlayOpen) {
            app.updatePlaying(window);
        } else if (!overlayOpen) {
            app.updatePausedAndGameOver(window);
        }

        // В орбитальной камере (клавиша 4) можно крутить обзор ещё и стрелками клавиатуры
        if (app.camera.Mode == ORBIT_CAM && !overlayOpen) {
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  app.camera.ProcessKeyboard(2, app.deltaTime);
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) app.camera.ProcessKeyboard(3, app.deltaTime);
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    app.camera.ProcessKeyboard(4, app.deltaTime);
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  app.camera.ProcessKeyboard(5, app.deltaTime);
        }

        // Рисуем сначала всю 3D-сцену с пост-обработкой, потом поверх неё 2D-интерфейс (HUD)
        app.render3DScene();
        app.renderHud();

        // Если запущен режим --shot, на 55-м кадре сохраняем картинку в BMP и выходим
        if (app.shotMode && ++app.shotFrames == 55) {
            app.saveBmpScreenshot(app.shotPath);
            glfwSetWindowShouldClose(window, true);
        }

        // Если запущен --bench, после 20 кадров прогрева замеряем средний FPS и печатаем в консоль
        if (benchMode) {
            if (benchFrames == 20) benchStart = glfwGetTime();
            benchFrames++;
            if (benchFrames >= benchTargetFrames + 20) {
                double elapsed = glfwGetTime() - benchStart;
                double avgFps = static_cast<double>(benchTargetFrames) / std::max(elapsed, 0.0001);
                double avgMs = 1000.0 / std::max(avgFps, 0.001);
                std::cout << "GL_RENDERER=" << reinterpret_cast<const char*>(glGetString(GL_RENDERER)) << "\n"
                          << "GL_VERSION=" << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << "\n"
                          << "RES=" << initW << "x" << initH << " CRT=" << benchCrt
                          << " AVG_FPS=" << avgFps << " FRAME_MS=" << avgMs << std::endl;
                glfwSetWindowShouldClose(window, true);
            }
        }

        // Меняем передний и задний буферы местами (двойная буферизация) и читаем новые события окна
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Аккуратно закрываем GLFW перед выходом
    gApp = nullptr;
    glfwTerminate();
    return 0;
}
