#ifndef WIRE_H
#define WIRE_H

#include "core/LogicGate.h"
#include <raylib.h>
#include <vector>

/**
 * Represents a wire connection between two gate pins.
 * Handles signal propagation and visual rendering with routing.
 */
class Wire {
public:
    /** Wire color configuration */
    struct Style {
        static Color off;
        static Color on;
        static Color selected;
    };

    bool isSelected;

private:
    bool state_;
    GatePin* sourcePin_;
    GatePin* destPin_;
    std::vector<Vector2> controlPoints_;
    bool isDraggingPoint_;
    int draggedPointIndex_;

public:
    /**
     * Constructs a wire between two pins.
     * @param srcPin Source output pin
     * @param dstPin Destination input pin
     */
    Wire(GatePin* srcPin, GatePin* dstPin);

    /** Updates wire state based on source pin */
    void update();

    /** Renders the wire to screen */
    void draw() const;

    // Path management
    void setControlPoints(const std::vector<Vector2>& points);
    const std::vector<Vector2>& getControlPoints() const;
    void recalculatePath();

    // Interactive dragging
    bool startDraggingPoint(Vector2 mousePos, float tolerance = 8.0f);
    void updateDraggedPoint(Vector2 mousePos);
    void stopDraggingPoint();
    bool isDraggingPoint() const;

    // Selection and interaction
    void setSelected(bool selected);
    bool getIsSelected() const;
    bool isMouseOver(Vector2 mousePos, float tolerance = 2.0f) const;

    // Property getters
    GatePin* getSourcePin() const;
    GatePin* getDestPin() const;
    bool getState() const;

    /** Handles cleanup when a connected gate is deleted */
    void onGateDeleted(LogicGate* deletedGate);

private:
    void drawWirePath(const std::vector<Vector2>& points, Color color, float thickness) const;
    void drawEnhancedWirePath(const std::vector<Vector2>& points, Color color, float thickness) const;
};

#endif // WIRE_H