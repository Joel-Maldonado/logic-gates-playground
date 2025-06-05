#ifndef PALETTE_MANAGER_H
#define PALETTE_MANAGER_H

#include <raylib.h>
#include <raymath.h>
#include "simulation/CircuitSimulator.h"
#include <vector>
#include <string>
#include <memory>

// Forward declaration
class CustomGateRegistry;

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
    CREATE_CUSTOM_GATE,
    CUSTOM_GATE_INSTANCE // For instantiating saved custom gates
};

/**
 * Structure for an item in the palette
 */
struct PaletteItem {
    Rectangle bounds;
    GateType type;
    std::string label;
    std::string customGateName; // Used if type is CUSTOM_GATE_INSTANCE

    PaletteItem() : type(GateType::NONE), customGateName("") {} // Default constructor
};

/**
 * Manages the component palette
 * Handles rendering and interaction with the palette
 */
class PaletteManager {
private:
    std::vector<PaletteItem> gatePalette_;
    GateType selectedGateType_;
    std::shared_ptr<CircuitSimulator> simulator_;
    CustomGateRegistry* customGateRegistry_; // Pointer to the registry
    std::string draggedCustomGateName_;    // Name of the custom gate being dragged

    // Drag and drop state
    bool isDraggingGate_;
    GateType draggedGateType_;
    Vector2 dragStartPos_;
    Vector2 currentDragPos_;
    bool createCustomGateActionTriggered_;

    Color getGateColor(GateType type) const;
    void drawGateIcon(GateType type, Rectangle bounds, Color color) const;

public:
    PaletteManager(std::shared_ptr<CircuitSimulator> sim, CustomGateRegistry* registry); // Updated constructor
    void initialize();
    void refreshPaletteItems(); // To reload all items, including custom ones
    void render(const Camera2D& camera);
    bool handleClick(Vector2 mousePos);
    std::unique_ptr<LogicGate> createSelectedGateInstance(Vector2 position);
    GateType getSelectedGateType() const;
    void setSelectedGateType(GateType type);

    // Method to check if the "Create Custom Gate" action was triggered
    bool wasCreateCustomGateActionTriggered() {
        bool val = createCustomGateActionTriggered_;
        createCustomGateActionTriggered_ = false; // Reset after check
        return val;
    }

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
    void handleWindowResize();
};

#endif // PALETTE_MANAGER_H
