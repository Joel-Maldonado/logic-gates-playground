#ifndef WIRE_H
#define WIRE_H

#include "core/LogicGate.h"
#include "raylib.h"
#include <vector>

class Wire {
public:
    bool isSelected;

private:
    bool state_;
    GatePin* sourcePin_;
    GatePin* destPin_;
    std::vector<Vector2> controlPoints_;
    bool isDraggingPoint_;
    int draggedPointIndex_;

public:
    struct Style {
        static Color off;
        static Color on;
        static Color selected;
    };

    Wire(GatePin* srcPin, GatePin* dstPin);
    void update();
    void draw() const;

    void setControlPoints(const std::vector<Vector2>& points);
    const std::vector<Vector2>& getControlPoints() const;
    void recalculatePath();

    bool startDraggingPoint(Vector2 mousePos, float tolerance = 8.0f);
    void updateDraggedPoint(Vector2 mousePos);
    void stopDraggingPoint();
    bool isDraggingPoint() const;

    void setSelected(bool selected);
    bool getIsSelected() const;
    bool isMouseOver(Vector2 mousePos, float tolerance = 2.0f) const;

    GatePin* getSourcePin() const;
    GatePin* getDestPin() const;
    bool getState() const;

private:
    void drawWirePath(const std::vector<Vector2>& points, Color color, float thickness) const;

public:
    void onGateDeleted(LogicGate* deletedGate);
};

#endif // WIRE_H