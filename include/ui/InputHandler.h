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
    enum class InteractionMode {
        IDLE,
        PALETTE_DRAG,
        WIRE_DRAW,
        GATE_PRESS_PENDING,
        GATE_DRAG,
        WIRE_POINT_DRAG,
        PAN
    };

    struct PressCapture {
        LogicGate* gate;
        Vector2 worldPos;
        bool valid;
    };

    std::shared_ptr<CircuitSimulator> simulator_;
    UIManager* uiManager_;
    InteractionMode mode_;
    PressCapture pressCapture_;

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
    LogicGate* findGateUnderMouse(Vector2 worldMousePos);
    const char* getModeName() const;

    /** Handles keyboard input and shortcuts */
    void handleKeyboardInput();

    // Interaction handlers
    bool handlePaletteClick(Vector2 rawMousePos);
    bool handleGateAndWireInteraction(Vector2 worldMousePos);

    // Drag and drop handlers
    bool handlePaletteDragStart(Vector2 rawMousePos);
    void handlePaletteDrag(Vector2 rawMousePos);
    bool handlePaletteDrop(Vector2 rawMousePos, Vector2 worldMousePos);

private:
    void setMode(InteractionMode mode);
    void clearPressCapture();
};

#endif // INPUT_HANDLER_H
