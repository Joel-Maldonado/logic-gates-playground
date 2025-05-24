#include "ui/InputHandler.h"
#include "app/Config.h"
#include "core/InputSource.h"
#include <raymath.h>

InputHandler::InputHandler(std::shared_ptr<CircuitSimulator> sim, UIManager* ui)
    : simulator_(sim), uiManager_(ui) {
}

void InputHandler::processInput() {
    Vector2 rawMousePos = GetMousePosition();
    Vector2 worldMousePos = GetScreenToWorld2D(rawMousePos, uiManager_->getCamera());

    if (uiManager_->getSelectedComponent() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        uiManager_->updateDragging(worldMousePos);
    }

    if (uiManager_->isDraggingWirePointActive() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        uiManager_->updateWirePointDragging(worldMousePos);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (uiManager_->getPaletteManager().isDraggingGateActive()) {
            handlePaletteDrag(rawMousePos);
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
        (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !uiManager_->isDrawingWireActive())) {

        if (!uiManager_->isPanningActive()) {
            uiManager_->startPanning(rawMousePos);
        } else {
            uiManager_->updatePanning(rawMousePos);
        }
    } else if (uiManager_->isPanningActive()) {
        uiManager_->stopPanning();
    }

    if (uiManager_->isDrawingWireActive()) {
        uiManager_->updateWirePreview(worldMousePos);

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
        if (uiManager_->getPaletteManager().isDraggingGateActive()) {
            handlePaletteDrop(rawMousePos, worldMousePos);
            return;
        }

        handleLeftMouseButtonRelease(worldMousePos);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ESCAPE)) {
        if (uiManager_->getPaletteManager().isDraggingGateActive()) {
            uiManager_->getPaletteManager().cancelDraggingGate();
            return;
        }

        if (!uiManager_->isPanningActive()) {
            handleRightMouseButtonPress();
        }
    }

    handleKeyboardInput();
}

void InputHandler::handleLeftMouseButtonPress(Vector2 rawMousePos, Vector2 worldMousePos) {
    if (handlePaletteClick(rawMousePos)) {
        return;
    }

    if (CheckCollisionPointRec(rawMousePos, uiManager_->getCanvasBounds())) {
        GatePin* clickedPin = findPinUnderMouse(worldMousePos);
        if (clickedPin) {
            if (uiManager_->isDrawingWireActive() && clickedPin->getType() == PinType::INPUT_PIN) {
                uiManager_->completeWireDrawing(clickedPin);
                return;
            }

            if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                uiManager_->startDrawingWire(clickedPin);
                return;
            }
        }

        if (uiManager_->isDrawingWireActive()) {
            uiManager_->cancelWireDrawing();
            return;
        }

        if (handleGateAndWireInteraction(worldMousePos)) {
            return;
        }

        uiManager_->deselectAll();
    }
}

void InputHandler::handleLeftMouseButtonRelease(Vector2 worldMousePos) {
    if (uiManager_->isDrawingWireActive()) {
        GatePin* endPin = findPinUnderMouse(worldMousePos);
        if (endPin && endPin->getType() == PinType::INPUT_PIN) {
            uiManager_->completeWireDrawing(endPin);
        }
    }

    InputSource* clickedInput = dynamic_cast<InputSource*>(uiManager_->getSelectedComponent());
    if (clickedInput && !uiManager_->wasDragged(worldMousePos)) {
        clickedInput->toggleState();
    }

    uiManager_->stopDragging();
    uiManager_->stopWirePointDragging();
}

GatePin* InputHandler::findPinUnderMouse(Vector2 worldMousePos) {
    for (const auto& gate : simulator_->getGates()) {
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
    if (!uiManager_->isPanningActive()) {
        uiManager_->cancelWireDrawing();
        uiManager_->deselectAll();
        uiManager_->getPaletteManager().setSelectedGateType(GateType::NONE);
    }
}

void InputHandler::handleKeyboardInput() {
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
        uiManager_->deleteSelected();
    }
}

bool InputHandler::handlePaletteClick(Vector2 rawMousePos) {
    bool clickedOnPalette = uiManager_->getPaletteManager().handleClick(rawMousePos);

    if (clickedOnPalette) {
        uiManager_->cancelWireDrawing();
        uiManager_->deselectAll();
    }

    return clickedOnPalette;
}

bool InputHandler::handleCanvasClick(Vector2 worldMousePos) {
    return handleGateAndWireInteraction(worldMousePos) || handleGatePlacement();
}

bool InputHandler::handleWireCompletion(Vector2 worldMousePos) {
    // Find pin under mouse
    GatePin* endPin = nullptr;
    for (const auto& gate : simulator_->getGates()) {
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
        return uiManager_->completeWireDrawing(endPin);
    }

    return false;
}

bool InputHandler::handleGateAndWireInteraction(Vector2 worldMousePos) {
    for (const auto& gate : simulator_->getGates()) {
        GatePin* clickedPin = gate->getPinAt(worldMousePos);
        if (clickedPin) {
            if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                uiManager_->startDrawingWire(clickedPin);
                return true;
            } else if (clickedPin->getType() == PinType::INPUT_PIN) {
                if (clickedPin->isConnectedInput()) {
                    for (const auto& wire : simulator_->getWires()) {
                        if (wire->getDestPin() == clickedPin) {
                            uiManager_->selectWire(wire.get());
                            return true;
                        }
                    }
                }

                uiManager_->selectComponent(gate.get());
                return true;
            }
        }

        if (gate->isMouseOver(worldMousePos)) {
            uiManager_->selectComponent(gate.get());
            uiManager_->startDraggingComponent(gate.get(), worldMousePos);

            InputSource* inputSrc = dynamic_cast<InputSource*>(gate.get());
            if (inputSrc) {
                uiManager_->setClickedInputSource(inputSrc);
            }

            return true;
        }
    }

    for (const auto& wire : simulator_->getWires()) {
        if (wire->isMouseOver(worldMousePos, Config::WIRE_HOVER_TOLERANCE)) {
            uiManager_->selectWire(wire.get());

            if (uiManager_->getSelectedWire() == wire.get()) {
                uiManager_->startDraggingWirePoint(worldMousePos);
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
    if (uiManager_->getPaletteManager().startDraggingGate(rawMousePos)) {
        uiManager_->cancelWireDrawing();
        uiManager_->deselectAll();
        return true;
    }
    return false;
}

void InputHandler::handlePaletteDrag(Vector2 rawMousePos) {
    uiManager_->getPaletteManager().updateDragPosition(rawMousePos);
}

bool InputHandler::handlePaletteDrop(Vector2 rawMousePos, Vector2 worldMousePos) {
    if (CheckCollisionPointRec(rawMousePos, uiManager_->getCanvasBounds())) {
        LogicGate* newGate = uiManager_->getPaletteManager().endDraggingGate(worldMousePos);
        if (newGate) {
            uiManager_->selectComponent(newGate);
            return true;
        }
    } else {
        uiManager_->getPaletteManager().cancelDraggingGate();
    }
    return false;
}
