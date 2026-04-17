#ifndef UI_GATE_GEOMETRY_H
#define UI_GATE_GEOMETRY_H

#include "core/LogicGate.h"

#include <raylib.h>
#include <vector>

struct GateShapeData {
    std::vector<Vector2> fillPath;
    std::vector<Vector2> strokePath;
    std::vector<std::vector<Vector2>> accentStrokes;
    bool circular = false;
    Vector2 circleCenter{0.0f, 0.0f};
    float circleRadius = 0.0f;
    bool hasBubble = false;
    Vector2 bubbleCenter{0.0f, 0.0f};
    float bubbleRadius = 0.0f;
};

class GateGeometry {
public:
    static GateShapeData buildShape(GateKind kind, Rectangle bounds);
    static std::vector<Vector2> pinAnchors(GateKind kind, Rectangle bounds);
    static bool hitTestBody(GateKind kind, Rectangle bounds, Vector2 point);
};

#endif // UI_GATE_GEOMETRY_H
