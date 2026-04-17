#ifndef APPLICATION_H
#define APPLICATION_H

#include "simulation/CircuitSimulator.h"
#include "ui/UIManager.h"
#include <memory>

class Application {
private:
    std::shared_ptr<CircuitSimulator> simulator;
    std::unique_ptr<UIManager> uiManager;
    bool isRunning;
    int prevWindowWidth;
    int prevWindowHeight;

public:
    Application();
    void initialize();
    void run();
    void update();
    void render();
    void processInput();
    void handleWindowResize();
    void cleanup();
};

#endif // APPLICATION_H
