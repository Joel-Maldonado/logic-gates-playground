#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <raylib.h>
#include "ui/UIManager.h"
#include "simulation/CircuitSimulator.h"
#include <memory>

/**
 * Handles all user input events for the circuit editor.
 * Manages mouse interactions, keyboard shortcuts, and coordinates
 * between UI components and the circuit simulator.
 */
class InputHandler {
private:
    std::shared_ptr<CircuitSimulator> simulator_;
    UIManager* uiManager_;

public:
    /**
     * Constructs the input handler.
     * @param sim Shared pointer to the circuit simulator
     * @param ui Pointer to the UI manager
     */
    InputHandler(std::shared_ptr<CircuitSimulator> sim, UIManager* ui);

    /** Main input processing method called each frame */
    void processInput();

    // Mouse event handlers
    void handleLeftMouseButtonPress(Vector2 rawMousePos, Vector2 worldMousePos);
    void handleLeftMouseButtonRelease(Vector2 worldMousePos);
    void handleRightMouseButtonPress();

    /** Finds a gate pin under the mouse cursor */
    GatePin* findPinUnderMouse(Vector2 worldMousePos);

    /** Handles keyboard input and shortcuts */
    void handleKeyboardInput();

    // Interaction handlers
    bool handlePaletteClick(Vector2 rawMousePos);
    bool handleCanvasClick(Vector2 worldMousePos);
    bool handleWireCompletion(Vector2 worldMousePos);
    bool handleGateAndWireInteraction(Vector2 worldMousePos);
    bool handleGatePlacement();

    // Drag and drop handlers
    bool handlePaletteDragStart(Vector2 rawMousePos);
    void handlePaletteDrag(Vector2 rawMousePos);
    bool handlePaletteDrop(Vector2 rawMousePos, Vector2 worldMousePos);
};

#endif // INPUT_HANDLER_H
