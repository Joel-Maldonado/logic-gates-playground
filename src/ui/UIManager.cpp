#include "ui/UIManager.h"
#include "app/Config.h"
#include "raymath.h"

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

    // Initialize camera
    camera.target = { (float)Config::SCREEN_WIDTH / 2, (float)Config::SCREEN_HEIGHT / 2 };
    camera.offset = { (float)Config::SCREEN_WIDTH / 2, (float)Config::SCREEN_HEIGHT / 2 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Initialize canvas bounds
    canvasBounds = {
        Config::PALETTE_WIDTH,
        0,
        (float)Config::SCREEN_WIDTH - Config::PALETTE_WIDTH,
        (float)Config::SCREEN_HEIGHT
    };

    // Initialize managers and renderers
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

    // Update camera position with inertia if needed
    updateCamera();

    // Draw canvas content
    BeginMode2D(camera);

    // Draw grid background
    if (Config::GRID_ENABLED) {
        renderGrid();
    }

    // Draw wires
    wireRenderer->renderWires(simulator->getWires());

    // Draw wire preview if drawing a wire
    if (isDrawingWire && wireStartPin) {
        Vector2 worldMousePos = GetScreenToWorld2D(GetMousePosition(), camera);

        // Check if mouse is over an input pin to determine routing style
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

        // Render the wire preview with appropriate routing
        wireRenderer->renderWirePreview(
            wireStartPin->getAbsolutePosition(),
            wirePreviewEndPos,
            isOverInputPin,  // Pass whether we're over an input pin
            Config::Colors::WIRE_PREVIEW,
            Config::WIRE_THICKNESS_PREVIEW
        );

        // Highlight potential connection pins
        gateRenderer->renderWirePreview(
            wireStartPin,
            wirePreviewEndPos,
            simulator->getGates(),
            worldMousePos
        );
    }

    // Draw gates
    gateRenderer->renderGates(simulator->getGates(), camera);

    EndMode2D();

    // Draw palette
    paletteManager->render(camera);

    // Draw FPS
    DrawFPS(GetScreenWidth() - 90, 10);

    EndDrawing();
}

void UIManager::processInput() {
    // This is handled by the InputHandler class
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

    // Create the wire
    Wire* wire = simulator->createWire(wireStartPin, pin);

    if (wire) {
        // The wire will automatically calculate its path in the constructor
        // using the new routing algorithm that ensures horizontal approach to input pins
        // No need to manually set control points here
    }

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
    dragStartPosition = mousePos; // Store the starting position for drag detection
}

void UIManager::updateDragging(Vector2 mousePos) {
    if (!isDraggingComponent || !selectedComponent) {
        return;
    }

    // Calculate the raw position based on mouse position and drag offset
    Vector2 rawPosition = Vector2Subtract(mousePos, dragStartOffset);

    // Snap the position to the grid in real-time if grid snapping is enabled
    Vector2 snappedPosition = snapToGrid(rawPosition);

    // Check for wire alignment snapping
    Vector2 alignedPosition = checkWireAlignmentSnapping(selectedComponent, snappedPosition);

    // Update the component's position with the snapped and aligned position
    selectedComponent->setPosition(alignedPosition);

    // Update all wire paths connected to this component
    updateWirePathsForComponent(selectedComponent);
}

void UIManager::stopDragging() {
    if (isDraggingComponent && selectedComponent) {
        // Snap the component to the grid
        Vector2 currentPos = selectedComponent->getPosition();
        Vector2 snappedPos = snapToGrid(currentPos);

        // Check for wire alignment snapping
        Vector2 alignedPos = checkWireAlignmentSnapping(selectedComponent, snappedPos);

        // Set the snapped and aligned position
        selectedComponent->setPosition(alignedPos);

        // Update wire paths after snapping
        updateWirePathsForComponent(selectedComponent);
    }

    isDraggingComponent = false;
}

bool UIManager::startDraggingWirePoint(Vector2 mousePos) {
    if (!selectedWire) {
        return false;
    }

    // Try to start dragging a control point on the selected wire
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

    // Update the dragged control point position
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
    // Calculate the distance between the start position and current position
    float dx = currentMousePos.x - dragStartPosition.x;
    float dy = currentMousePos.y - dragStartPosition.y;
    float distance = sqrt(dx*dx + dy*dy);

    // If the distance is greater than a threshold, consider it a drag
    return distance > Config::DRAG_THRESHOLD;
}

void UIManager::renderGrid() {
    // Calculate the visible area in world coordinates
    Vector2 screenTopLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 screenBottomRight = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    // Calculate the starting and ending grid lines
    int startX = floor(screenTopLeft.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    int startY = floor(screenTopLeft.y / Config::GRID_SIZE) * Config::GRID_SIZE;
    int endX = ceil(screenBottomRight.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    int endY = ceil(screenBottomRight.y / Config::GRID_SIZE) * Config::GRID_SIZE;

    // Draw vertical grid lines
    for (float x = startX; x <= endX; x += Config::GRID_SIZE) {
        DrawLineV({x, screenTopLeft.y}, {x, screenBottomRight.y}, Config::Colors::GRID_LINE);
    }

    // Draw horizontal grid lines
    for (float y = startY; y <= endY; y += Config::GRID_SIZE) {
        DrawLineV({screenTopLeft.x, y}, {screenBottomRight.x, y}, Config::Colors::GRID_LINE);
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
        // Calculate the movement delta in screen space
        Vector2 delta = {
            mousePos.x - lastMousePosition.x,
            mousePos.y - lastMousePosition.y
        };

        // Update camera target (move in opposite direction of mouse movement)
        camera.target.x -= delta.x / camera.zoom;
        camera.target.y -= delta.y / camera.zoom;

        // Update velocity for inertia
        panVelocity.x = delta.x / camera.zoom;
        panVelocity.y = delta.y / camera.zoom;

        // Update last mouse position
        lastMousePosition = mousePos;
    }
}

void UIManager::stopPanning() {
    isPanning = false;
}

void UIManager::updateCamera() {
    if (!isPanning && (panVelocity.x != 0 || panVelocity.y != 0)) {
        // Apply inertia to slow down the panning
        camera.target.x -= panVelocity.x;
        camera.target.y -= panVelocity.y;

        // Reduce velocity
        panVelocity.x *= panInertia;
        panVelocity.y *= panInertia;

        // Stop when velocity is very small
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

    // Get all wires associated with this component
    const std::vector<Wire*>& associatedWires = component->getAssociatedWires();

    // Recalculate the path for each wire
    for (Wire* wire : associatedWires) {
        if (wire) {
            wire->recalculatePath();
        }
    }
}

Vector2 UIManager::snapToGrid(Vector2 position) {
    // Return original position if grid snapping is disabled or grid size is invalid
    if (!Config::GRID_SNAPPING_ENABLED || Config::GRID_SIZE <= 0) {
        return position;
    }

    // Round to nearest grid point
    Vector2 snappedPos;
    snappedPos.x = round(position.x / Config::GRID_SIZE) * Config::GRID_SIZE;
    snappedPos.y = round(position.y / Config::GRID_SIZE) * Config::GRID_SIZE;

    return snappedPos;
}

void UIManager::handleWindowResize(int newWidth, int newHeight) {
    // Update camera offset to match the new center of the screen
    camera.offset = { (float)newWidth / 2, (float)newHeight / 2 };

    // Update canvas bounds to match the new window size
    canvasBounds = {
        Config::PALETTE_WIDTH,
        0,
        (float)newWidth - Config::PALETTE_WIDTH,
        (float)newHeight
    };

    // Notify the palette manager of the window resize
    paletteManager->handleWindowResize(newHeight);
}

Vector2 UIManager::checkWireAlignmentSnapping(LogicGate* gate, Vector2 position) {
    if (!gate) {
        return position;
    }

    // Define the snapping threshold (how close pins need to be to snap)
    const float SNAP_THRESHOLD = 15.0f;

    // Get all wires associated with this gate
    const std::vector<Wire*>& associatedWires = gate->getAssociatedWires();

    // If no wires are connected, return the original position
    if (associatedWires.empty()) {
        return position;
    }

    // Create a copy of the position that we'll modify
    Vector2 adjustedPosition = position;
    bool hasSnapped = false;

    // For each wire, check if we should snap to align with the connected gate's pins
    for (Wire* wire : associatedWires) {
        if (!wire || hasSnapped) continue;

        // Get the pins at both ends of the wire
        GatePin* sourcePin = wire->getSourcePin();
        GatePin* destPin = wire->getDestPin();

        // Skip if either pin is null
        if (!sourcePin || !destPin) continue;

        // Get the parent gates of both pins
        LogicGate* sourceGate = sourcePin->getParentGate();
        LogicGate* destGate = destPin->getParentGate();

        // Skip if either gate is null
        if (!sourceGate || !destGate) continue;

        // Determine which pins belong to which gate
        GatePin* thisGatePin;
        GatePin* otherGatePin;

        if (gate == sourceGate) {
            thisGatePin = sourcePin;
            otherGatePin = destPin;
        } else {
            thisGatePin = destPin;
            otherGatePin = sourcePin;
        }

        // Calculate what the pin position would be at the current gate position
        // We need to account for the pin's relative offset from the gate's position
        Vector2 currentPinOffset = Vector2Subtract(thisGatePin->getAbsolutePosition(), gate->getPosition());
        Vector2 projectedPinPos = Vector2Add(position, currentPinOffset);

        // Get the absolute position of the other gate's pin
        Vector2 otherPinPos = otherGatePin->getAbsolutePosition();

        // Check for horizontal alignment (y-axis)
        float yDiff = fabs(projectedPinPos.y - otherPinPos.y);
        if (yDiff < SNAP_THRESHOLD) {
            // Calculate the adjustment needed to align the pins
            float yAdjustment = otherPinPos.y - projectedPinPos.y;
            adjustedPosition.y = position.y + yAdjustment;
            hasSnapped = true;
        }

        // Check for vertical alignment (x-axis)
        // Only check if we haven't already snapped horizontally
        if (!hasSnapped) {
            float xDiff = fabs(projectedPinPos.x - otherPinPos.x);
            if (xDiff < SNAP_THRESHOLD) {
                // Calculate the adjustment needed to align the pins
                float xAdjustment = otherPinPos.x - projectedPinPos.x;
                adjustedPosition.x = position.x + xAdjustment;
                hasSnapped = true;
            }
        }
    }

    return adjustedPosition;
}
