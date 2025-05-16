#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "raylib.h"
#include "ui/UIManager.h"
#include "simulation/CircuitSimulator.h"
#include <memory>

/**
 * Handles user input
 */
class InputHandler {
private:
    std::shared_ptr<CircuitSimulator> simulator;
    UIManager* uiManager;

public:
    InputHandler(std::shared_ptr<CircuitSimulator> sim, UIManager* ui);
    void processInput();
    void handleLeftMouseButtonPress(Vector2 rawMousePos, Vector2 worldMousePos);
    void handleLeftMouseButtonRelease(Vector2 worldMousePos);
    GatePin* findPinUnderMouse(Vector2 worldMousePos);
    void handleRightMouseButtonPress();
    void handleKeyboardInput();
    bool handlePaletteClick(Vector2 rawMousePos);
    bool handleCanvasClick(Vector2 worldMousePos);
    bool handleWireCompletion(Vector2 worldMousePos);
    bool handleGateAndWireInteraction(Vector2 worldMousePos);
    bool handleGatePlacement(Vector2 worldMousePos);
    bool handlePaletteDragStart(Vector2 rawMousePos);
    void handlePaletteDrag(Vector2 rawMousePos);
    bool handlePaletteDrop(Vector2 rawMousePos, Vector2 worldMousePos);
};

#endif // INPUT_HANDLER_H
