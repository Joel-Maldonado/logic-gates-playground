#include "app/Application.h"
#include "app/Config.h"
#include "core/CustomGateRegistry.h" // Added include
#include <raylib.h>

Application::Application()
    : isRunning(false),
      prevWindowWidth(Config::SCREEN_WIDTH),
      prevWindowHeight(Config::SCREEN_HEIGHT) {
    simulator = std::make_shared<CircuitSimulator>();
    customGateRegistry_ = std::make_unique<CustomGateRegistry>("custom_gates"); // Instantiate registry
}

void Application::initialize() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Config::WINDOW_TITLE);
    SetTargetFPS(Config::TARGET_FPS);

    customGateRegistry_->loadDefinitions(); // Load definitions after window init (if paths are relative etc)

    uiManager = std::make_unique<UIManager>(simulator, customGateRegistry_.get()); // Pass registry pointer
    uiManager->initialize();
    inputHandler = std::make_unique<InputHandler>(simulator, uiManager.get());

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
    inputHandler->processInput();
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
