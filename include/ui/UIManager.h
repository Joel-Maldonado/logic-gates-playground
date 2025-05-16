#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "raylib.h"
#include "simulation/CircuitSimulator.h"
#include "ui/PaletteManager.h"
#include "ui/GateRenderer.h"
#include "ui/WireRenderer.h"
#include "core/InputSource.h"
#include <memory>

class UIManager {
private:
    Camera2D camera;
    Rectangle canvasBounds;
    Vector2 wirePreviewEndPos;
    bool isDrawingWire;
    GatePin* wireStartPin;

    bool isPanning;
    Vector2 panStartPosition;
    Vector2 lastMousePosition;
    Vector2 panVelocity;
    float panInertia;

    std::shared_ptr<CircuitSimulator> simulator;
    std::unique_ptr<PaletteManager> paletteManager;
    std::unique_ptr<GateRenderer> gateRenderer;
    std::unique_ptr<WireRenderer> wireRenderer;

    LogicGate* selectedComponent;
    Wire* selectedWire;
    bool isDraggingComponent;
    bool isDraggingWirePoint;
    Vector2 dragStartOffset;
    InputSource* clickedInputSource;
    Vector2 dragStartPosition;

public:
    UIManager(std::shared_ptr<CircuitSimulator> sim);
    void initialize();
    void render();
    void processInput();

    Camera2D& getCamera();
    Rectangle getCanvasBounds() const;

    void selectComponent(LogicGate* component);
    void selectWire(Wire* wire);
    void deselectAll();

    void startDrawingWire(GatePin* pin);
    void updateWirePreview(Vector2 mousePos);
    bool completeWireDrawing(GatePin* pin);
    void cancelWireDrawing();

    void startDraggingComponent(LogicGate* component, Vector2 mousePos);
    void updateDragging(Vector2 mousePos);
    void stopDragging();

    bool startDraggingWirePoint(Vector2 mousePos);
    void updateWirePointDragging(Vector2 mousePos);
    void stopWirePointDragging();
    bool isDraggingWirePointActive() const;

    void setClickedInputSource(InputSource* inputSource);
    bool wasDragged(Vector2 currentMousePos) const;
    void deleteSelected();

    LogicGate* getSelectedComponent() const;
    Wire* getSelectedWire() const;
    bool isDrawingWireActive() const;
    PaletteManager& getPaletteManager();

    void renderGrid();
    void startPanning(Vector2 mousePos);
    void updatePanning(Vector2 mousePos);
    void stopPanning();
    void updateCamera();
    bool isPanningActive() const;

    void updateWirePathsForComponent(LogicGate* component);
    Vector2 snapToGrid(Vector2 position);
    Vector2 checkWireAlignmentSnapping(LogicGate* gate, Vector2 position);
    void handleWindowResize(int newWidth, int newHeight);
};

#endif // UI_MANAGER_H
