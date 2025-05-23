#include "ui/UIManager.h"
#include "app/Config.h"
#include <raymath.h>
#include <algorithm>
#include <cmath>

UIManager::UIManager(std::shared_ptr<CircuitSimulator> sim)
    : isDrawingWire(false),
      wireStartPin(nullptr),
      isPanning(false),
      panStartPosition({0, 0}),
      lastMousePosition({0, 0}),
      panVelocity({0, 0}),
      panInertia(0.9f),
      simulator(sim),
      selectedComponent(nullptr),
      selectedWire(nullptr),
      isDraggingComponent(false),
      isDraggingWirePoint(false),
      dragStartOffset({0, 0}),
      clickedInputSource(nullptr),
      dragStartPosition({0, 0}) {

    camera.target = { (float)Config::SCREEN_WIDTH / 2, (float)Config::SCREEN_HEIGHT / 2 };
    camera.offset = { (float)Config::SCREEN_WIDTH / 2, (float)Config::SCREEN_HEIGHT / 2 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    canvasBounds = {
        Config::PALETTE_WIDTH,
        0,
        (float)Config::SCREEN_WIDTH - Config::PALETTE_WIDTH,
        (float)Config::SCREEN_HEIGHT
    };

    paletteManager = std::make_unique<PaletteManager>(simulator);
    gateRenderer = std::make_unique<GateRenderer>();
    wireRenderer = std::make_unique<WireRenderer>();
}

void UIManager::initialize() {
    paletteManager->initialize();
}

void UIManager::render() {
    BeginDrawing();
    ClearBackground(Config::Colors::BACKGROUND);

    updateCamera();

    BeginMode2D(camera);

    if (Config::GRID_ENABLED) {
        renderGrid();
    }

    wireRenderer->renderWires(simulator->getWires());

    if (isDrawingWire && wireStartPin) {
        Vector2 worldMousePos = GetScreenToWorld2D(GetMousePosition(), camera);

        bool isOverInputPin = false;
        for (const auto& gate : simulator->getGates()) {
            for (size_t i = 0; i < gate->getInputPinCount(); i++) {
                GatePin* pin = gate->getInputPin(i);
                if (pin && pin->isMouseOverPin(worldMousePos) && !pin->isConnectedInput()) {
                    isOverInputPin = true;
                    break;
                }
            }
            if (isOverInputPin) break;
        }

        wireRenderer->renderWirePreview(
            wireStartPin->getAbsolutePosition(),
            wirePreviewEndPos,
            isOverInputPin,
            Config::Colors::WIRE_PREVIEW,
            Config::WIRE_THICKNESS_PREVIEW
        );

        gateRenderer->renderWirePreview(
            wireStartPin,
            simulator->getGates(),
            worldMousePos
        );
    }

    gateRenderer->renderGates(simulator->getGates(), camera);

    EndMode2D();

    paletteManager->render(camera);

    DrawFPS(GetScreenWidth() - 90, 10);

    EndDrawing();
}

void UIManager::processInput() {
}

Camera2D& UIManager::getCamera() {
    return camera;
}

Rectangle UIManager::getCanvasBounds() const {
    return canvasBounds;
}

void UIManager::selectComponent(LogicGate* component) {
    deselectAll();

    if (component) {
        selectedComponent = component;
        selectedComponent->setSelected(true);
    }
}

void UIManager::selectWire(Wire* wire) {
    deselectAll();

    if (wire) {
        selectedWire = wire;
        selectedWire->setSelected(true);
    }
}

void UIManager::deselectAll() {
    if (selectedComponent) {
        selectedComponent->setSelected(false);
        selectedComponent = nullptr;
    }

    if (selectedWire) {
        selectedWire->setSelected(false);
        selectedWire = nullptr;
    }
}

void UIManager::startDrawingWire(GatePin* pin) {
    if (!pin || pin->getType() != PinType::OUTPUT_PIN) {
        return;
    }

    isDrawingWire = true;
    wireStartPin = pin;
    wirePreviewEndPos = pin->getAbsolutePosition();
    isDraggingComponent = false;
}

void UIManager::updateWirePreview(Vector2 mousePos) {
    if (!isDrawingWire || !wireStartPin) {
        return;
    }

    wirePreviewEndPos = mousePos;
}

bool UIManager::completeWireDrawing(GatePin* pin) {
    if (!isDrawingWire || !wireStartPin || !pin) {
        return false;
    }

    if (pin == wireStartPin || pin->getType() != PinType::INPUT_PIN || pin->isConnectedInput()) {
        return false;
    }

    Wire* wire = simulator->createWire(wireStartPin, pin);

    isDrawingWire = false;
    wireStartPin = nullptr;

    return (wire != nullptr);
}

void UIManager::cancelWireDrawing() {
    isDrawingWire = false;
    wireStartPin = nullptr;
}

void UIManager::startDraggingComponent(LogicGate* component, Vector2 mousePos) {
    if (!component) {
        return;
    }

    selectComponent(component);
    isDraggingComponent = true;
    dragStartOffset = Vector2Subtract(mousePos, component->getPosition());
    dragStartPosition = mousePos;
}

void UIManager::updateDragging(Vector2 mousePos) {
    if (!isDraggingComponent || !selectedComponent) {
        return;
    }

    Vector2 position = Vector2Subtract(mousePos, dragStartOffset);
    Vector2 alignedPosition = checkWireAlignmentSnapping(selectedComponent, position);
    selectedComponent->setPosition(alignedPosition);
    updateWirePathsForComponent(selectedComponent);
}

void UIManager::stopDragging() {
    if (isDraggingComponent && selectedComponent) {
        Vector2 currentPos = selectedComponent->getPosition();
        Vector2 alignedPos = checkWireAlignmentSnapping(selectedComponent, currentPos);
        selectedComponent->setPosition(alignedPos);
        updateWirePathsForComponent(selectedComponent);
    }

    isDraggingComponent = false;
}

bool UIManager::startDraggingWirePoint(Vector2 mousePos) {
    if (!selectedWire) {
        return false;
    }

    if (selectedWire->startDraggingPoint(mousePos)) {
        isDraggingWirePoint = true;
        return true;
    }

    return false;
}

void UIManager::updateWirePointDragging(Vector2 mousePos) {
    if (!isDraggingWirePoint || !selectedWire) {
        return;
    }

    selectedWire->updateDraggedPoint(mousePos);
}

void UIManager::stopWirePointDragging() {
    if (selectedWire) {
        selectedWire->stopDraggingPoint();
    }
    isDraggingWirePoint = false;
}

bool UIManager::isDraggingWirePointActive() const {
    return isDraggingWirePoint;
}

void UIManager::deleteSelected() {
    if (selectedComponent) {
        simulator->removeGate(selectedComponent);
        selectedComponent = nullptr;
    } else if (selectedWire) {
        simulator->removeWire(selectedWire);
        selectedWire = nullptr;
    }
}

LogicGate* UIManager::getSelectedComponent() const {
    return selectedComponent;
}

Wire* UIManager::getSelectedWire() const {
    return selectedWire;
}

bool UIManager::isDrawingWireActive() const {
    return isDrawingWire;
}

PaletteManager& UIManager::getPaletteManager() {
    return *paletteManager;
}

void UIManager::setClickedInputSource(InputSource* inputSource) {
    clickedInputSource = inputSource;
}

bool UIManager::wasDragged(Vector2 currentMousePos) const {
    float dx = currentMousePos.x - dragStartPosition.x;
    float dy = currentMousePos.y - dragStartPosition.y;
    float distance = sqrt(dx*dx + dy*dy);
    return distance > Config::DRAG_THRESHOLD;
}

void UIManager::renderGrid() {
    Vector2 screenTopLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 screenBottomRight = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    int startX = floor(screenTopLeft.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    int startY = floor(screenTopLeft.y / Config::GRID_SIZE) * Config::GRID_SIZE;
    int endX = ceil(screenBottomRight.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    int endY = ceil(screenBottomRight.y / Config::GRID_SIZE) * Config::GRID_SIZE;

    float majorGridSize = Config::GRID_SIZE * 4.0f;
    int majorStartX = floor(screenTopLeft.x / majorGridSize) * majorGridSize;
    int majorStartY = floor(screenTopLeft.y / majorGridSize) * majorGridSize;
    int majorEndX = ceil(screenBottomRight.x / majorGridSize) * majorGridSize;
    int majorEndY = ceil(screenBottomRight.y / majorGridSize) * majorGridSize;

    for (float x = majorStartX; x <= majorEndX; x += majorGridSize) {
        DrawLineV({x, screenTopLeft.y}, {x, screenBottomRight.y}, Config::Colors::GRID_LINE);
    }
    for (float y = majorStartY; y <= majorEndY; y += majorGridSize) {
        DrawLineV({screenTopLeft.x, y}, {screenBottomRight.x, y}, Config::Colors::GRID_LINE);
    }

    float dotSize = 1.5f / camera.zoom;
    dotSize = std::max(0.5f, std::min(2.0f, dotSize));

    for (float x = startX; x <= endX; x += Config::GRID_SIZE) {
        for (float y = startY; y <= endY; y += Config::GRID_SIZE) {
            bool isMajorIntersection = (fmod(x, majorGridSize) == 0.0f && fmod(y, majorGridSize) == 0.0f);
            Color dotColor = isMajorIntersection ? Config::Colors::GRID_LINE : Config::Colors::GRID_DOT;
            float currentDotSize = isMajorIntersection ? dotSize * 1.5f : dotSize;

            DrawCircleV({x, y}, currentDotSize, dotColor);
        }
    }
}

void UIManager::startPanning(Vector2 mousePos) {
    if (!isPanning) {
        isPanning = true;
        panStartPosition = mousePos;
        lastMousePosition = mousePos;
        panVelocity = {0, 0};
    }
}

void UIManager::updatePanning(Vector2 mousePos) {
    if (isPanning) {
        Vector2 delta = {
            mousePos.x - lastMousePosition.x,
            mousePos.y - lastMousePosition.y
        };

        camera.target.x -= delta.x / camera.zoom;
        camera.target.y -= delta.y / camera.zoom;

        panVelocity.x = delta.x / camera.zoom;
        panVelocity.y = delta.y / camera.zoom;

        lastMousePosition = mousePos;
    }
}

void UIManager::stopPanning() {
    isPanning = false;
}

void UIManager::updateCamera() {
    if (!isPanning && (panVelocity.x != 0 || panVelocity.y != 0)) {
        camera.target.x -= panVelocity.x;
        camera.target.y -= panVelocity.y;

        panVelocity.x *= panInertia;
        panVelocity.y *= panInertia;

        if (fabs(panVelocity.x) < 0.01f) panVelocity.x = 0;
        if (fabs(panVelocity.y) < 0.01f) panVelocity.y = 0;
    }
}

bool UIManager::isPanningActive() const {
    return isPanning;
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
    camera.offset = { (float)newWidth / 2, (float)newHeight / 2 };

    canvasBounds = {
        Config::PALETTE_WIDTH,
        0,
        (float)newWidth - Config::PALETTE_WIDTH,
        (float)newHeight
    };

    paletteManager->handleWindowResize();
}

Vector2 UIManager::checkWireAlignmentSnapping(LogicGate* gate, Vector2 position) {
    if (!gate) {
        return position;
    }

    const float SNAP_THRESHOLD = 15.0f;
    const std::vector<Wire*>& associatedWires = gate->getAssociatedWires();

    if (associatedWires.empty()) {
        return position;
    }

    Vector2 adjustedPosition = position;
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
        Vector2 projectedPinPos = Vector2Add(position, currentPinOffset);
        Vector2 otherPinPos = otherGatePin->getAbsolutePosition();

        float yDiff = fabs(projectedPinPos.y - otherPinPos.y);
        if (yDiff < SNAP_THRESHOLD) {
            float yAdjustment = otherPinPos.y - projectedPinPos.y;
            adjustedPosition.y = position.y + yAdjustment;
            hasSnapped = true;
        }

        if (!hasSnapped) {
            float xDiff = fabs(projectedPinPos.x - otherPinPos.x);
            if (xDiff < SNAP_THRESHOLD) {
                float xAdjustment = otherPinPos.x - projectedPinPos.x;
                adjustedPosition.x = position.x + xAdjustment;
                hasSnapped = true;
            }
        }
    }

    return adjustedPosition;
}
