#ifndef APPLICATION_H
#define APPLICATION_H

#include "simulation/CircuitSimulator.h"
#include "ui/UIManager.h"
#include "ui/InputHandler.h"
#include <memory>

/**
 * Main application class
 * Coordinates between UI, simulation, and core logic
 */
class Application {
private:
    std::shared_ptr<CircuitSimulator> simulator;
    std::unique_ptr<UIManager> uiManager;
    std::unique_ptr<InputHandler> inputHandler;
    bool isRunning;
    int prevWindowWidth;
    int prevWindowHeight;

public:
    /**
     * Constructs a new Application
     */
    Application();

    /**
     * Initializes the application
     */
    void initialize();

    /**
     * Runs the main application loop
     */
    void run();

    /**
     * Updates the application state
     */
    void update();

    /**
     * Renders the application
     */
    void render();

    /**
     * Processes user input
     */
    void processInput();

    /**
     * Checks if the window has been resized and updates the UI accordingly
     */
    void handleWindowResize();

    /**
     * Cleans up resources
     */
    void cleanup();
};

#endif // APPLICATION_H
