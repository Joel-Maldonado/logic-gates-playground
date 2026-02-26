#ifndef PALETTE_MANAGER_H
#define PALETTE_MANAGER_H

#include "simulation/CircuitSimulator.h"
#include "ui/DesignTokens.h"

#include <raylib.h>
#include <memory>
#include <string>
#include <vector>

enum class GateType {
    NONE,
    INPUT_SOURCE,
    OUTPUT_SINK,
    AND,
    OR,
    XOR,
    NOT
};

struct PaletteItem {
    Rectangle bounds;
    GateType type;
    std::string label;
};

class PaletteManager {
public:
    explicit PaletteManager(std::shared_ptr<CircuitSimulator> simulator);

    void initialize();
    void setBounds(Rectangle bounds);
    Rectangle getPaletteBounds() const;

    void render(const DesignTokens& tokens, Rectangle canvasBounds) const;

    bool handleClick(Vector2 mousePos);
    bool startDraggingGate(Vector2 mousePos);
    void updateDragPosition(Vector2 mousePos);
    LogicGate* endDraggingGate(Vector2 worldPos);
    void cancelDraggingGate();

    bool isDraggingGateActive() const;
    GateType getDraggedGateType() const;
    Vector2 getCurrentDragPosition() const;

    std::unique_ptr<LogicGate> createSelectedGateInstance(Vector2 position);
    std::unique_ptr<LogicGate> createGateInstance(GateType type, Vector2 position);

    GateType getSelectedGateType() const;
    void setSelectedGateType(GateType type);

    static std::string getGateTypeName(GateType type);
    static GateKind toGateKind(GateType type);

    void handleWindowResize();

private:
    std::vector<PaletteItem> gatePalette_;
    std::shared_ptr<CircuitSimulator> simulator_;

    Rectangle bounds_;
    GateType selectedGateType_;

    bool isDraggingGate_;
    GateType draggedGateType_;
    Vector2 currentDragPos_;

    void drawGateIcon(GateType type, Rectangle iconBounds, const DesignTokens& tokens) const;
};

#endif // PALETTE_MANAGER_H
