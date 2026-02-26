#include "app/Application.h"
#include "app/Config.h"
#include <raylib.h>

Application::Application()
    : isRunning(false),
      prevWindowWidth(Config::SCREEN_WIDTH),
      prevWindowHeight(Config::SCREEN_HEIGHT) {
    simulator = std::make_shared<CircuitSimulator>();
}

void Application::initialize() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Config::WINDOW_TITLE);
    SetTargetFPS(Config::TARGET_FPS);

    uiManager = std::make_unique<UIManager>(simulator);
    uiManager->initialize();

    isRunning = true;
}

void Application::run() {
    while (isRunning && !WindowShouldClose()) {
        handleWindowResize();
        processInput();
        update();
        render();
    }

    cleanup();
}

void Application::update() {
    simulator->update();
}

void Application::render() {
    uiManager->render();
}

void Application::processInput() {
    uiManager->processInput();
}

void Application::handleWindowResize() {
    int currentWidth = GetScreenWidth();
    int currentHeight = GetScreenHeight();

    if (currentWidth != prevWindowWidth || currentHeight != prevWindowHeight) {
        uiManager->handleWindowResize(currentWidth, currentHeight);
        prevWindowWidth = currentWidth;
        prevWindowHeight = currentHeight;
    }
}

void Application::cleanup() {
    simulator->clear();
    CloseWindow();
}
