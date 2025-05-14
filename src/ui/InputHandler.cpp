#include "ui/InputHandler.h"
#include "app/Config.h"
#include "core/InputSource.h"
#include "raymath.h"

InputHandler::InputHandler(std::shared_ptr<CircuitSimulator> sim, UIManager* ui)
    : simulator(sim), uiManager(ui) {
}

void InputHandler::processInput() {
    Vector2 rawMousePos = GetMousePosition();
    Vector2 worldMousePos = GetScreenToWorld2D(rawMousePos, uiManager->getCamera());

    // Handle component dragging
    if (uiManager->getSelectedComponent() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        uiManager->updateDragging(worldMousePos);
    }

    // Handle wire point dragging
    if (uiManager->isDraggingWirePointActive() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        uiManager->updateWirePointDragging(worldMousePos);
    }

    // Handle palette drag-and-drop
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // If we're already dragging from the palette, update the drag position
        if (uiManager->getPaletteManager().isDraggingGateActive()) {
            handlePaletteDrag(rawMousePos);
        }
    }

    // Handle panning with middle or right mouse button
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
        (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !uiManager->isDrawingWireActive())) {

        // Start panning if not already panning
        if (!uiManager->isPanningActive()) {
            uiManager->startPanning(rawMousePos);
        } else {
            uiManager->updatePanning(rawMousePos);
        }
    } else if (uiManager->isPanningActive()) {
        // Stop panning when mouse button is released
        uiManager->stopPanning();
    }

    // Update wire preview position if drawing a wire
    if (uiManager->isDrawingWireActive()) {
        uiManager->updateWirePreview(worldMousePos);

        // Check for potential connection pins
        GatePin* hoverPin = findPinUnderMouse(worldMousePos);
        if (hoverPin && hoverPin->getType() == PinType::INPUT_PIN && !hoverPin->isConnectedInput()) {
            // Show visual feedback for valid connection
            // This is handled in GateRenderer::renderWirePreview
        }
    }

    // Handle mouse button press
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Check if we're starting to drag from the palette
        if (handlePaletteDragStart(rawMousePos)) {
            return;
        }

        handleLeftMouseButtonPress(rawMousePos, worldMousePos);
    }

    // Handle mouse button release
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        // If we're dragging from the palette, handle the drop
        if (uiManager->getPaletteManager().isDraggingGateActive()) {
            handlePaletteDrop(rawMousePos, worldMousePos);
            return;
        }

        handleLeftMouseButtonRelease(worldMousePos);
    }

    // Handle right mouse button or escape key
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ESCAPE)) {
        // If we're dragging from the palette, cancel the drag
        if (uiManager->getPaletteManager().isDraggingGateActive()) {
            uiManager->getPaletteManager().cancelDraggingGate();
            return;
        }

        // Only handle right click as cancel if we're not using it for panning
        if (!uiManager->isPanningActive()) {
            handleRightMouseButtonPress();
        }
    }

    // Handle keyboard input
    handleKeyboardInput();
}

void InputHandler::handleLeftMouseButtonPress(Vector2 rawMousePos, Vector2 worldMousePos) {
    // Check if clicked on palette
    if (handlePaletteClick(rawMousePos)) {
        return;
    }

    // Check if clicked on canvas
    if (CheckCollisionPointRec(rawMousePos, uiManager->getCanvasBounds())) {
        // First, check if clicked on a pin
        GatePin* clickedPin = findPinUnderMouse(worldMousePos);
        if (clickedPin) {
            // If drawing a wire and clicked on an input pin, try to complete the wire
            if (uiManager->isDrawingWireActive() && clickedPin->getType() == PinType::INPUT_PIN) {
                uiManager->completeWireDrawing(clickedPin);
                return;
            }

            // If clicked on an output pin, start drawing a wire
            if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                uiManager->startDrawingWire(clickedPin);
                return;
            }
        }

        // If drawing a wire but didn't click on a pin, cancel wire drawing
        if (uiManager->isDrawingWireActive()) {
            uiManager->cancelWireDrawing();
            return;
        }

        // Otherwise, handle interaction with gates and wires
        if (handleGateAndWireInteraction(worldMousePos)) {
            return;
        }

        // Just deselect if clicked on empty canvas
        uiManager->deselectAll();
    }
}

void InputHandler::handleLeftMouseButtonRelease(Vector2 worldMousePos) {
    // If we're drawing a wire, try to complete it
    if (uiManager->isDrawingWireActive()) {
        GatePin* endPin = findPinUnderMouse(worldMousePos);
        if (endPin && endPin->getType() == PinType::INPUT_PIN) {
            uiManager->completeWireDrawing(endPin);
        }
    }

    // Check if we need to toggle an input source
    // Only toggle if it was a click (not a drag)
    InputSource* clickedInput = dynamic_cast<InputSource*>(uiManager->getSelectedComponent());
    if (clickedInput && !uiManager->wasDragged(worldMousePos)) {
        clickedInput->toggleState();
    }

    // Stop component dragging
    uiManager->stopDragging();

    // Stop wire point dragging
    uiManager->stopWirePointDragging();
}

GatePin* InputHandler::findPinUnderMouse(Vector2 worldMousePos) {
    // Check all gates for pins under the mouse
    for (const auto& gate : simulator->getGates()) {
        // Check input pins
        for (size_t i = 0; i < gate->getInputPinCount(); i++) {
            GatePin* pin = gate->getInputPin(i);
            if (pin && pin->isMouseOverPin(worldMousePos)) {
                return pin;
            }
        }

        // Check output pins
        for (size_t i = 0; i < gate->getOutputPinCount(); i++) {
            GatePin* pin = gate->getOutputPin(i);
            if (pin && pin->isMouseOverPin(worldMousePos)) {
                return pin;
            }
        }
    }

    return nullptr;
}

void InputHandler::handleRightMouseButtonPress() {
    // Only cancel wire drawing and deselect if we're not panning
    if (!uiManager->isPanningActive()) {
        uiManager->cancelWireDrawing();
        uiManager->deselectAll();
        uiManager->getPaletteManager().setSelectedGateType(GateType::NONE);
    }
}

void InputHandler::handleKeyboardInput() {
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
        uiManager->deleteSelected();
    }
}

bool InputHandler::handlePaletteClick(Vector2 rawMousePos) {
    bool clickedOnPalette = uiManager->getPaletteManager().handleClick(rawMousePos);

    if (clickedOnPalette) {
        uiManager->cancelWireDrawing();
        uiManager->deselectAll();
    }

    return clickedOnPalette;
}

bool InputHandler::handleCanvasClick(Vector2 worldMousePos) {
    return handleGateAndWireInteraction(worldMousePos) || handleGatePlacement(worldMousePos);
}

bool InputHandler::handleWireCompletion(Vector2 worldMousePos) {
    // Find pin under mouse
    GatePin* endPin = nullptr;
    for (const auto& gate : simulator->getGates()) {
        for (size_t i = 0; i < gate->getInputPinCount(); i++) {
            GatePin* pin = gate->getInputPin(i);
            if (pin && pin->isMouseOverPin(worldMousePos)) {
                endPin = pin;
                break;
            }
        }
        if (endPin) break;
    }

    if (endPin) {
        return uiManager->completeWireDrawing(endPin);
    }

    return false;
}

bool InputHandler::handleGateAndWireInteraction(Vector2 worldMousePos) {
    // Check for interaction with gates
    for (const auto& gate : simulator->getGates()) {
        // Check if clicked on a pin first (pins have priority over gate body)
        GatePin* clickedPin = gate->getPinAt(worldMousePos);
        if (clickedPin) {
            if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                // Start drawing a wire from this output pin
                uiManager->startDrawingWire(clickedPin);
                return true;
            } else if (clickedPin->getType() == PinType::INPUT_PIN) {
                // If there's already a wire connected to this input pin, select it
                if (clickedPin->isConnectedInput()) {
                    // Find the wire connected to this pin
                    for (const auto& wire : simulator->getWires()) {
                        if (wire->getDestPin() == clickedPin) {
                            uiManager->selectWire(wire.get());
                            return true;
                        }
                    }
                }

                // Otherwise, select the gate
                uiManager->selectComponent(gate.get());
                return true;
            }
        }

        // Check if clicked on gate body
        if (gate->isMouseOver(worldMousePos)) {
            uiManager->selectComponent(gate.get());
            uiManager->startDraggingComponent(gate.get(), worldMousePos);

            // Special handling for InputSource - mark it for potential toggle
            // We'll only toggle if it's a click, not a drag
            InputSource* inputSrc = dynamic_cast<InputSource*>(gate.get());
            if (inputSrc) {
                uiManager->setClickedInputSource(inputSrc);
            }

            return true;
        }
    }

    // Check for interaction with wires
    for (const auto& wire : simulator->getWires()) {
        if (wire->isMouseOver(worldMousePos, Config::WIRE_HOVER_TOLERANCE)) {
            // Select the wire
            uiManager->selectWire(wire.get());

            // Try to start dragging a control point if the wire is already selected
            if (uiManager->getSelectedWire() == wire.get()) {
                uiManager->startDraggingWirePoint(worldMousePos);
            }

            return true;
        }
    }

    return false;
}

bool InputHandler::handleGatePlacement(Vector2 worldMousePos) {
    // This method is kept for compatibility but is no longer used for direct placement
    // Gates are now placed only via drag-and-drop
    return false;
}

bool InputHandler::handlePaletteDragStart(Vector2 rawMousePos) {
    // Check if we're clicking on a palette item
    if (uiManager->getPaletteManager().startDraggingGate(rawMousePos)) {
        // Cancel any active wire drawing or selection
        uiManager->cancelWireDrawing();
        uiManager->deselectAll();
        return true;
    }
    return false;
}

void InputHandler::handlePaletteDrag(Vector2 rawMousePos) {
    // Update the position of the gate being dragged
    uiManager->getPaletteManager().updateDragPosition(rawMousePos);
}

bool InputHandler::handlePaletteDrop(Vector2 rawMousePos, Vector2 worldMousePos) {
    // Only place a gate if we're dropping on the canvas
    if (CheckCollisionPointRec(rawMousePos, uiManager->getCanvasBounds())) {
        // The PaletteManager will use its stored snapped position internally
        // so we don't need to pass it again
        LogicGate* newGate = uiManager->getPaletteManager().endDraggingGate(worldMousePos);
        if (newGate) {
            // Select the newly placed gate
            uiManager->selectComponent(newGate);
            return true;
        }
    } else {
        // If dropped outside the canvas, cancel the drag
        uiManager->getPaletteManager().cancelDraggingGate();
    }
    return false;
}
