#include "ui/UIManager.h"
#include "app/Config.h"
#include "ui/InteractionHelpers.h"
#include "ui/VisualEffects.h"
#include <raymath.h>
#include <algorithm>
#include <cmath>

UIManager::UIManager(std::shared_ptr<CircuitSimulator> sim)
    : isDrawingWire_(false),
      wireStartPin_(nullptr),
      isPanning_(false),
      panStartPosition_({0, 0}),
      lastMousePosition_({0, 0}),
      panVelocity_({0, 0}),
      panInertia_(0.9f),
      simulator_(sim),
      selectedComponent_(nullptr),
      selectedWire_(nullptr),
      isDraggingComponent_(false),
      isDraggingWirePoint_(false),
      isDragPending_(false),
      dragStartOffset_({0, 0}),
      dragStartPosition_({0, 0}),
      dragStartComponentPosition_({0, 0}) {

    camera_.target = { (float)Config::SCREEN_WIDTH / 2, (float)Config::SCREEN_HEIGHT / 2 };
    camera_.offset = { (float)Config::SCREEN_WIDTH / 2, (float)Config::SCREEN_HEIGHT / 2 };
    camera_.rotation = 0.0f;
    camera_.zoom = 1.0f;

    canvasBounds_ = {
        Config::PALETTE_WIDTH,
        0,
        (float)Config::SCREEN_WIDTH - Config::PALETTE_WIDTH,
        (float)Config::SCREEN_HEIGHT
    };

    paletteManager_ = std::make_unique<PaletteManager>(simulator_);
    gateRenderer_ = std::make_unique<GateRenderer>();
    wireRenderer_ = std::make_unique<WireRenderer>();
}

void UIManager::initialize() {
    paletteManager_->initialize();
}

void UIManager::render() {
    BeginDrawing();
    ClearBackground(Config::Colors::BACKGROUND);

    // Canvas background gradient (separates palette and world nicely)
    Rectangle canvasRect = { Config::PALETTE_WIDTH, 0.0f,
                             (float)GetScreenWidth() - Config::PALETTE_WIDTH,
                             (float)GetScreenHeight() };
    VisualEffects::drawRoundedRectangleGradient(canvasRect, 0.0f,
                                                Config::Colors::CANVAS_BG_TOP,
                                                Config::Colors::CANVAS_BG_BOTTOM);

    updateCamera();

    BeginMode2D(camera_);

    if (Config::GRID_ENABLED) {
        renderGrid();
    }

    // Alignment guides while dragging components
    renderDragAlignmentGuides();

    wireRenderer_->renderWires(simulator_->getWires());

    if (isDrawingWire_ && wireStartPin_) {
        Vector2 worldMousePos = GetScreenToWorld2D(GetMousePosition(), camera_);

        bool isOverInputPin = false;
        for (const auto& gate : simulator_->getGates()) {
            for (size_t i = 0; i < gate->getInputPinCount(); i++) {
                GatePin* pin = gate->getInputPin(i);
                if (pin && pin->isMouseOverPin(worldMousePos) && !pin->isConnectedInput()) {
                    isOverInputPin = true;
                    break;
                }
            }
            if (isOverInputPin) break;
        }

        wireRenderer_->renderWirePreview(
            wireStartPin_->getAbsolutePosition(),
            wirePreviewEndPos_,
            isOverInputPin,
            Config::Colors::WIRE_PREVIEW,
            Config::WIRE_THICKNESS_PREVIEW
        );

        gateRenderer_->renderWirePreview(
            wireStartPin_,
            simulator_->getGates(),
            worldMousePos
        );
    }

    gateRenderer_->renderGates(simulator_->getGates(), camera_);

    EndMode2D();

    paletteManager_->render(camera_);

    // Zoom and snap indicators (screen-space)
    char hudText[64];
    snprintf(hudText, sizeof(hudText), "Zoom: %d%%   Snap: %s", (int)(camera_.zoom * 100.0f),
             gridSnapEnabled_ ? "On" : "Off");
    DrawText(hudText, GetScreenWidth() - 260, GetScreenHeight() - 30, 18, Config::Colors::PALETTE_TEXT);

    DrawFPS(GetScreenWidth() - 90, 10);

    if (debugOverlayEnabled_) {
        const auto stats = simulator_->getLastStats();
        Rectangle overlay = {10.0f, 10.0f, 360.0f, 110.0f};
        DrawRectangleRounded(overlay, 0.15f, 8, Fade(Config::Colors::SURFACE_VARIANT, 0.9f));
        DrawRectangleRoundedLines(overlay, 0.15f, 8, 1.0f, Config::Colors::GATE_OUTLINE);

        char statsLine[128];
        snprintf(statsLine, sizeof(statsLine), "Passes: %d  Stable: %s  Oscillating: %s",
                 stats.passes,
                 stats.stable ? "yes" : "no",
                 stats.oscillating ? "yes" : "no");

        DrawText("Debug Overlay (F1)", 20, 18, 18, Config::Colors::PALETTE_TEXT);
        DrawText(statsLine, 20, 46, 16, Config::Colors::PALETTE_TEXT);
        DrawText(TextFormat("Interaction: %s", interactionModeLabel_.c_str()),
                 20, 72, 16, Config::Colors::PALETTE_TEXT);
        DrawText(TextFormat("Snap: %s", gridSnapEnabled_ ? "On" : "Off"),
                 20, 94, 16, Config::Colors::PALETTE_TEXT);
    }

    EndDrawing();
}

void UIManager::renderDragAlignmentGuides() {
    if (!isDraggingComponent_ || !selectedComponent_) return;

    const float SNAP_THRESHOLD = 15.0f;
    const std::vector<Wire*>& associatedWires = selectedComponent_->getAssociatedWires();
    if (associatedWires.empty()) return;

    Vector2 gatePos = selectedComponent_->getPosition();
    Color guideColor = Fade(Config::Colors::HOVER_HIGHLIGHT, 0.35f);
    float thickness = 1.0f / std::max(0.8f, camera_.zoom); // keep thin at zoomed in

    for (Wire* wire : associatedWires) {
        if (!wire) continue;
        GatePin* sourcePin = wire->getSourcePin();
        GatePin* destPin = wire->getDestPin();
        if (!sourcePin || !destPin) continue;

        LogicGate* sourceGate = sourcePin->getParentGate();
        LogicGate* destGate = destPin->getParentGate();
        if (!sourceGate || !destGate) continue;

        GatePin* thisGatePin = (selectedComponent_ == sourceGate) ? sourcePin : destPin;
        GatePin* otherGatePin = (selectedComponent_ == sourceGate) ? destPin   : sourcePin;

        Vector2 currentOffset = Vector2Subtract(thisGatePin->getAbsolutePosition(), selectedComponent_->getPosition());
        Vector2 projectedPinPos = Vector2Add(gatePos, currentOffset);
        Vector2 otherPinPos = otherGatePin->getAbsolutePosition();

        // Horizontal guide
        float yDiff = fabsf(projectedPinPos.y - otherPinPos.y);
        if (yDiff < SNAP_THRESHOLD) {
            DrawLineEx({projectedPinPos.x - 10000.0f, otherPinPos.y},
                       {projectedPinPos.x + 10000.0f, otherPinPos.y}, thickness, guideColor);
        }

        // Vertical guide
        float xDiff = fabsf(projectedPinPos.x - otherPinPos.x);
        if (xDiff < SNAP_THRESHOLD) {
            DrawLineEx({otherPinPos.x, projectedPinPos.y - 10000.0f},
                       {otherPinPos.x, projectedPinPos.y + 10000.0f}, thickness, guideColor);
        }
    }
}

void UIManager::processInput() {
}

Camera2D& UIManager::getCamera() {
    return camera_;
}

Rectangle UIManager::getCanvasBounds() const {
    return canvasBounds_;
}

void UIManager::selectComponent(LogicGate* component) {
    deselectAll();

    if (component) {
        simulator_->bringGateToFront(component);
        selectedComponent_ = component;
        selectedComponent_->setSelected(true);
    }
}

void UIManager::selectWire(Wire* wire) {
    deselectAll();

    if (wire) {
        simulator_->bringWireToFront(wire);
        selectedWire_ = wire;
        selectedWire_->setSelected(true);
    }
}

void UIManager::deselectAll() {
    if (selectedComponent_) {
        selectedComponent_->setSelected(false);
        selectedComponent_ = nullptr;
    }

    if (selectedWire_) {
        selectedWire_->setSelected(false);
        selectedWire_ = nullptr;
    }
}

void UIManager::startDrawingWire(GatePin* pin) {
    if (!pin || pin->getType() != PinType::OUTPUT_PIN) {
        return;
    }

    isDrawingWire_ = true;
    wireStartPin_ = pin;
    wirePreviewEndPos_ = pin->getAbsolutePosition();
    isDraggingComponent_ = false;
}

void UIManager::updateWirePreview(Vector2 mousePos) {
    if (!isDrawingWire_ || !wireStartPin_) {
        return;
    }

    wirePreviewEndPos_ = mousePos;
}

bool UIManager::completeWireDrawing(GatePin* pin) {
    if (!isDrawingWire_ || !wireStartPin_ || !pin) {
        return false;
    }

    if (pin == wireStartPin_ || pin->getType() != PinType::INPUT_PIN || pin->isConnectedInput()) {
        return false;
    }

    Wire* wire = simulator_->createWire(wireStartPin_, pin);

    isDrawingWire_ = false;
    wireStartPin_ = nullptr;

    return (wire != nullptr);
}

void UIManager::cancelWireDrawing() {
    isDrawingWire_ = false;
    wireStartPin_ = nullptr;
}

void UIManager::startDraggingComponent(LogicGate* component, Vector2 mousePos) {
    if (!component) {
        return;
    }

    selectComponent(component);
    // Don't start dragging immediately; set as pending and only start
    // if the cursor moves past a threshold (for less jumpy feel).
    isDragPending_ = true;
    isDraggingComponent_ = false;
    dragStartOffset_ = Vector2Subtract(mousePos, component->getPosition());
    dragStartPosition_ = mousePos;
    dragStartComponentPosition_ = component->getPosition();
}

void UIManager::updateDragging(Vector2 mousePos) {
    if (!isDraggingComponent_ || !selectedComponent_) {
        return;
    }

    Vector2 position = Vector2Subtract(mousePos, dragStartOffset_);
    bool isShiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if (isShiftDown) {
        InteractionHelpers::DragAxis axis = InteractionHelpers::determineDominantAxis(dragStartPosition_, mousePos);
        position = InteractionHelpers::applyAxisLock(position, dragStartComponentPosition_, axis);
    }

    bool snapEnabled = !(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));
    Vector2 alignedPosition = checkWireAlignmentSnapping(selectedComponent_, position, snapEnabled, snapEnabled);
    selectedComponent_->setPosition(alignedPosition);
    updateWirePathsForComponent(selectedComponent_);
}

void UIManager::stopDragging() {
    if (isDraggingComponent_ && selectedComponent_) {
        Vector2 currentPos = selectedComponent_->getPosition();
        bool snapEnabled = !(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));
        Vector2 alignedPos = checkWireAlignmentSnapping(selectedComponent_, currentPos, snapEnabled, snapEnabled);
        selectedComponent_->setPosition(alignedPos);
        updateWirePathsForComponent(selectedComponent_);
    }

    isDraggingComponent_ = false;
    isDragPending_ = false;
}

void UIManager::tryStartDrag(Vector2 mousePos) {
    if (!isDragPending_ || !selectedComponent_) return;
    if (InteractionHelpers::exceedsDragThreshold(dragStartPosition_, mousePos)) {
        isDraggingComponent_ = true;
        isDragPending_ = false;
    }
}

bool UIManager::startDraggingWirePoint(Vector2 mousePos) {
    if (!selectedWire_) {
        return false;
    }

    if (selectedWire_->startDraggingPoint(mousePos)) {
        isDraggingWirePoint_ = true;
        return true;
    }

    return false;
}

void UIManager::updateWirePointDragging(Vector2 mousePos) {
    if (!isDraggingWirePoint_ || !selectedWire_) {
        return;
    }

    selectedWire_->updateDraggedPoint(mousePos);
}

void UIManager::stopWirePointDragging() {
    if (selectedWire_) {
        selectedWire_->stopDraggingPoint();
    }
    isDraggingWirePoint_ = false;
}

bool UIManager::isDraggingWirePointActive() const {
    return isDraggingWirePoint_;
}

void UIManager::deleteSelected() {
    if (selectedComponent_) {
        simulator_->removeGate(selectedComponent_);
        selectedComponent_ = nullptr;
    } else if (selectedWire_) {
        simulator_->removeWire(selectedWire_);
        selectedWire_ = nullptr;
    }
}

LogicGate* UIManager::getSelectedComponent() const {
    return selectedComponent_;
}

Wire* UIManager::getSelectedWire() const {
    return selectedWire_;
}

bool UIManager::isDrawingWireActive() const {
    return isDrawingWire_;
}

PaletteManager& UIManager::getPaletteManager() {
    return *paletteManager_;
}

bool UIManager::wasDragged(Vector2 currentMousePos) const {
    return InteractionHelpers::exceedsDragThreshold(dragStartPosition_, currentMousePos);
}

void UIManager::renderGrid() {
    Vector2 screenTopLeft = GetScreenToWorld2D({0, 0}, camera_);
    Vector2 screenBottomRight = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, camera_);

    int startX = floor(screenTopLeft.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    int startY = floor(screenTopLeft.y / Config::GRID_SIZE) * Config::GRID_SIZE;
    int endX = ceil(screenBottomRight.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    int endY = ceil(screenBottomRight.y / Config::GRID_SIZE) * Config::GRID_SIZE;

    float majorGridSize = Config::GRID_SIZE * 4.0f;
    int majorStartX = floor(screenTopLeft.x / majorGridSize) * majorGridSize;
    int majorStartY = floor(screenTopLeft.y / majorGridSize) * majorGridSize;
    int majorEndX = ceil(screenBottomRight.x / majorGridSize) * majorGridSize;
    int majorEndY = ceil(screenBottomRight.y / majorGridSize) * majorGridSize;

    // Zoom-adaptive visibility and thickness
    float z = camera_.zoom;
    float vis = std::max(0.2f, std::min(1.0f, (z - 0.5f) / (2.5f - 0.5f)));
    Color majorColor = Fade(Config::Colors::GRID_LINE, vis);
    float majorThickness = std::max(1.0f, Config::GRID_LINE_THICKNESS * (0.75f + 0.5f * z));

    for (float x = majorStartX; x <= majorEndX; x += majorGridSize) {
        DrawLineEx({x, screenTopLeft.y}, {x, screenBottomRight.y}, majorThickness, majorColor);
    }
    for (float y = majorStartY; y <= majorEndY; y += majorGridSize) {
        DrawLineEx({screenTopLeft.x, y}, {screenBottomRight.x, y}, majorThickness, majorColor);
    }

    float dotSize = 1.0f / camera_.zoom;
    dotSize = std::max(0.5f, std::min(1.2f, dotSize));

    // Reduce dot density when zoomed out
    float step = Config::GRID_SIZE * (z < 0.6f ? 3.0f : (z < 0.85f ? 2.0f : 1.0f));
    for (float x = startX; x <= endX; x += step) {
        for (float y = startY; y <= endY; y += step) {
            bool isMajorIntersection = (fmod(x, majorGridSize) == 0.0f && fmod(y, majorGridSize) == 0.0f);
            Color baseColor = isMajorIntersection ? Config::Colors::GRID_LINE : Config::Colors::GRID_DOT;
            Color dotColor = Fade(baseColor, vis);
            float currentDotSize = isMajorIntersection ? dotSize * 1.5f : dotSize;

            DrawCircleV({x, y}, currentDotSize, dotColor);
        }
    }
}

void UIManager::startPanning(Vector2 mousePos) {
    if (!isPanning_) {
        isPanning_ = true;
        panStartPosition_ = mousePos;
        lastMousePosition_ = mousePos;
        panVelocity_ = {0, 0};
    }
}

void UIManager::updatePanning(Vector2 mousePos) {
    if (isPanning_) {
        Vector2 delta = {
            mousePos.x - lastMousePosition_.x,
            mousePos.y - lastMousePosition_.y
        };

        camera_.target.x -= delta.x / camera_.zoom;
        camera_.target.y -= delta.y / camera_.zoom;

        panVelocity_.x = delta.x / camera_.zoom;
        panVelocity_.y = delta.y / camera_.zoom;

        lastMousePosition_ = mousePos;
    }
}

void UIManager::stopPanning() {
    isPanning_ = false;
}

void UIManager::updateCamera() {
    if (!isPanning_ && (panVelocity_.x != 0 || panVelocity_.y != 0)) {
        camera_.target.x -= panVelocity_.x;
        camera_.target.y -= panVelocity_.y;

        panVelocity_.x *= panInertia_;
        panVelocity_.y *= panInertia_;

        if (fabs(panVelocity_.x) < 0.01f) panVelocity_.x = 0;
        if (fabs(panVelocity_.y) < 0.01f) panVelocity_.y = 0;
    }
}

bool UIManager::isPanningActive() const {
    return isPanning_;
}

void UIManager::updateWirePathsForComponent(LogicGate* component) {
    if (!component) {
        return;
    }

    const std::vector<Wire*>& associatedWires = component->getAssociatedWires();

    for (Wire* wire : associatedWires) {
        if (wire) {
            wire->recalculatePath();
        }
    }
}


void UIManager::handleWindowResize(int newWidth, int newHeight) {
    camera_.offset = { (float)newWidth / 2, (float)newHeight / 2 };

    canvasBounds_ = {
        Config::PALETTE_WIDTH,
        0,
        (float)newWidth - Config::PALETTE_WIDTH,
        (float)newHeight
    };

    paletteManager_->handleWindowResize();
}

Vector2 UIManager::checkWireAlignmentSnapping(LogicGate* gate, Vector2 position,
                                              bool enableGridSnap,
                                              bool enableAlignmentSnap) {
    if (!gate) {
        return position;
    }

    const float SNAP_THRESHOLD = 15.0f;
    const std::vector<Wire*>& associatedWires = gate->getAssociatedWires();

    // Optional grid snap to keep movement tidy
    Vector2 snapped = position;
    if (gridSnapEnabled_ && enableGridSnap) {
        float gs = Config::GRID_SIZE;
        snapped.x = roundf(snapped.x / gs) * gs;
        snapped.y = roundf(snapped.y / gs) * gs;
    }

    if (!enableAlignmentSnap || associatedWires.empty()) {
        return snapped;
    }

    Vector2 adjustedPosition = snapped;
    bool hasSnapped = false;

    for (Wire* wire : associatedWires) {
        if (!wire || hasSnapped) continue;

        GatePin* sourcePin = wire->getSourcePin();
        GatePin* destPin = wire->getDestPin();

        if (!sourcePin || !destPin) continue;

        LogicGate* sourceGate = sourcePin->getParentGate();
        LogicGate* destGate = destPin->getParentGate();

        if (!sourceGate || !destGate) continue;

        GatePin* thisGatePin;
        GatePin* otherGatePin;

        if (gate == sourceGate) {
            thisGatePin = sourcePin;
            otherGatePin = destPin;
        } else {
            thisGatePin = destPin;
            otherGatePin = sourcePin;
        }

        Vector2 currentPinOffset = Vector2Subtract(thisGatePin->getAbsolutePosition(), gate->getPosition());
        Vector2 projectedPinPos = Vector2Add(snapped, currentPinOffset);
        Vector2 otherPinPos = otherGatePin->getAbsolutePosition();

        float yDiff = fabs(projectedPinPos.y - otherPinPos.y);
        if (yDiff < SNAP_THRESHOLD) {
            float yAdjustment = otherPinPos.y - projectedPinPos.y;
            adjustedPosition.y = snapped.y + yAdjustment;
            hasSnapped = true;
        }

        if (!hasSnapped) {
            float xDiff = fabs(projectedPinPos.x - otherPinPos.x);
            if (xDiff < SNAP_THRESHOLD) {
                float xAdjustment = otherPinPos.x - projectedPinPos.x;
                adjustedPosition.x = snapped.x + xAdjustment;
                hasSnapped = true;
            }
        }
    }

    return adjustedPosition;
}
