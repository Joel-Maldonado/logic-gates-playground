#include "ui/GateGeometry.h"
#include "core/GateSymbolGeometry.h"

#include <cmath>

namespace {

constexpr int CURVE_SEGMENTS = 24;

bool pointInPolygon(const std::vector<Vector2>& poly, Vector2 p) {
    if (poly.size() < 3) {
        return false;
    }

    bool inside = false;
    size_t j = poly.size() - 1;
    for (size_t i = 0; i < poly.size(); ++i) {
        const Vector2 pi = poly[i];
        const Vector2 pj = poly[j];

        const bool intersects = ((pi.y > p.y) != (pj.y > p.y)) &&
            (p.x < (pj.x - pi.x) * (p.y - pi.y) / ((pj.y - pi.y) + 0.00001f) + pi.x);
        if (intersects) {
            inside = !inside;
        }

        j = i;
    }

    return inside;
}

std::vector<Vector2> circlePoints(Vector2 center, float radius, int segments) {
    std::vector<Vector2> points;
    points.reserve(static_cast<size_t>(segments));

    for (int i = 0; i < segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float a = t * 2.0f * PI;
        points.push_back({center.x + cosf(a) * radius, center.y + sinf(a) * radius});
    }

    return points;
}

void appendSemiCircle(std::vector<Vector2>& points, Vector2 center, float radius, bool rightHalf) {
    const float start = rightHalf ? -PI / 2.0f : PI / 2.0f;
    const float end = rightHalf ? PI / 2.0f : 3.0f * PI / 2.0f;

    for (int i = 0; i <= CURVE_SEGMENTS; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(CURVE_SEGMENTS);
        const float a = start + t * (end - start);
        points.push_back({center.x + cosf(a) * radius, center.y + sinf(a) * radius});
    }
}

std::vector<Vector2> orBackCurve(const GateSymbolGeometry::BodyProfile& profile, Rectangle bounds) {
    std::vector<Vector2> curve;
    curve.reserve(CURVE_SEGMENTS + 1);

    for (int i = 0; i <= CURVE_SEGMENTS; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(CURVE_SEGMENTS);
        const float y = bounds.y + t * bounds.height;
        const float x = GateSymbolGeometry::curvedBackX(profile, t);
        curve.push_back({x, y});
    }

    return curve;
}

std::vector<Vector2> xorBackCurve(const GateSymbolGeometry::BodyProfile& profile, Rectangle bounds) {
    std::vector<Vector2> curve;
    curve.reserve(CURVE_SEGMENTS + 1);

    for (int i = 0; i <= CURVE_SEGMENTS; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(CURVE_SEGMENTS);
        const float y = bounds.y + t * bounds.height;
        const float x = GateSymbolGeometry::xorRearCurveX(profile, t);
        curve.push_back({x, y});
    }

    return curve;
}

} // namespace

GateShapeData GateGeometry::buildShape(GateKind kind, Rectangle bounds) {
    GateShapeData shape;

    switch (kind) {
        case GateKind::INPUT_SOURCE: {
            shape.fillPath = {
                {bounds.x, bounds.y},
                {bounds.x + bounds.width, bounds.y},
                {bounds.x + bounds.width, bounds.y + bounds.height},
                {bounds.x, bounds.y + bounds.height}
            };
            shape.strokePath = shape.fillPath;
            break;
        }
        case GateKind::OUTPUT_SINK: {
            const Vector2 center = {bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f};
            const float radius = bounds.width / 2.0f;
            shape.circular = true;
            shape.circleCenter = center;
            shape.circleRadius = radius;
            shape.fillPath = circlePoints(center, radius, CURVE_SEGMENTS);
            shape.strokePath = shape.fillPath;
            break;
        }
        case GateKind::AND_GATE: {
            const GateSymbolGeometry::BodyProfile profile = GateSymbolGeometry::andProfile(bounds);
            const float radius = bounds.height / 2.0f;
            const float flatWidth = profile.actualWidth - radius;
            const Vector2 center = {profile.leftX + flatWidth, profile.centerY};

            shape.fillPath.push_back({profile.leftX, bounds.y});
            shape.fillPath.push_back({profile.leftX + flatWidth, bounds.y});
            appendSemiCircle(shape.fillPath, center, radius, true);
            shape.fillPath.push_back({profile.leftX + flatWidth, bounds.y + bounds.height});
            shape.fillPath.push_back({profile.leftX, bounds.y + bounds.height});
            shape.strokePath = shape.fillPath;
            break;
        }
        case GateKind::OR_GATE: {
            const GateSymbolGeometry::BodyProfile profile = GateSymbolGeometry::orXorProfile(bounds);
            const Vector2 tip = {profile.rightX, profile.centerY};
            const std::vector<Vector2> curve = orBackCurve(profile, bounds);

            shape.fillPath.push_back(curve.front());
            shape.fillPath.push_back(tip);
            shape.fillPath.push_back(curve.back());
            for (int i = static_cast<int>(curve.size()) - 2; i > 0; --i) {
                shape.fillPath.push_back(curve[static_cast<size_t>(i)]);
            }
            shape.strokePath = shape.fillPath;
            break;
        }
        case GateKind::XOR_GATE: {
            const GateSymbolGeometry::BodyProfile profile = GateSymbolGeometry::orXorProfile(bounds);
            const Vector2 tip = {profile.rightX, profile.centerY};
            const std::vector<Vector2> curve = orBackCurve(profile, bounds);

            shape.fillPath.push_back(curve.front());
            shape.fillPath.push_back(tip);
            shape.fillPath.push_back(curve.back());
            for (int i = static_cast<int>(curve.size()) - 2; i > 0; --i) {
                shape.fillPath.push_back(curve[static_cast<size_t>(i)]);
            }
            shape.strokePath = shape.fillPath;
            shape.accentStrokes.push_back(xorBackCurve(profile, bounds));
            break;
        }
        case GateKind::NOT_GATE: {
            const GateSymbolGeometry::BodyProfile profile = GateSymbolGeometry::notProfile(bounds);
            const Vector2 topLeft = {profile.leftX, bounds.y};
            const Vector2 bottomLeft = {profile.leftX, bounds.y + bounds.height};
            const Vector2 tip = {profile.rightX, profile.centerY};
            shape.fillPath = {topLeft, bottomLeft, tip};
            shape.strokePath = shape.fillPath;
            shape.hasBubble = true;
            shape.bubbleCenter = GateSymbolGeometry::notBubbleCenter(profile);
            shape.bubbleRadius = GateSymbolGeometry::notBubbleRadius(bounds.height);
            break;
        }
    }

    return shape;
}

std::vector<Vector2> GateGeometry::pinAnchors(GateKind kind, Rectangle bounds) {
    return GateSymbolGeometry::pinAnchors(kind, bounds);
}

bool GateGeometry::hitTestBody(GateKind kind, Rectangle bounds, Vector2 point) {
    const GateShapeData shape = buildShape(kind, bounds);

    if (shape.circular) {
        return CheckCollisionPointCircle(point, shape.circleCenter, shape.circleRadius);
    }

    if (shape.hasBubble && CheckCollisionPointCircle(point, shape.bubbleCenter, shape.bubbleRadius)) {
        return true;
    }

    if (shape.fillPath.size() < 3) {
        return CheckCollisionPointRec(point, bounds);
    }

    return pointInPolygon(shape.fillPath, point);
}
