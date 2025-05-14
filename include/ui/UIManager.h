#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "raylib.h"
#include "simulation/CircuitSimulator.h"
#include "ui/PaletteManager.h"
#include "ui/GateRenderer.h"
#include "ui/WireRenderer.h"
#include "core/InputSource.h"
#include <memory>

/**
 * Manages the user interface for the application
 * Coordinates rendering and input handling
 */
class UIManager {
private:
    Camera2D camera;
    Rectangle canvasBounds;
    Vector2 wirePreviewEndPos;
    bool isDrawingWire;
    GatePin* wireStartPin;

    // Panning state
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
    /**
     * Constructs a new UIManager
     *
     * @param sim Shared pointer to the circuit simulator
     */
    UIManager(std::shared_ptr<CircuitSimulator> sim);

    /**
     * Initializes the UI
     */
    void initialize();

    /**
     * Renders the UI
     */
    void render();

    /**
     * Processes user input
     */
    void processInput();

    /**
     * Gets the camera
     *
     * @return Reference to the camera
     */
    Camera2D& getCamera();

    /**
     * Gets the canvas bounds
     *
     * @return Canvas bounds rectangle
     */
    Rectangle getCanvasBounds() const;

    /**
     * Selects a component
     *
     * @param component Pointer to the component to select
     */
    void selectComponent(LogicGate* component);

    /**
     * Selects a wire
     *
     * @param wire Pointer to the wire to select
     */
    void selectWire(Wire* wire);

    /**
     * Deselects all components and wires
     */
    void deselectAll();

    /**
     * Starts drawing a wire from a pin
     *
     * @param pin Pointer to the pin to start from
     */
    void startDrawingWire(GatePin* pin);

    /**
     * Updates the wire preview end position
     *
     * @param mousePos Current mouse position in world coordinates
     */
    void updateWirePreview(Vector2 mousePos);

    /**
     * Completes drawing a wire to a pin
     *
     * @param pin Pointer to the pin to connect to
     * @return True if the wire was created successfully
     */
    bool completeWireDrawing(GatePin* pin);

    /**
     * Cancels drawing a wire
     */
    void cancelWireDrawing();

    /**
     * Starts dragging a component
     *
     * @param component Pointer to the component to drag
     * @param mousePos Current mouse position
     */
    void startDraggingComponent(LogicGate* component, Vector2 mousePos);

    /**
     * Updates component dragging
     *
     * @param mousePos Current mouse position
     */
    void updateDragging(Vector2 mousePos);

    /**
     * Stops dragging a component
     */
    void stopDragging();

    /**
     * Starts dragging a wire control point
     *
     * @param mousePos Current mouse position
     * @return True if a control point was found to drag
     */
    bool startDraggingWirePoint(Vector2 mousePos);

    /**
     * Updates wire point dragging
     *
     * @param mousePos Current mouse position
     */
    void updateWirePointDragging(Vector2 mousePos);

    /**
     * Stops dragging a wire control point
     */
    void stopWirePointDragging();

    /**
     * Checks if a wire control point is being dragged
     *
     * @return True if a wire control point is being dragged
     */
    bool isDraggingWirePointActive() const;

    /**
     * Sets the clicked input source for potential toggle
     *
     * @param inputSource Pointer to the input source that was clicked
     */
    void setClickedInputSource(InputSource* inputSource);

    /**
     * Checks if the component was dragged or just clicked
     *
     * @param currentMousePos Current mouse position
     * @return True if the component was dragged, false if it was just clicked
     */
    bool wasDragged(Vector2 currentMousePos) const;

    /**
     * Deletes the selected component or wire
     */
    void deleteSelected();

    /**
     * Gets the selected component
     *
     * @return Pointer to the selected component, or nullptr if none is selected
     */
    LogicGate* getSelectedComponent() const;

    /**
     * Gets the selected wire
     *
     * @return Pointer to the selected wire, or nullptr if none is selected
     */
    Wire* getSelectedWire() const;

    /**
     * Checks if a wire is being drawn
     *
     * @return True if a wire is being drawn
     */
    bool isDrawingWireActive() const;

    /**
     * Gets the palette manager
     *
     * @return Reference to the palette manager
     */
    PaletteManager& getPaletteManager();

    /**
     * Renders the grid background
     */
    void renderGrid();

    /**
     * Starts panning the view
     *
     * @param mousePos Current mouse position in screen coordinates
     */
    void startPanning(Vector2 mousePos);

    /**
     * Updates panning based on mouse movement
     *
     * @param mousePos Current mouse position in screen coordinates
     */
    void updatePanning(Vector2 mousePos);

    /**
     * Stops panning the view
     */
    void stopPanning();

    /**
     * Updates the camera position with inertia
     */
    void updateCamera();

    /**
     * Checks if panning is active
     *
     * @return True if panning is active
     */
    bool isPanningActive() const;

    /**
     * Updates all wire paths connected to a component
     *
     * @param component Pointer to the component whose wires need updating
     */
    void updateWirePathsForComponent(LogicGate* component);

    /**
     * Snaps a position to the nearest grid point
     *
     * @param position Position to snap
     * @return Snapped position
     */
    Vector2 snapToGrid(Vector2 position);

    /**
     * Checks if a gate being dragged should snap to align with connected gates
     *
     * @param gate Pointer to the gate being dragged
     * @param position Current position of the gate
     * @return Position adjusted for wire alignment snapping
     */
    Vector2 checkWireAlignmentSnapping(LogicGate* gate, Vector2 position);

    /**
     * Handles window resize events
     *
     * @param newWidth New window width
     * @param newHeight New window height
     */
    void handleWindowResize(int newWidth, int newHeight);
};

#endif // UI_MANAGER_H
