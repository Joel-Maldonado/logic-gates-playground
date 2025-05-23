#include "ui/WireRenderer.h"
#include "ui/VisualEffects.h"
#include "app/Config.h"
#include "core/GatePin.h"
#include <raymath.h>

WireRenderer::WireRenderer() {
}

void WireRenderer::renderWires(const std::vector<std::unique_ptr<Wire>>& wires) {
    for (const auto& wire : wires) {
        renderWire(wire.get());
    }
}

void WireRenderer::renderWire(const Wire* wire) {
    if (!wire) {
        return;
    }

    // Wire handles its own drawing using its control points
    wire->draw();
}

void WireRenderer::renderWirePreview(Vector2 startPos, Vector2 endPos, bool isDestInput, Color color, float thickness) {
    std::vector<Vector2> path = calculatePreviewPath(startPos, endPos, isDestInput);
    renderWirePathPreview(path, color, thickness);
}

void WireRenderer::renderWirePathPreview(const std::vector<Vector2>& path, Color color, float thickness) {
    if (path.size() < 2) {
        return;
    }

    // Draw line segments without glow effect
    for (size_t i = 0; i < path.size() - 1; i++) {
        DrawLineEx(path[i], path[i + 1], thickness, color);
    }

    // Add animated direction arrow if the last segment is long enough
    if (path.size() >= 2) {
        Vector2 lastSegmentStart = path[path.size() - 2];
        Vector2 lastSegmentEnd = path[path.size() - 1];
        float distance = Vector2Distance(lastSegmentStart, lastSegmentEnd);

        if (distance > 30.0f) {
            // Animate arrow position along the segment
            float pulseValue = VisualEffects::getPulseValue(6.0f);
            float t = 0.7f + pulseValue * 0.2f; // Animate between 70% and 90% of segment

            Vector2 arrowPos = {
                lastSegmentStart.x + t * (lastSegmentEnd.x - lastSegmentStart.x),
                lastSegmentStart.y + t * (lastSegmentEnd.y - lastSegmentStart.y)
            };

            // Calculate direction vector and perpendicular for arrow shape
            Vector2 dir = Vector2Normalize(Vector2Subtract(lastSegmentEnd, lastSegmentStart));
            Vector2 perp = { -dir.y, dir.x };  // 90-degree rotation for perpendicular

            // Create arrow triangle points with slight size animation
            float arrowSize = 8.0f + pulseValue * 2.0f;
            Vector2 arrowTip = Vector2Add(arrowPos, Vector2Scale(dir, arrowSize));
            Vector2 arrowLeft = Vector2Subtract(arrowPos, Vector2Scale(perp, arrowSize * 0.5f));
            Vector2 arrowRight = Vector2Add(arrowPos, Vector2Scale(perp, arrowSize * 0.5f));

            // Draw arrow triangle
            DrawTriangle(arrowTip, arrowLeft, arrowRight, color);
        }
    }
}

std::vector<Vector2> WireRenderer::calculatePreviewPath(Vector2 startPos, Vector2 endPos, bool isDestInput) {
    return router_.calculatePreviewPath(startPos, endPos, isDestInput);
}

Color WireRenderer::getWireColor(const Wire* wire) const {
    if (!wire) {
        return Config::Colors::WIRE_OFF;
    }

    if (wire->getIsSelected()) {
        return Config::Colors::WIRE_SELECTED;
    }

    return wire->getState() ? Config::Colors::WIRE_ON : Config::Colors::WIRE_OFF;
}

float WireRenderer::getWireThickness(const Wire* wire) const {
    if (!wire) {
        return Config::WIRE_THICKNESS_NORMAL;
    }

    return wire->getIsSelected() ? Config::WIRE_THICKNESS_SELECTED : Config::WIRE_THICKNESS_NORMAL;
}
