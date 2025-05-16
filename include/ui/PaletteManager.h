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

    Color getGateColor(GateType type) const;
    void drawGateIcon(GateType type, Rectangle bounds, Color color) const;

public:
    PaletteManager(std::shared_ptr<CircuitSimulator> sim);
    void initialize();
    void render(const Camera2D& camera);
    bool handleClick(Vector2 mousePos);
    std::unique_ptr<LogicGate> createSelectedGateInstance(Vector2 position);
    GateType getSelectedGateType() const;
    void setSelectedGateType(GateType type);
    static std::string getGateTypeName(GateType type);
    Rectangle getPaletteBounds() const;
    bool startDraggingGate(Vector2 mousePos);
    void updateDragPosition(Vector2 mousePos);
    LogicGate* endDraggingGate(Vector2 worldPos);
    void cancelDraggingGate();
    bool isDraggingGateActive() const;
    GateType getDraggedGateType() const;
    Vector2 getCurrentDragPosition() const;
    std::unique_ptr<LogicGate> createGateInstance(GateType type, Vector2 position);
    void handleWindowResize(int newHeight);
};

#endif // PALETTE_MANAGER_H
