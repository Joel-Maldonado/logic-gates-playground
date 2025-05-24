#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <raylib.h>
#include "simulation/CircuitSimulator.h"
#include "ui/PaletteManager.h"
#include "ui/GateRenderer.h"
#include "ui/WireRenderer.h"
#include "core/InputSource.h"
#include <memory>

/**
 * Main UI management class that coordinates all user interface elements
 * including camera control, component selection, wire drawing, and input handling.
 */
class UIManager {
private:
    // Camera and viewport
    Camera2D camera_;
    Rectangle canvasBounds_;

    // Wire drawing state
    Vector2 wirePreviewEndPos_;
    bool isDrawingWire_;
    GatePin* wireStartPin_;

    // Panning state
    bool isPanning_;
    Vector2 panStartPosition_;
    Vector2 lastMousePosition_;
    Vector2 panVelocity_;
    float panInertia_;

    // Core components
    std::shared_ptr<CircuitSimulator> simulator_;
    std::unique_ptr<PaletteManager> paletteManager_;
    std::unique_ptr<GateRenderer> gateRenderer_;
    std::unique_ptr<WireRenderer> wireRenderer_;

    // Selection and interaction state
    LogicGate* selectedComponent_;
    Wire* selectedWire_;
    bool isDraggingComponent_;
    bool isDraggingWirePoint_;
    Vector2 dragStartOffset_;
    InputSource* clickedInputSource_;
    Vector2 dragStartPosition_;

public:
    UIManager(std::shared_ptr<CircuitSimulator> sim);

    /** Initializes UI components and sets up initial state */
    void initialize();

    /** Renders all UI elements */
    void render();

    /** Processes user input events */
    void processInput();

    // Camera and viewport
    Camera2D& getCamera();
    Rectangle getCanvasBounds() const;

    // Selection management
    void selectComponent(LogicGate* component);
    void selectWire(Wire* wire);
    void deselectAll();

    // Wire drawing operations
    void startDrawingWire(GatePin* pin);
    void updateWirePreview(Vector2 mousePos);
    bool completeWireDrawing(GatePin* pin);
    void cancelWireDrawing();

    // Component dragging operations
    void startDraggingComponent(LogicGate* component, Vector2 mousePos);
    void updateDragging(Vector2 mousePos);
    void stopDragging();

    // Wire point dragging operations
    bool startDraggingWirePoint(Vector2 mousePos);
    void updateWirePointDragging(Vector2 mousePos);
    void stopWirePointDragging();
    bool isDraggingWirePointActive() const;

    // Input source interaction
    void setClickedInputSource(InputSource* inputSource);
    bool wasDragged(Vector2 currentMousePos) const;
    void deleteSelected();

    // State queries
    LogicGate* getSelectedComponent() const;
    Wire* getSelectedWire() const;
    bool isDrawingWireActive() const;
    PaletteManager& getPaletteManager();

    // Camera and rendering utilities
    void renderGrid();
    void startPanning(Vector2 mousePos);
    void updatePanning(Vector2 mousePos);
    void stopPanning();
    void updateCamera();
    bool isPanningActive() const;

    // Wire and component utilities
    void updateWirePathsForComponent(LogicGate* component);
    Vector2 checkWireAlignmentSnapping(LogicGate* gate, Vector2 position);
    void handleWindowResize(int newWidth, int newHeight);
};

#endif // UI_MANAGER_H
