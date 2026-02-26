#include "ui/InteractionController.h"

#include "app/Config.h"
#include "core/InputSource.h"
#include "ui/GateGeometry.h"
#include "ui/InteractionHelpers.h"
#include "ui/UIManager.h"
#include "ui/commands/AddGateCommand.h"
#include "ui/commands/AddWireCommand.h"
#include "ui/commands/DeleteSelectionCommand.h"
#include "ui/commands/DuplicateSelectionCommand.h"
#include "ui/commands/MoveGatesCommand.h"

#include <raymath.h>
#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace {

Rectangle normalizeRect(Rectangle rect) {
    if (rect.width < 0.0f) {
        rect.x += rect.width;
        rect.width = -rect.width;
    }
    if (rect.height < 0.0f) {
        rect.y += rect.height;
        rect.height = -rect.height;
    }
    return rect;
}

int outputPinIndex(const LogicGate* gate, const GatePin* pin) {
    if (!gate || !pin) {
        return -1;
    }
    for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
        if (gate->getOutputPin(i) == pin) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int inputPinIndex(const LogicGate* gate, const GatePin* pin) {
    if (!gate || !pin) {
        return -1;
    }
    for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
        if (gate->getInputPin(i) == pin) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

} // namespace

InteractionController::InteractionController(std::shared_ptr<CircuitSimulator> simulator, UIManager* uiManager)
    : simulator_(std::move(simulator)),
      uiManager_(uiManager),
      mode_(Mode::Idle),
      isPanning_(false),
      lastPanMousePos_({0.0f, 0.0f}),
      dragPending_(false),
      isDraggingGates_(false),
      dragStartMouseWorld_({0.0f, 0.0f}),
      isDraggingWirePoint_(false),
      marqueeAdditive_(false),
      wireStartPin_(nullptr) {
    setMode(Mode::Idle);
}

void InteractionController::processInput() {
    if (!uiManager_ || !simulator_) {
        return;
    }

    if (isPrimaryModifierDown() && IsKeyPressed(KEY_K)) {
        uiManager_->getCommandPalette().toggle();
    }

    if (uiManager_->getCommandPalette().isOpen()) {
        const std::optional<std::string> action = uiManager_->getCommandPalette().handleInput();
        if (action.has_value()) {
            handleCommandPaletteAction(action.value());
        }
        uiManager_->setHovered(nullptr, nullptr);
        uiManager_->setInteractionModeLabel("CommandPalette");
        return;
    }

    Vector2 rawMousePos = GetMousePosition();
    Camera2D& cam = uiManager_->getCamera();
    Vector2 worldMousePos = GetScreenToWorld2D(rawMousePos, cam);

    if (uiManager_->isPointInCanvas(rawMousePos)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            Vector2 before = GetScreenToWorld2D(rawMousePos, cam);
            cam.zoom += wheel * 0.1f;
            cam.zoom = std::clamp(cam.zoom,
                                  uiManager_->getTokens().metrics.zoomMin,
                                  uiManager_->getTokens().metrics.zoomMax);
            Vector2 after = GetScreenToWorld2D(rawMousePos, cam);
            cam.target.x += before.x - after.x;
            cam.target.y += before.y - after.y;
        }
    }

    LogicGate* hoveredGate = nullptr;
    Wire* hoveredWire = nullptr;
    if (uiManager_->isPointInCanvas(rawMousePos)) {
        hoveredGate = findGateUnderMouse(worldMousePos);
        if (!hoveredGate) {
            hoveredWire = findWireUnderMouse(worldMousePos);
        }
    }
    uiManager_->setHovered(hoveredGate, hoveredWire);

    bool panningRequested = IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
                            (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !uiManager_->getWirePreviewState().active);
    if (panningRequested) {
        if (!isPanning_) {
            isPanning_ = true;
            lastPanMousePos_ = rawMousePos;
        } else {
            Vector2 delta = {rawMousePos.x - lastPanMousePos_.x, rawMousePos.y - lastPanMousePos_.y};
            cam.target.x -= delta.x / cam.zoom;
            cam.target.y -= delta.y / cam.zoom;
            lastPanMousePos_ = rawMousePos;
        }
        setMode(Mode::Pan);
    } else {
        isPanning_ = false;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (uiManager_->getPaletteManager().startDraggingGate(rawMousePos)) {
            clearSelection();
            clearPressCapture();
            uiManager_->clearWirePreview();
            setMode(Mode::PaletteDrag);
            return;
        }

        if (!uiManager_->isPointInCanvas(rawMousePos)) {
            clearPressCapture();
            return;
        }

        GatePin* clickedPin = findPinUnderMouse(worldMousePos);
        if (clickedPin) {
            if (clickedPin->getType() == PinType::OUTPUT_PIN) {
                wireStartPin_ = clickedPin;
                WirePreviewState& wirePreview = uiManager_->getWirePreviewState();
                wirePreview.active = true;
                wirePreview.start = clickedPin->getAbsolutePosition();
                wirePreview.end = worldMousePos;
                wirePreview.validTarget = false;
                setMode(Mode::WireDraw);
                clearPressCapture();
                return;
            }

            if (clickedPin->getType() == PinType::INPUT_PIN && clickedPin->isConnectedInput()) {
                for (auto wireIt = simulator_->getWires().rbegin(); wireIt != simulator_->getWires().rend(); ++wireIt) {
                    if ((*wireIt)->getDestPin() == clickedPin) {
                        selectSingleWire(wireIt->get());
                        if ((*wireIt)->startDraggingPoint(worldMousePos)) {
                            isDraggingWirePoint_ = true;
                            setMode(Mode::WirePointDrag);
                        }
                        clearPressCapture();
                        return;
                    }
                }
            }
        }

        LogicGate* clickedGate = findGateUnderMouse(worldMousePos);
        if (clickedGate) {
            if (isShiftDown()) {
                uiManager_->getSelection().toggleGate(clickedGate);
            } else if (!uiManager_->getSelection().containsGate(clickedGate)) {
                selectSingleGate(clickedGate);
            }

            beginGatePress(clickedGate, worldMousePos);
            return;
        }

        Wire* clickedWire = findWireUnderMouse(worldMousePos);
        if (clickedWire) {
            if (isShiftDown()) {
                uiManager_->getSelection().toggleWire(clickedWire);
            } else {
                selectSingleWire(clickedWire);
            }

            if (uiManager_->getSelection().wires.size() == 1 && uiManager_->getSelection().gates.empty()) {
                if (clickedWire->startDraggingPoint(worldMousePos)) {
                    isDraggingWirePoint_ = true;
                    setMode(Mode::WirePointDrag);
                }
            }

            clearPressCapture();
            return;
        }

        if (isShiftDown()) {
            beginMarquee(worldMousePos);
            return;
        }

        clearSelection();
        clearPressCapture();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && uiManager_->getPaletteManager().isDraggingGateActive()) {
        uiManager_->getPaletteManager().updateDragPosition(rawMousePos);
        setMode(Mode::PaletteDrag);
    }

    if (uiManager_->getWirePreviewState().active) {
        WirePreviewState& wirePreview = uiManager_->getWirePreviewState();
        wirePreview.end = worldMousePos;

        GatePin* hoverPin = findPinUnderMouse(worldMousePos);
        wirePreview.validTarget = hoverPin && wireStartPin_ &&
                                  hoverPin != wireStartPin_ &&
                                  hoverPin->getType() == PinType::INPUT_PIN &&
                                  !hoverPin->isConnectedInput();
        setMode(Mode::WireDraw);
    }

    if (dragPending_ || isDraggingGates_) {
        updateGateDrag(worldMousePos);
    }

    if (isDraggingWirePoint_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (uiManager_->getSelection().wires.size() == 1) {
            uiManager_->getSelection().wires.front()->updateDraggedPoint(worldMousePos);
            setMode(Mode::WirePointDrag);
        }
    }

    if (uiManager_->getMarqueeState().active) {
        updateMarquee(worldMousePos);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (uiManager_->getPaletteManager().isDraggingGateActive()) {
            const PaletteDragPreviewState preview = uiManager_->buildPaletteDragPreviewState();
            if (preview.active && preview.inCanvas) {
                const Vector2 dropPos = preview.snapApplied ? preview.worldSnapped : preview.worldRaw;
                GateKind kind = PaletteManager::toGateKind(uiManager_->getPaletteManager().getDraggedGateType());
                auto cmd = std::make_unique<AddGateCommand>(simulator_, kind, dropPos);
                AddGateCommand* cmdPtr = cmd.get();
                uiManager_->getCommandStack().execute(std::move(cmd));
                if (!cmdPtr->gateId().empty()) {
                    LogicGate* createdGate = simulator_->findGateById(cmdPtr->gateId());
                    if (createdGate) {
                        selectSingleGate(createdGate);
                    }
                }
            }
            uiManager_->getPaletteManager().cancelDraggingGate();
            setMode(Mode::Idle);
            clearPressCapture();
            return;
        }

        if (uiManager_->getMarqueeState().active) {
            finishMarquee();
        }

        if (isDraggingGates_ || dragPending_) {
            finishGateDrag();
        }

        if (uiManager_->getWirePreviewState().active) {
            GatePin* endPin = findPinUnderMouse(worldMousePos);
            if (wireStartPin_ && endPin &&
                endPin->getType() == PinType::INPUT_PIN &&
                !endPin->isConnectedInput() &&
                endPin != wireStartPin_) {
                LogicGate* sourceGate = wireStartPin_->getParentGate();
                LogicGate* destGate = endPin->getParentGate();
                int sourceIndex = outputPinIndex(sourceGate, wireStartPin_);
                int destIndex = inputPinIndex(destGate, endPin);
                if (sourceGate && destGate && sourceIndex >= 0 && destIndex >= 0) {
                    uiManager_->getCommandStack().execute(std::make_unique<AddWireCommand>(
                        simulator_,
                        sourceGate->getId(),
                        sourceIndex,
                        destGate->getId(),
                        destIndex));
                }
            }
            uiManager_->clearWirePreview();
            wireStartPin_ = nullptr;
        }

        if (isDraggingWirePoint_) {
            if (uiManager_->getSelection().wires.size() == 1) {
                uiManager_->getSelection().wires.front()->stopDraggingPoint();
            }
            isDraggingWirePoint_ = false;
        }

        if (pressCapture_.valid && InteractionHelpers::isClickWithinThreshold(pressCapture_.worldPos, worldMousePos)) {
            LogicGate* releasedGate = findGateUnderMouse(worldMousePos);
            if (releasedGate == pressCapture_.gate && releasedGate && releasedGate->getKind() == GateKind::INPUT_SOURCE) {
                static_cast<InputSource*>(releasedGate)->toggleState();
            }
        }

        clearPressCapture();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ESCAPE)) {
        if (uiManager_->getPaletteManager().isDraggingGateActive()) {
            uiManager_->getPaletteManager().cancelDraggingGate();
        }

        if (uiManager_->getWirePreviewState().active) {
            uiManager_->clearWirePreview();
            wireStartPin_ = nullptr;
        } else if (uiManager_->getMarqueeState().active) {
            uiManager_->getMarqueeState().active = false;
        } else {
            clearSelection();
        }

        clearPressCapture();
        setMode(Mode::Idle);
    }

    handleShortcuts();

    if (mode_ != Mode::Pan &&
        mode_ != Mode::PaletteDrag &&
        mode_ != Mode::WireDraw &&
        mode_ != Mode::WirePointDrag &&
        mode_ != Mode::Marquee &&
        !dragPending_ &&
        !isDraggingGates_) {
        setMode(Mode::Idle);
    }
}

void InteractionController::setMode(Mode mode) {
    mode_ = mode;
    uiManager_->setInteractionModeLabel(modeName(mode));
}

const char* InteractionController::modeName(Mode mode) const {
    switch (mode) {
        case Mode::Idle: return "Idle";
        case Mode::PaletteDrag: return "PaletteDrag";
        case Mode::WireDraw: return "WireDraw";
        case Mode::GatePressPending: return "GatePressPending";
        case Mode::GateDrag: return "GateDrag";
        case Mode::WirePointDrag: return "WirePointDrag";
        case Mode::Pan: return "Pan";
        case Mode::Marquee: return "Marquee";
    }

    return "Idle";
}

bool InteractionController::isPrimaryModifierDown() const {
    return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
           IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
}

bool InteractionController::isShiftDown() const {
    return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
}

bool InteractionController::isAltDown() const {
    return IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
}

GatePin* InteractionController::findPinUnderMouse(Vector2 worldMousePos) const {
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

LogicGate* InteractionController::findGateUnderMouse(Vector2 worldMousePos) const {
    for (auto gateIt = simulator_->getGates().rbegin(); gateIt != simulator_->getGates().rend(); ++gateIt) {
        LogicGate* gate = gateIt->get();
        if (GateGeometry::hitTestBody(gate->getKind(), gate->getBounds(), worldMousePos)) {
            return gate;
        }
    }

    return nullptr;
}

Wire* InteractionController::findWireUnderMouse(Vector2 worldMousePos) const {
    for (auto wireIt = simulator_->getWires().rbegin(); wireIt != simulator_->getWires().rend(); ++wireIt) {
        if ((*wireIt)->isMouseOver(worldMousePos, Config::WIRE_HOVER_TOLERANCE)) {
            return wireIt->get();
        }
    }

    return nullptr;
}

void InteractionController::beginGatePress(LogicGate* gate, Vector2 worldMousePos) {
    if (!gate) {
        return;
    }

    dragPending_ = true;
    isDraggingGates_ = false;
    dragStartMouseWorld_ = worldMousePos;

    draggedGateIds_.clear();
    dragFromPositions_.clear();

    std::vector<LogicGate*> gatesToMove = uiManager_->getSelection().gates;
    if (gatesToMove.empty()) {
        gatesToMove.push_back(gate);
    }

    for (LogicGate* selectedGate : gatesToMove) {
        if (!selectedGate) {
            continue;
        }

        simulator_->bringGateToFront(selectedGate);
        draggedGateIds_.push_back(selectedGate->getId());
        dragFromPositions_.push_back(selectedGate->getPosition());
    }

    if (gate->getKind() == GateKind::INPUT_SOURCE) {
        pressCapture_.gate = gate;
        pressCapture_.worldPos = worldMousePos;
        pressCapture_.valid = true;
    } else {
        clearPressCapture();
    }

    setMode(Mode::GatePressPending);
}

void InteractionController::updateGateDrag(Vector2 worldMousePos) {
    if (!dragPending_ && !isDraggingGates_) {
        return;
    }

    if (dragPending_ && InteractionHelpers::exceedsDragThreshold(dragStartMouseWorld_, worldMousePos)) {
        isDraggingGates_ = true;
        dragPending_ = false;
        setMode(Mode::GateDrag);
    }

    if (!isDraggingGates_) {
        return;
    }

    Vector2 delta = Vector2Subtract(worldMousePos, dragStartMouseWorld_);
    if (isShiftDown()) {
        if (fabsf(delta.x) >= fabsf(delta.y)) {
            delta.y = 0.0f;
        } else {
            delta.x = 0.0f;
        }
    }

    const float grid = uiManager_->getTokens().metrics.gridSize;
    const bool snap = uiManager_->isGridSnapEnabled() && !isAltDown();

    for (size_t i = 0; i < draggedGateIds_.size(); ++i) {
        LogicGate* gate = simulator_->findGateById(draggedGateIds_[i]);
        if (!gate) {
            continue;
        }

        Vector2 newPos = Vector2Add(dragFromPositions_[i], delta);
        if (snap) {
            newPos = InteractionHelpers::snapToGrid(newPos, grid);
        }

        gate->setPosition(newPos);
        for (Wire* wire : gate->getAssociatedWires()) {
            if (wire) {
                wire->recalculatePath();
            }
        }
    }
}

void InteractionController::finishGateDrag() {
    if (!dragPending_ && !isDraggingGates_) {
        return;
    }

    if (isDraggingGates_) {
        std::vector<Vector2> toPositions;
        std::vector<std::string> movedGateIds;
        std::vector<Vector2> fromPositions;

        for (size_t i = 0; i < draggedGateIds_.size(); ++i) {
            LogicGate* gate = simulator_->findGateById(draggedGateIds_[i]);
            if (!gate) {
                continue;
            }

            movedGateIds.push_back(draggedGateIds_[i]);
            fromPositions.push_back(dragFromPositions_[i]);
            toPositions.push_back(gate->getPosition());
        }

        if (!movedGateIds.empty()) {
            bool changed = false;
            for (size_t i = 0; i < movedGateIds.size(); ++i) {
                if (Vector2Distance(fromPositions[i], toPositions[i]) > 0.001f) {
                    changed = true;
                    break;
                }
            }

            if (changed) {
                uiManager_->getCommandStack().execute(std::make_unique<MoveGatesCommand>(
                    simulator_,
                    movedGateIds,
                    fromPositions,
                    toPositions));
            }
        }
    }

    dragPending_ = false;
    isDraggingGates_ = false;
    draggedGateIds_.clear();
    dragFromPositions_.clear();
}

void InteractionController::beginMarquee(Vector2 worldMousePos) {
    uiManager_->getMarqueeState().active = true;
    uiManager_->getMarqueeState().rect = {worldMousePos.x, worldMousePos.y, 0.0f, 0.0f};
    marqueeAdditive_ = isShiftDown();
    clearPressCapture();
    setMode(Mode::Marquee);
}

void InteractionController::updateMarquee(Vector2 worldMousePos) {
    if (!uiManager_->getMarqueeState().active) {
        return;
    }

    Rectangle& rect = uiManager_->getMarqueeState().rect;
    rect.width = worldMousePos.x - rect.x;
    rect.height = worldMousePos.y - rect.y;
    setMode(Mode::Marquee);
}

void InteractionController::finishMarquee() {
    if (!uiManager_->getMarqueeState().active) {
        return;
    }

    Rectangle rect = normalizeRect(uiManager_->getMarqueeState().rect);
    uiManager_->getMarqueeState().active = false;

    if (!marqueeAdditive_) {
        clearSelection();
    }

    for (const auto& gate : simulator_->getGates()) {
        if (CheckCollisionRecs(rect, gate->getBounds())) {
            uiManager_->getSelection().addGate(gate.get());
        }
    }

    for (const auto& wire : simulator_->getWires()) {
        const std::vector<Vector2>& path = wire->getControlPoints();
        for (const Vector2& point : path) {
            if (CheckCollisionPointRec(point, rect)) {
                uiManager_->getSelection().addWire(wire.get());
                break;
            }
        }
    }
}

void InteractionController::clearPressCapture() {
    pressCapture_.gate = nullptr;
    pressCapture_.worldPos = {0.0f, 0.0f};
    pressCapture_.valid = false;
}

void InteractionController::selectSingleGate(LogicGate* gate) {
    clearSelection();
    if (gate) {
        uiManager_->getSelection().addGate(gate);
        simulator_->bringGateToFront(gate);
    }
}

void InteractionController::selectSingleWire(Wire* wire) {
    clearSelection();
    if (wire) {
        uiManager_->getSelection().addWire(wire);
        simulator_->bringWireToFront(wire);
    }
}

void InteractionController::clearSelection() {
    uiManager_->getSelection().clear();
}

void InteractionController::handleShortcuts() {
    if (isPrimaryModifierDown() && IsKeyPressed(KEY_Z) && !isShiftDown()) {
        uiManager_->getCommandStack().undo();
    }

    if ((isPrimaryModifierDown() && IsKeyPressed(KEY_Z) && isShiftDown()) ||
        (isPrimaryModifierDown() && IsKeyPressed(KEY_Y))) {
        uiManager_->getCommandStack().redo();
    }

    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
        if (!uiManager_->getSelection().empty()) {
            uiManager_->getCommandStack().execute(
                std::make_unique<DeleteSelectionCommand>(simulator_, uiManager_->getSelection()));
            clearSelection();
        }
    }

    if (IsKeyPressed(KEY_G)) {
        uiManager_->toggleGridVisibility();
        uiManager_->setStatusText(uiManager_->isGridVisible() ? "Grid visible" : "Grid hidden");
    }

    if (IsKeyPressed(KEY_F1)) {
        uiManager_->toggleDebugOverlay();
    }

    if (isPrimaryModifierDown() && IsKeyPressed(KEY_A)) {
        clearSelection();
        for (const auto& gate : simulator_->getGates()) {
            uiManager_->getSelection().addGate(gate.get());
        }
        for (const auto& wire : simulator_->getWires()) {
            uiManager_->getSelection().addWire(wire.get());
        }
    }

    if (isPrimaryModifierDown() && IsKeyPressed(KEY_D)) {
        if (!uiManager_->getSelection().gates.empty()) {
            auto duplicateCmd = std::make_unique<DuplicateSelectionCommand>(
                simulator_,
                uiManager_->getSelection(),
                Vector2{40.0f, 30.0f});
            DuplicateSelectionCommand* cmdPtr = duplicateCmd.get();
            uiManager_->getCommandStack().execute(std::move(duplicateCmd));

            clearSelection();
            for (const std::string& id : cmdPtr->createdGateIds()) {
                LogicGate* gate = simulator_->findGateById(id);
                if (gate) {
                    uiManager_->getSelection().addGate(gate);
                }
            }
        }
    }

    if (IsKeyPressed(KEY_F)) {
        frameSelection();
    }

    Vector2 nudge = {0.0f, 0.0f};
    if (IsKeyPressed(KEY_LEFT)) nudge.x -= 1.0f;
    if (IsKeyPressed(KEY_RIGHT)) nudge.x += 1.0f;
    if (IsKeyPressed(KEY_UP)) nudge.y -= 1.0f;
    if (IsKeyPressed(KEY_DOWN)) nudge.y += 1.0f;

    if ((nudge.x != 0.0f || nudge.y != 0.0f) && !uiManager_->getSelection().gates.empty()) {
        const float step = uiManager_->getTokens().metrics.gridSize * (isShiftDown() ? 5.0f : 1.0f);
        nudge.x *= step;
        nudge.y *= step;

        std::vector<std::string> ids;
        std::vector<Vector2> from;
        std::vector<Vector2> to;
        for (LogicGate* gate : uiManager_->getSelection().gates) {
            if (!gate) {
                continue;
            }
            ids.push_back(gate->getId());
            from.push_back(gate->getPosition());
            to.push_back(Vector2Add(gate->getPosition(), nudge));
        }

        if (!ids.empty()) {
            uiManager_->getCommandStack().execute(std::make_unique<MoveGatesCommand>(
                simulator_, ids, from, to));
        }
    }
}

void InteractionController::handleCommandPaletteAction(const std::string& actionId) {
    if (actionId == "undo") {
        uiManager_->getCommandStack().undo();
    } else if (actionId == "redo") {
        uiManager_->getCommandStack().redo();
    } else if (actionId == "select_all") {
        clearSelection();
        for (const auto& gate : simulator_->getGates()) {
            uiManager_->getSelection().addGate(gate.get());
        }
        for (const auto& wire : simulator_->getWires()) {
            uiManager_->getSelection().addWire(wire.get());
        }
    } else if (actionId == "duplicate") {
        if (!uiManager_->getSelection().gates.empty()) {
            auto duplicateCmd = std::make_unique<DuplicateSelectionCommand>(
                simulator_,
                uiManager_->getSelection(),
                Vector2{40.0f, 30.0f});
            DuplicateSelectionCommand* cmdPtr = duplicateCmd.get();
            uiManager_->getCommandStack().execute(std::move(duplicateCmd));

            clearSelection();
            for (const std::string& id : cmdPtr->createdGateIds()) {
                LogicGate* gate = simulator_->findGateById(id);
                if (gate) {
                    uiManager_->getSelection().addGate(gate);
                }
            }
        }
    } else if (actionId == "delete") {
        if (!uiManager_->getSelection().empty()) {
            uiManager_->getCommandStack().execute(
                std::make_unique<DeleteSelectionCommand>(simulator_, uiManager_->getSelection()));
            clearSelection();
        }
    } else if (actionId == "frame") {
        frameSelection();
    } else if (actionId == "toggle_grid") {
        uiManager_->toggleGridVisibility();
        uiManager_->setStatusText(uiManager_->isGridVisible() ? "Grid visible" : "Grid hidden");
    } else if (actionId == "toggle_snap") {
        uiManager_->toggleGridSnap();
        uiManager_->setStatusText(uiManager_->isGridSnapEnabled() ? "Grid snap enabled" : "Grid snap disabled");
    }
}

void InteractionController::frameSelection() {
    if (uiManager_->getSelection().empty()) {
        return;
    }

    bool initialized = false;
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

    auto includePoint = [&](Vector2 p) {
        if (!initialized) {
            minX = maxX = p.x;
            minY = maxY = p.y;
            initialized = true;
            return;
        }

        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
    };

    for (LogicGate* gate : uiManager_->getSelection().gates) {
        if (!gate) {
            continue;
        }
        Rectangle b = gate->getBounds();
        includePoint({b.x, b.y});
        includePoint({b.x + b.width, b.y + b.height});
    }

    for (Wire* wire : uiManager_->getSelection().wires) {
        if (!wire) {
            continue;
        }
        for (const Vector2& p : wire->getControlPoints()) {
            includePoint(p);
        }
    }

    if (!initialized) {
        return;
    }

    Camera2D& camera = uiManager_->getCamera();
    camera.target = {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f};

    const Rectangle canvas = uiManager_->getCanvasBounds();
    const float width = std::max(1.0f, maxX - minX);
    const float height = std::max(1.0f, maxY - minY);

    const float zoomX = (canvas.width * 0.7f) / width;
    const float zoomY = (canvas.height * 0.7f) / height;
    const float fitZoom = std::min(zoomX, zoomY);

    camera.zoom = std::clamp(fitZoom,
                             uiManager_->getTokens().metrics.zoomMin,
                             uiManager_->getTokens().metrics.zoomMax);
}
