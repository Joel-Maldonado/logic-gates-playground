#include "ui/InputHandler.h"
#include "app/Config.h"
#include "core/InputSource.h"
#include "ui/InteractionHelpers.h"

InputHandler::InputHandler(std::shared_ptr<CircuitSimulator> sim, UIManager* ui)
    : simulator_(std::move(sim)),
      uiManager_(ui),
      mode_(InteractionMode::IDLE),
      pressCapture_({nullptr, {0.0f, 0.0f}, false}) {
    setMode(InteractionMode::IDLE);
}

void InputHandler::processInput() {
    Vector2 rawMousePos = GetMousePosition();
    Vector2 worldMousePos = GetScreenToWorld2D(rawMousePos, uiManager_->getCamera());

    // Mouse wheel zoom (simple, clamped)
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Camera2D& cam = uiManager_->getCamera();
        Vector2 mouseWorldBefore = GetScreenToWorld2D(rawMousePos, cam);
        cam.zoom += wheel * 0.1f;
        if (cam.zoom < 0.5f) cam.zoom = 0.5f;
        if (cam.zoom > 2.5f) cam.zoom = 2.5f;
        Vector2 mouseWorldAfter = GetScreenToWorld2D(rawMousePos, cam);
        Vector2 delta = { mouseWorldBefore.x - mouseWorldAfter.x, mouseWorldBefore.y - mouseWorldAfter.y };
        cam.target.x += delta.x;
        cam.target.y += delta.y;
    }

    // Update wire hover feedback each frame in reverse order (top-most first).
    for (const auto& wire : simulator_->getWires()) {
        wire->setHovered(false);
    }

    for (auto it = simulator_->getWires().rbegin(); it != simulator_->getWires().rend(); ++it) {
        if ((*it)->isMouseOver(worldMousePos, Config::WIRE_HOVER_TOLERANCE)) {
            (*it)->setHovered(true);
            break;
        }
    }

    bool isPanningRequested = IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
                              (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !uiManager_->isDrawingWireActive());

    if (isPanningRequested) {
        if (!uiManager_->isPanningActive()) {
            uiManager_->startPanning(rawMousePos);
        } else {
            uiManager_->updatePanning(rawMousePos);
        }
        setMode(InteractionMode::PAN);
    } else if (uiManager_->isPanningActive()) {
        uiManager_->stopPanning();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && uiManager_->getPaletteManager().isDraggingGateActive()) {
        handlePaletteDrag(rawMousePos);
        setMode(InteractionMode::PALETTE_DRAG);
    }

    if (uiManager_->getSelectedComponent() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        uiManager_->tryStartDrag(worldMousePos);
        if (uiManager_->isDraggingComponentActive()) {
            uiManager_->updateDragging(worldMousePos);
            setMode(InteractionMode::GATE_DRAG);
        } else if (uiManager_->isDragPendingActive()) {
            setMode(InteractionMode::GATE_PRESS_PENDING);
        }
    }

    if (uiManager_->isDraggingWirePointActive() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        uiManager_->updateWirePointDragging(worldMousePos);
        setMode(InteractionMode::WIRE_POINT_DRAG);
    }

    if (uiManager_->isDrawingWireActive()) {
        uiManager_->updateWirePreview(worldMousePos);
        setMode(InteractionMode::WIRE_DRAW);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (handlePaletteDragStart(rawMousePos)) {
            clearPressCapture();
            return;
        }

        handleLeftMouseButtonPress(rawMousePos, worldMousePos);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (uiManager_->getPaletteManager().isDraggingGateActive()) {
            handlePaletteDrop(rawMousePos, worldMousePos);
            clearPressCapture();
            setMode(InteractionMode::IDLE);
            return;
        }

        handleLeftMouseButtonRelease(worldMousePos);

        if (uiManager_->isDrawingWireActive()) {
            GatePin* endPin = findPinUnderMouse(worldMousePos);
            if (!(endPin && endPin->getType() == PinType::INPUT_PIN && !endPin->isConnectedInput())) {
                uiManager_->cancelWireDrawing();
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ESCAPE)) {
        if (uiManager_->getPaletteManager().isDraggingGateActive()) {
            uiManager_->getPaletteManager().cancelDraggingGate();
            clearPressCapture();
            setMode(InteractionMode::IDLE);
            return;
        }

        if (!uiManager_->isPanningActive()) {
            handleRightMouseButtonPress();
            clearPressCapture();
            setMode(InteractionMode::IDLE);
        }
    }

    handleKeyboardInput();

    if (uiManager_->isPanningActive() || uiManager_->isDraggingComponentActive()) {
        SetMouseCursor(MOUSE_CURSOR_RESIZE_ALL);
    } else if (uiManager_->isDrawingWireActive()) {
        SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
    } else {
        bool overPin = (findPinUnderMouse(worldMousePos) != nullptr);
        bool overWire = false;
        for (auto it = simulator_->getWires().rbegin(); it != simulator_->getWires().rend(); ++it) {
            if ((*it)->isMouseOver(worldMousePos, Config::WIRE_HOVER_TOLERANCE)) {
                overWire = true;
                break;
            }
        }

        Rectangle paletteBounds = uiManager_->getPaletteManager().getPaletteBounds();
        bool overPalette = CheckCollisionPointRec(rawMousePos, paletteBounds);

        if (overPin || overWire || overPalette || uiManager_->getPaletteManager().isDraggingGateActive()) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        } else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
    }

    if (!uiManager_->isPanningActive() &&
        !uiManager_->isDraggingComponentActive() &&
        !uiManager_->isDrawingWireActive() &&
        !uiManager_->isDraggingWirePointActive() &&
        !uiManager_->getPaletteManager().isDraggingGateActive() &&
        !uiManager_->isDragPendingActive()) {
        setMode(InteractionMode::IDLE);
    }
}

void InputHandler::handleLeftMouseButtonPress(Vector2 rawMousePos, Vector2 worldMousePos) {
    if (handlePaletteClick(rawMousePos)) {
        clearPressCapture();
        return;
    }

    if (CheckCollisionPointRec(rawMousePos, uiManager_->getCanvasBounds())) {
        GatePin* clickedPin = findPinUnderMouse(worldMousePos);
        if (clickedPin) {
            if (uiManager_->isDrawingWireActive() && clickedPin->getType() == PinType::INPUT_PIN) {
                uiManager_->completeWireDrawing(clickedPin);
                clearPressCapture();
                return;
            }

            if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                uiManager_->startDrawingWire(clickedPin);
                clearPressCapture();
                setMode(InteractionMode::WIRE_DRAW);
                return;
            }
        }

        if (uiManager_->isDrawingWireActive()) {
            uiManager_->cancelWireDrawing();
            clearPressCapture();
            return;
        }

        if (handleGateAndWireInteraction(worldMousePos)) {
            return;
        }

        uiManager_->deselectAll();
        clearPressCapture();
    }
}

void InputHandler::handleLeftMouseButtonRelease(Vector2 worldMousePos) {
    if (uiManager_->isDrawingWireActive()) {
        GatePin* endPin = findPinUnderMouse(worldMousePos);
        if (endPin && endPin->getType() == PinType::INPUT_PIN && !endPin->isConnectedInput()) {
            uiManager_->completeWireDrawing(endPin);
        }
    }

    if (pressCapture_.valid && InteractionHelpers::isClickWithinThreshold(pressCapture_.worldPos, worldMousePos)) {
        LogicGate* releaseGate = findGateUnderMouse(worldMousePos);
        if (releaseGate == pressCapture_.gate && releaseGate && releaseGate->getKind() == GateKind::INPUT_SOURCE) {
            static_cast<InputSource*>(releaseGate)->toggleState();
        }
    }

    clearPressCapture();
    uiManager_->stopDragging();
    uiManager_->stopWirePointDragging();
}

GatePin* InputHandler::findPinUnderMouse(Vector2 worldMousePos) {
    for (auto gateIt = simulator_->getGates().rbegin(); gateIt != simulator_->getGates().rend(); ++gateIt) {
        LogicGate* gate = gateIt->get();

        for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
            GatePin* pin = gate->getInputPin(i);
            if (pin && pin->isMouseOverPin(worldMousePos)) {
                return pin;
            }
        }

        for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
            GatePin* pin = gate->getOutputPin(i);
            if (pin && pin->isMouseOverPin(worldMousePos)) {
                return pin;
            }
        }
    }

    return nullptr;
}

LogicGate* InputHandler::findGateUnderMouse(Vector2 worldMousePos) {
    for (auto gateIt = simulator_->getGates().rbegin(); gateIt != simulator_->getGates().rend(); ++gateIt) {
        if ((*gateIt)->isMouseOver(worldMousePos)) {
            return gateIt->get();
        }
    }

    return nullptr;
}

const char* InputHandler::getModeName() const {
    switch (mode_) {
        case InteractionMode::IDLE: return "Idle";
        case InteractionMode::PALETTE_DRAG: return "PaletteDrag";
        case InteractionMode::WIRE_DRAW: return "WireDraw";
        case InteractionMode::GATE_PRESS_PENDING: return "GatePressPending";
        case InteractionMode::GATE_DRAG: return "GateDrag";
        case InteractionMode::WIRE_POINT_DRAG: return "WirePointDrag";
        case InteractionMode::PAN: return "Pan";
        default: return "Idle";
    }
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

    if (IsKeyPressed(KEY_G)) {
        uiManager_->toggleGridSnap();
    }

    if (IsKeyPressed(KEY_F1)) {
        uiManager_->toggleDebugOverlay();
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

bool InputHandler::handleGateAndWireInteraction(Vector2 worldMousePos) {
    LogicGate* clickedGate = nullptr;
    GatePin* clickedPin = nullptr;

    for (auto gateIt = simulator_->getGates().rbegin(); gateIt != simulator_->getGates().rend(); ++gateIt) {
        GatePin* pin = (*gateIt)->getPinAt(worldMousePos);
        if (pin) {
            clickedGate = gateIt->get();
            clickedPin = pin;
            break;
        }
    }

    if (clickedPin && clickedGate) {
        if (clickedPin->getType() == PinType::OUTPUT_PIN) {
            uiManager_->startDrawingWire(clickedPin);
            clearPressCapture();
            setMode(InteractionMode::WIRE_DRAW);
            return true;
        }

        if (clickedPin->getType() == PinType::INPUT_PIN && clickedPin->isConnectedInput()) {
            for (auto wireIt = simulator_->getWires().rbegin(); wireIt != simulator_->getWires().rend(); ++wireIt) {
                if ((*wireIt)->getDestPin() == clickedPin) {
                    uiManager_->selectWire(wireIt->get());
                    clearPressCapture();
                    return true;
                }
            }
        }

        uiManager_->selectComponent(clickedGate);
        clearPressCapture();
        return true;
    }

    LogicGate* topGate = findGateUnderMouse(worldMousePos);
    if (topGate) {
        uiManager_->selectComponent(topGate);
        uiManager_->startDraggingComponent(topGate, worldMousePos);

        if (topGate->getKind() == GateKind::INPUT_SOURCE) {
            pressCapture_.gate = topGate;
            pressCapture_.worldPos = worldMousePos;
            pressCapture_.valid = true;
            setMode(InteractionMode::GATE_PRESS_PENDING);
        } else {
            clearPressCapture();
        }

        return true;
    }

    for (auto wireIt = simulator_->getWires().rbegin(); wireIt != simulator_->getWires().rend(); ++wireIt) {
        if ((*wireIt)->isMouseOver(worldMousePos, Config::WIRE_HOVER_TOLERANCE)) {
            uiManager_->selectWire(wireIt->get());

            if (uiManager_->getSelectedWire() == wireIt->get() &&
                uiManager_->startDraggingWirePoint(worldMousePos)) {
                setMode(InteractionMode::WIRE_POINT_DRAG);
            }

            clearPressCapture();
            return true;
        }
    }

    return false;
}

bool InputHandler::handlePaletteDragStart(Vector2 rawMousePos) {
    if (uiManager_->getPaletteManager().startDraggingGate(rawMousePos)) {
        uiManager_->cancelWireDrawing();
        uiManager_->deselectAll();
        clearPressCapture();
        setMode(InteractionMode::PALETTE_DRAG);
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
            setMode(InteractionMode::IDLE);
            return true;
        }
    } else {
        uiManager_->getPaletteManager().cancelDraggingGate();
    }

    setMode(InteractionMode::IDLE);
    return false;
}

void InputHandler::setMode(InteractionMode mode) {
    mode_ = mode;
    uiManager_->setInteractionModeLabel(getModeName());
}

void InputHandler::clearPressCapture() {
    pressCapture_.gate = nullptr;
    pressCapture_.worldPos = {0.0f, 0.0f};
    pressCapture_.valid = false;
}
