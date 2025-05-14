#ifndef PALETTE_MANAGER_H
#define PALETTE_MANAGER_H

#include "raylib.h"
#include "raymath.h"
#include "simulation/CircuitSimulator.h"
#include <vector>
#include <string>
#include <memory>

/**
 * Enum for gate types available in the palette
 */
enum class GateType {
    NONE,
    INPUT_SOURCE,
    OUTPUT_SINK,
    AND,
    OR,
    XOR,
    NOT,
    NAND,
    NOR,
    XNOR
};

/**
 * Structure for an item in the palette
 */
struct PaletteItem {
    Rectangle bounds;
    GateType type;
    std::string label;
};

/**
 * Manages the component palette
 * Handles rendering and interaction with the palette
 */
class PaletteManager {
private:
    std::vector<PaletteItem> gatePalette;
    GateType selectedGateType;
    std::shared_ptr<CircuitSimulator> simulator;

    // Drag and drop state
    bool isDraggingGate;
    GateType draggedGateType;
    Vector2 dragStartPos;
    Vector2 currentDragPos;
    Vector2 snappedWorldPos; // Stores the snapped world position during dragging

    /**
     * Gets the color for a gate type
     *
     * @param type Gate type
     * @return Color for the gate
     */
    Color getGateColor(GateType type) const;

    /**
     * Draws an icon for a gate type
     *
     * @param type Gate type
     * @param bounds Bounds for the icon
     * @param color Color for the icon
     */
    void drawGateIcon(GateType type, Rectangle bounds, Color color) const;

public:
    /**
     * Constructs a new PaletteManager
     *
     * @param sim Shared pointer to the circuit simulator
     */
    PaletteManager(std::shared_ptr<CircuitSimulator> sim);

    /**
     * Initializes the palette
     */
    void initialize();

    /**
     * Renders the palette
     *
     * @param camera Camera used for world-to-screen conversions
     */
    void render(const Camera2D& camera);

    /**
     * Processes a click on the palette
     *
     * @param mousePos Mouse position
     * @return True if a palette item was clicked
     */
    bool handleClick(Vector2 mousePos);

    /**
     * Creates a gate instance of the selected type
     *
     * @param position Position for the new gate
     * @return Unique pointer to the created gate
     */
    std::unique_ptr<LogicGate> createSelectedGateInstance(Vector2 position);

    /**
     * Gets the selected gate type
     *
     * @return Selected gate type
     */
    GateType getSelectedGateType() const;

    /**
     * Sets the selected gate type
     *
     * @param type Gate type to select
     */
    void setSelectedGateType(GateType type);

    /**
     * Gets the name of a gate type
     *
     * @param type Gate type
     * @return Name of the gate type
     */
    static std::string getGateTypeName(GateType type);

    /**
     * Gets the palette bounds
     *
     * @return Rectangle representing the palette bounds
     */
    Rectangle getPaletteBounds() const;

    /**
     * Starts dragging a gate from the palette
     *
     * @param mousePos Mouse position
     * @return True if a gate was started to be dragged
     */
    bool startDraggingGate(Vector2 mousePos);

    /**
     * Updates the position of the gate being dragged
     *
     * @param mousePos Current mouse position
     */
    void updateDragPosition(Vector2 mousePos);

    /**
     * Ends dragging a gate and places it at the specified position
     *
     * @param worldPos World position to place the gate
     * @return Pointer to the created gate, or nullptr if no gate was created
     */
    LogicGate* endDraggingGate(Vector2 worldPos);

    /**
     * Cancels dragging a gate
     */
    void cancelDraggingGate();

    /**
     * Checks if a gate is being dragged
     *
     * @return True if a gate is being dragged
     */
    bool isDraggingGateActive() const;

    /**
     * Gets the type of gate being dragged
     *
     * @return Type of gate being dragged
     */
    GateType getDraggedGateType() const;

    /**
     * Gets the current position of the gate being dragged
     *
     * @return Current position of the gate being dragged
     */
    Vector2 getCurrentDragPosition() const;

    /**
     * Gets the current position of the gate being dragged, snapped to the grid
     *
     * @param worldPos World position to snap
     * @return Snapped position for the gate preview
     */
    Vector2 getSnappedDragPosition(Vector2 worldPos) const;

    /**
     * Gets the stored snapped world position for the current drag operation
     *
     * @return Snapped world position
     */
    Vector2 getStoredSnappedPosition() const;

    /**
     * Creates a gate instance of a specific type
     *
     * @param type Type of gate to create
     * @param position Position for the new gate
     * @return Unique pointer to the created gate
     */
    std::unique_ptr<LogicGate> createGateInstance(GateType type, Vector2 position);

    /**
     * Handles window resize events
     *
     * @param newHeight New window height
     */
    void handleWindowResize(int newHeight);
};

#endif // PALETTE_MANAGER_H
