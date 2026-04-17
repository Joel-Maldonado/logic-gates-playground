#include "ui/WirePainter.h"

#include <raylib.h>
#include <raymath.h>

#include <algorithm>

namespace {

Color mix(Color a, Color b, float t) {
    const float k = std::clamp(t, 0.0f, 1.0f);
    return {
        static_cast<unsigned char>(a.r + (b.r - a.r) * k),
        static_cast<unsigned char>(a.g + (b.g - a.g) * k),
        static_cast<unsigned char>(a.b + (b.b - a.b) * k),
        static_cast<unsigned char>(a.a + (b.a - a.a) * k)
    };
}

void drawOrthogonalPath(const std::vector<Vector2>& points, float thickness, Color color) {
    if (points.size() < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < points.size(); ++i) {
        DrawLineEx(points[i], points[i + 1], thickness, color);
    }

    if (points.size() > 2) {
        const float elbowRadius = std::max(1.0f, thickness * 0.65f);
        for (size_t i = 1; i + 1 < points.size(); ++i) {
            DrawCircleV(points[i], elbowRadius, color);
        }
    }
}

Vector2 animatedSignalPosition(const std::vector<Vector2>& points) {
    if (points.size() < 2) {
        return {0.0f, 0.0f};
    }

    std::vector<float> lengths;
    lengths.reserve(points.size() - 1);

    float totalLength = 0.0f;
    for (size_t i = 0; i + 1 < points.size(); ++i) {
        const float length = Vector2Distance(points[i], points[i + 1]);
        lengths.push_back(length);
        totalLength += length;
    }

    if (totalLength <= 0.001f) {
        return points.front();
    }

    const float progress = fmodf(static_cast<float>(GetTime()) * 0.65f, 1.0f);
    const float target = progress * totalLength;

    float walked = 0.0f;
    for (size_t i = 0; i < lengths.size(); ++i) {
        if (walked + lengths[i] >= target) {
            const float localT = (target - walked) / lengths[i];
            return Vector2Lerp(points[i], points[i + 1], localT);
        }
        walked += lengths[i];
    }

    return points.back();
}

} // namespace

void WirePainter::renderWires(const std::vector<std::unique_ptr<Wire>>& wires,
                              const EditorSelection& selection,
                              const Wire* hoveredWire,
                              const DesignTokens& tokens) const {
    for (const auto& wire : wires) {
        if (!wire) {
            continue;
        }

        Color color = wire->getState() ? tokens.colors.wireOn : tokens.colors.wireOff;
        float thickness = 2.0f;

        if (selection.containsWire(wire.get())) {
            color = mix(tokens.colors.wireSelection, tokens.colors.textPrimary, 0.06f);
            thickness = 3.4f;
        } else if (wire.get() == hoveredWire) {
            color = mix(tokens.colors.wireHover, tokens.colors.accentPrimary, 0.35f);
            thickness = 2.8f;
        }

        const std::vector<Vector2>& path = wire->getControlPoints();
        drawOrthogonalPath(path, thickness, color);

        if (wire->getState()) {
            const Vector2 signalPos = animatedSignalPosition(path);
            DrawCircleV(signalPos, 4.5f, Fade(tokens.colors.wireOn, 0.2f));
            DrawCircleV(signalPos, 3.0f, tokens.colors.wireOn);
        }

        if (selection.containsWire(wire.get())) {
            for (const Vector2& p : path) {
                DrawCircleV(p, 5.0f, tokens.colors.canvasBackground);
                DrawCircleV(p, 4.0f, tokens.colors.accentSelection);
            }
        }
    }
}

void WirePainter::renderWirePreview(const std::vector<Vector2>& previewPath,
                                    bool validTarget,
                                    const DesignTokens& tokens) const {
    if (previewPath.size() < 2) {
        return;
    }

    const Color color = validTarget ? tokens.colors.accentSelection : tokens.colors.accentWarning;
    drawOrthogonalPath(previewPath, 2.4f, color);
}
