#include "ui/InputHandler.h"
#include "app/Config.h"
#include "core/InputSource.h"
#include <raymath.h>

InputHandler::InputHandler(std::shared_ptr<CircuitSimulator> sim, UIManager* ui)
    : simulator(sim), uiManager(ui) {
}

void InputHandler::processInput() {
    Vector2 rawMousePos = GetMousePosition();
    Vector2 worldMousePos = GetScreenToWorld2D(rawMousePos, uiManager->getCamera());

    if (uiManager->getSelectedComponent() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        uiManager->updateDragging(worldMousePos);
    }

    if (uiManager->isDraggingWirePointActive() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        uiManager->updateWirePointDragging(worldMousePos);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (uiManager->getPaletteManager().isDraggingGateActive()) {
            handlePaletteDrag(rawMousePos);
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
        (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !uiManager->isDrawingWireActive())) {

        if (!uiManager->isPanningActive()) {
            uiManager->startPanning(rawMousePos);
        } else {
            uiManager->updatePanning(rawMousePos);
        }
    } else if (uiManager->isPanningActive()) {
        uiManager->stopPanning();
    }

    if (uiManager->isDrawingWireActive()) {
        uiManager->updateWirePreview(worldMousePos);

        GatePin* hoverPin = findPinUnderMouse(worldMousePos);
        if (hoverPin && hoverPin->getType() == PinType::INPUT_PIN && !hoverPin->isConnectedInput()) {
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (handlePaletteDragStart(rawMousePos)) {
            return;
        }

        handleLeftMouseButtonPress(rawMousePos, worldMousePos);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (uiManager->getPaletteManager().isDraggingGateActive()) {
            handlePaletteDrop(rawMousePos, worldMousePos);
            return;
        }

        handleLeftMouseButtonRelease(worldMousePos);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ESCAPE)) {
        if (uiManager->getPaletteManager().isDraggingGateActive()) {
            uiManager->getPaletteManager().cancelDraggingGate();
            return;
        }

        if (!uiManager->isPanningActive()) {
            handleRightMouseButtonPress();
        }
    }

    handleKeyboardInput();
}

void InputHandler::handleLeftMouseButtonPress(Vector2 rawMousePos, Vector2 worldMousePos) {
    if (handlePaletteClick(rawMousePos)) {
        return;
    }

    if (CheckCollisionPointRec(rawMousePos, uiManager->getCanvasBounds())) {
        GatePin* clickedPin = findPinUnderMouse(worldMousePos);
        if (clickedPin) {
            if (uiManager->isDrawingWireActive() && clickedPin->getType() == PinType::INPUT_PIN) {
                uiManager->completeWireDrawing(clickedPin);
                return;
            }

            if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                uiManager->startDrawingWire(clickedPin);
                return;
            }
        }

        if (uiManager->isDrawingWireActive()) {
            uiManager->cancelWireDrawing();
            return;
        }

        if (handleGateAndWireInteraction(worldMousePos)) {
            return;
        }

        uiManager->deselectAll();
    }
}

void InputHandler::handleLeftMouseButtonRelease(Vector2 worldMousePos) {
    if (uiManager->isDrawingWireActive()) {
        GatePin* endPin = findPinUnderMouse(worldMousePos);
        if (endPin && endPin->getType() == PinType::INPUT_PIN) {
            uiManager->completeWireDrawing(endPin);
        }
    }

    InputSource* clickedInput = dynamic_cast<InputSource*>(uiManager->getSelectedComponent());
    if (clickedInput && !uiManager->wasDragged(worldMousePos)) {
        clickedInput->toggleState();
    }

    uiManager->stopDragging();
    uiManager->stopWirePointDragging();
}

GatePin* InputHandler::findPinUnderMouse(Vector2 worldMousePos) {
    for (const auto& gate : simulator->getGates()) {
        for (size_t i = 0; i < gate->getInputPinCount(); i++) {
            GatePin* pin = gate->getInputPin(i);
            if (pin && pin->isMouseOverPin(worldMousePos)) {
                return pin;
            }
        }

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
    return handleGateAndWireInteraction(worldMousePos) || handleGatePlacement();
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
    for (const auto& gate : simulator->getGates()) {
        GatePin* clickedPin = gate->getPinAt(worldMousePos);
        if (clickedPin) {
            if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                uiManager->startDrawingWire(clickedPin);
                return true;
            } else if (clickedPin->getType() == PinType::INPUT_PIN) {
                if (clickedPin->isConnectedInput()) {
                    for (const auto& wire : simulator->getWires()) {
                        if (wire->getDestPin() == clickedPin) {
                            uiManager->selectWire(wire.get());
                            return true;
                        }
                    }
                }

                uiManager->selectComponent(gate.get());
                return true;
            }
        }

        if (gate->isMouseOver(worldMousePos)) {
            uiManager->selectComponent(gate.get());
            uiManager->startDraggingComponent(gate.get(), worldMousePos);

            InputSource* inputSrc = dynamic_cast<InputSource*>(gate.get());
            if (inputSrc) {
                uiManager->setClickedInputSource(inputSrc);
            }

            return true;
        }
    }

    for (const auto& wire : simulator->getWires()) {
        if (wire->isMouseOver(worldMousePos, Config::WIRE_HOVER_TOLERANCE)) {
            uiManager->selectWire(wire.get());

            if (uiManager->getSelectedWire() == wire.get()) {
                uiManager->startDraggingWirePoint(worldMousePos);
            }

            return true;
        }
    }

    return false;
}

bool InputHandler::handleGatePlacement() {
    return false;
}

bool InputHandler::handlePaletteDragStart(Vector2 rawMousePos) {
    if (uiManager->getPaletteManager().startDraggingGate(rawMousePos)) {
        uiManager->cancelWireDrawing();
        uiManager->deselectAll();
        return true;
    }
    return false;
}

void InputHandler::handlePaletteDrag(Vector2 rawMousePos) {
    uiManager->getPaletteManager().updateDragPosition(rawMousePos);
}

bool InputHandler::handlePaletteDrop(Vector2 rawMousePos, Vector2 worldMousePos) {
    if (CheckCollisionPointRec(rawMousePos, uiManager->getCanvasBounds())) {
        LogicGate* newGate = uiManager->getPaletteManager().endDraggingGate(worldMousePos);
        if (newGate) {
            uiManager->selectComponent(newGate);
            return true;
        }
    } else {
        uiManager->getPaletteManager().cancelDraggingGate();
    }
    return false;
}
