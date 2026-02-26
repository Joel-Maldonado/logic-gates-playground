#ifndef WIRE_H
#define WIRE_H

#include "core/LogicGate.h"
#include <raylib.h>
#include <vector>

class Wire {
private:
    bool state_;
    GatePin* sourcePin_;
    GatePin* destPin_;
    std::vector<Vector2> controlPoints_;
    bool isDraggingPoint_;
    int draggedPointIndex_;
    bool draggedPrevSegmentHorizontal_;

public:
    Wire(GatePin* srcPin, GatePin* dstPin);

    bool update();

    // Path management
    void setControlPoints(const std::vector<Vector2>& points);
    const std::vector<Vector2>& getControlPoints() const;
    void recalculatePath();

    // Interactive dragging
    bool startDraggingPoint(Vector2 mousePos, float tolerance = 8.0f);
    void updateDraggedPoint(Vector2 mousePos);
    void stopDraggingPoint();
    bool isDraggingPoint() const;

    // Interaction helpers
    bool isMouseOver(Vector2 mousePos, float tolerance = 2.0f) const;

    // Property getters
    GatePin* getSourcePin() const;
    GatePin* getDestPin() const;
    bool getState() const;
};

#endif // WIRE_H
