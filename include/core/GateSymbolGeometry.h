#ifndef CORE_GATE_SYMBOL_GEOMETRY_H
#define CORE_GATE_SYMBOL_GEOMETRY_H

#include "core/LogicGate.h"

#include <raylib.h>
#include <vector>

namespace GateSymbolGeometry {

struct BodyProfile {
    float leftX;
    float rightX;
    float centerY;
    float actualWidth;
    float curveDepth;
};

BodyProfile andProfile(Rectangle bounds);
BodyProfile orXorProfile(Rectangle bounds);
BodyProfile notProfile(Rectangle bounds);

float curvedBackX(const BodyProfile& profile, float t);
float xorRearCurveX(const BodyProfile& profile, float t);

Vector2 notBubbleCenter(const BodyProfile& profile);
float notBubbleRadius(float height);

std::vector<Vector2> pinOffsets(GateKind kind, Vector2 size);
std::vector<Vector2> pinAnchors(GateKind kind, Rectangle bounds);

} // namespace GateSymbolGeometry

#endif // CORE_GATE_SYMBOL_GEOMETRY_H
