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
    /**
     * Constructs a new InputHandler
     *
     * @param sim Shared pointer to the circuit simulator
     * @param ui Pointer to the UI manager
     */
    InputHandler(std::shared_ptr<CircuitSimulator> sim, UIManager* ui);

    /**
     * Processes user input
     */
    void processInput();

    /**
     * Handles a left mouse button press
     *
     * @param rawMousePos Mouse position in screen coordinates
     * @param worldMousePos Mouse position in world coordinates
     */
    void handleLeftMouseButtonPress(Vector2 rawMousePos, Vector2 worldMousePos);

    /**
     * Handles a left mouse button release
     *
     * @param worldMousePos Mouse position in world coordinates
     */
    void handleLeftMouseButtonRelease(Vector2 worldMousePos);

    /**
     * Finds a pin under the mouse cursor
     *
     * @param worldMousePos Mouse position in world coordinates
     * @return Pointer to the pin under the mouse, or nullptr if none
     */
    GatePin* findPinUnderMouse(Vector2 worldMousePos);

    /**
     * Handles a right mouse button press
     */
    void handleRightMouseButtonPress();

    /**
     * Handles keyboard input
     */
    void handleKeyboardInput();

    /**
     * Handles a click on the palette
     *
     * @param rawMousePos Mouse position in screen coordinates
     * @return True if a palette item was clicked
     */
    bool handlePaletteClick(Vector2 rawMousePos);

    /**
     * Handles a click on the canvas
     *
     * @param worldMousePos Mouse position in world coordinates
     * @return True if a canvas item was clicked
     */
    bool handleCanvasClick(Vector2 worldMousePos);

    /**
     * Handles completing a wire connection
     *
     * @param worldMousePos Mouse position in world coordinates
     * @return True if a wire was completed
     */
    bool handleWireCompletion(Vector2 worldMousePos);

    /**
     * Handles interaction with gates and wires
     *
     * @param worldMousePos Mouse position in world coordinates
     * @return True if an interaction was handled
     */
    bool handleGateAndWireInteraction(Vector2 worldMousePos);

    /**
     * Handles placing a new gate
     *
     * @param worldMousePos Mouse position in world coordinates
     * @return True if a gate was placed
     */
    bool handleGatePlacement(Vector2 worldMousePos);

    /**
     * Handles starting to drag a gate from the palette
     *
     * @param rawMousePos Mouse position in screen coordinates
     * @return True if a drag operation was started
     */
    bool handlePaletteDragStart(Vector2 rawMousePos);

    /**
     * Handles dragging a gate from the palette
     *
     * @param rawMousePos Mouse position in screen coordinates
     */
    void handlePaletteDrag(Vector2 rawMousePos);

    /**
     * Handles dropping a gate from the palette
     *
     * @param rawMousePos Mouse position in screen coordinates
     * @param worldMousePos Mouse position in world coordinates
     * @return True if a gate was placed
     */
    bool handlePaletteDrop(Vector2 rawMousePos, Vector2 worldMousePos);
};

#endif // INPUT_HANDLER_H
