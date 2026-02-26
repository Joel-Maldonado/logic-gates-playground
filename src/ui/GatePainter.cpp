#include "ui/GatePainter.h"

#include "app/Config.h"
#include "core/InputSource.h"
#include "core/OutputSink.h"
#include "ui/GateGeometry.h"

#include <raylib.h>
#include <raymath.h>
#include <algorithm>

namespace {

const char* gateLabel(GateKind kind) {
    switch (kind) {
        case GateKind::INPUT_SOURCE: return "IN";
        case GateKind::OUTPUT_SINK: return "OUT";
        case GateKind::AND_GATE: return "AND";
        case GateKind::OR_GATE: return "OR";
        case GateKind::XOR_GATE: return "XOR";
        case GateKind::NOT_GATE: return "NOT";
    }

    return "";
}

Color mix(Color a, Color b, float t) {
    const float clamped = std::clamp(t, 0.0f, 1.0f);
    return {
        static_cast<unsigned char>(a.r + (b.r - a.r) * clamped),
        static_cast<unsigned char>(a.g + (b.g - a.g) * clamped),
        static_cast<unsigned char>(a.b + (b.b - a.b) * clamped),
        static_cast<unsigned char>(a.a + (b.a - a.a) * clamped)
    };
}

Color gateAccent(GateKind kind, const DesignTokens& tokens) {
    switch (kind) {
        case GateKind::AND_GATE: return tokens.colors.gateAccentAnd;
        case GateKind::OR_GATE: return tokens.colors.gateAccentOr;
        case GateKind::XOR_GATE: return tokens.colors.gateAccentXor;
        case GateKind::NOT_GATE: return tokens.colors.gateAccentNot;
        case GateKind::INPUT_SOURCE:
        case GateKind::OUTPUT_SINK:
            return tokens.colors.accentPrimary;
    }

    return tokens.colors.accentPrimary;
}

void drawPathLines(const std::vector<Vector2>& points, float thickness, Color stroke, bool closed) {
    if (points.size() < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < points.size(); ++i) {
        DrawLineEx(points[i], points[i + 1], thickness, stroke);
    }

    if (closed && points.size() > 2) {
        DrawLineEx(points.back(), points.front(), thickness, stroke);
    }
}

void drawPathFill(const std::vector<Vector2>& points, Color fill) {
    if (points.size() < 3) {
        return;
    }

    Vector2 center{0.0f, 0.0f};
    for (const Vector2& p : points) {
        center.x += p.x;
        center.y += p.y;
    }
    center.x /= static_cast<float>(points.size());
    center.y /= static_cast<float>(points.size());

    for (size_t i = 0; i < points.size(); ++i) {
        const Vector2 a = points[i];
        const Vector2 b = points[(i + 1) % points.size()];
        DrawTriangle(center, a, b, fill);
    }
}

void drawPin(Vector2 pos, bool active, bool emphasize, const DesignTokens& tokens) {
    const float outer = tokens.metrics.pinRadius + 1.7f;
    Color halo = tokens.colors.canvasBackground;
    if (emphasize) {
        halo = mix(tokens.colors.canvasBackground, tokens.colors.accentSelection, 0.35f);
    }
    DrawCircleV(pos, outer, halo);
    DrawCircleV(pos, tokens.metrics.pinRadius, active ? tokens.colors.pinOn : tokens.colors.pinOff);
}

Vector2 gateSizeForKind(GateKind kind) {
    switch (kind) {
        case GateKind::INPUT_SOURCE:
        case GateKind::OUTPUT_SINK:
            return {Config::INPUT_OUTPUT_SIZE, Config::INPUT_OUTPUT_SIZE};
        case GateKind::AND_GATE:
        case GateKind::OR_GATE:
        case GateKind::XOR_GATE:
        case GateKind::NOT_GATE:
            return {Config::DEFAULT_GATE_WIDTH, Config::DEFAULT_GATE_HEIGHT};
    }

    return {Config::DEFAULT_GATE_WIDTH, Config::DEFAULT_GATE_HEIGHT};
}

} // namespace

void GatePainter::renderGate(const LogicGate* gate, bool selected, bool hovered, const DesignTokens& tokens) const {
    if (!gate) {
        return;
    }

    const Rectangle bounds = gate->getBounds();
    const GateShapeData shape = GateGeometry::buildShape(gate->getKind(), bounds);

    const Color accent = gateAccent(gate->getKind(), tokens);
    Color fill = mix(tokens.colors.gateFill, accent, 0.15f);
    if (gate->getKind() == GateKind::INPUT_SOURCE) {
        fill = static_cast<const InputSource*>(gate)->getCurrentState()
            ? mix(tokens.colors.panelElevated, tokens.colors.accentPrimary, 0.8f)
            : tokens.colors.panelElevated;
    } else if (gate->getKind() == GateKind::OUTPUT_SINK) {
        fill = static_cast<const OutputSink*>(gate)->isActive()
            ? mix(tokens.colors.panelElevated, tokens.colors.accentPrimary, 0.85f)
            : tokens.colors.panelElevated;
    } else if (hovered) {
        fill = mix(fill, accent, 0.08f);
    }

    Color stroke = tokens.colors.gateStroke;
    if (selected) {
        stroke = tokens.colors.accentSelection;
    } else if (hovered) {
        stroke = mix(tokens.colors.gateStroke, accent, 0.5f);
    }

    if (shape.circular) {
        DrawCircleV(shape.circleCenter, shape.circleRadius, fill);
        DrawCircleLines(static_cast<int>(shape.circleCenter.x),
                        static_cast<int>(shape.circleCenter.y),
                        shape.circleRadius,
                        stroke);
    } else {
        drawPathFill(shape.fillPath, fill);
        drawPathLines(shape.strokePath, tokens.metrics.strokeWidth, stroke, true);
    }

    for (const auto& accentStroke : shape.accentStrokes) {
        drawPathLines(accentStroke, std::max(1.4f, tokens.metrics.strokeWidth * 0.9f), mix(stroke, accent, 0.55f), false);
    }

    if (shape.hasBubble) {
        DrawCircleV(shape.bubbleCenter, shape.bubbleRadius, fill);
        DrawCircleLines(static_cast<int>(shape.bubbleCenter.x),
                        static_cast<int>(shape.bubbleCenter.y),
                        shape.bubbleRadius,
                        stroke);
    }

    const char* label = gateLabel(gate->getKind());
    const float labelSize = tokens.typography.smallSize;
    const Vector2 labelMeasure = MeasureTextEx(tokens.typography.ui, label, labelSize, 1.0f);
    const Vector2 labelPos = {
        bounds.x + (bounds.width - labelMeasure.x) * 0.5f,
        bounds.y - labelMeasure.y - 3.0f
    };
    DrawTextEx(tokens.typography.ui, label, labelPos, labelSize, 1.0f, tokens.colors.textMuted);

    if (gate->getKind() == GateKind::INPUT_SOURCE || gate->getKind() == GateKind::OUTPUT_SINK) {
        const char* state = "0";
        if (gate->getKind() == GateKind::INPUT_SOURCE) {
            state = static_cast<const InputSource*>(gate)->getCurrentState() ? "1" : "0";
        } else {
            state = static_cast<const OutputSink*>(gate)->isActive() ? "1" : "0";
        }

        const float stateSize = tokens.typography.bodySize + 4.0f;
        const Vector2 stateMeasure = MeasureTextEx(tokens.typography.mono, state, stateSize, 1.0f);
        const Vector2 statePos = {
            bounds.x + (bounds.width - stateMeasure.x) * 0.5f,
            bounds.y + (bounds.height - stateMeasure.y) * 0.5f
        };
        DrawTextEx(tokens.typography.mono, state, statePos, stateSize, 1.0f, tokens.colors.textPrimary);
    }

    for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
        const GatePin* pin = gate->getInputPin(i);
        if (!pin) {
            continue;
        }

        drawPin(pin->getAbsolutePosition(), pin->getState(), selected, tokens);
    }

    for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
        const GatePin* pin = gate->getOutputPin(i);
        if (!pin) {
            continue;
        }

        drawPin(pin->getAbsolutePosition(), pin->getState(), selected, tokens);
    }
}

void GatePainter::renderGhostGate(GateKind kind, Vector2 worldPos, bool snapped, const DesignTokens& tokens) const {
    const Vector2 size = gateSizeForKind(kind);
    const Rectangle bounds = {worldPos.x, worldPos.y, size.x, size.y};
    const GateShapeData shape = GateGeometry::buildShape(kind, bounds);
    const Color accent = gateAccent(kind, tokens);
    const Color fill = mix(tokens.colors.ghostFill, accent, 0.12f);
    const Color stroke = tokens.colors.ghostStroke;

    if (shape.circular) {
        DrawCircleV(shape.circleCenter, shape.circleRadius, fill);
        DrawCircleLines(static_cast<int>(shape.circleCenter.x),
                        static_cast<int>(shape.circleCenter.y),
                        shape.circleRadius,
                        stroke);
    } else {
        drawPathFill(shape.fillPath, fill);
        drawPathLines(shape.strokePath, tokens.metrics.strokeWidth, stroke, true);
    }

    for (const auto& accentStroke : shape.accentStrokes) {
        drawPathLines(accentStroke, std::max(1.2f, tokens.metrics.strokeWidth * 0.85f), mix(stroke, accent, 0.65f), false);
    }

    if (shape.hasBubble) {
        DrawCircleV(shape.bubbleCenter, shape.bubbleRadius, fill);
        DrawCircleLines(static_cast<int>(shape.bubbleCenter.x),
                        static_cast<int>(shape.bubbleCenter.y),
                        shape.bubbleRadius,
                        stroke);
    }

    const std::vector<Vector2> anchors = GateGeometry::pinAnchors(kind, bounds);
    for (size_t i = 0; i < anchors.size(); ++i) {
        const bool isOutput = (i + 1 == anchors.size());
        DrawCircleV(anchors[i], tokens.metrics.pinRadius + 1.2f, tokens.colors.canvasBackground);
        DrawCircleV(anchors[i], tokens.metrics.pinRadius, isOutput ? tokens.colors.pinOn : tokens.colors.pinOff);
    }

    const char* label = gateLabel(kind);
    const Vector2 labelSize = MeasureTextEx(tokens.typography.ui, label, tokens.typography.smallSize, 1.0f);
    DrawTextEx(tokens.typography.ui,
               label,
               {bounds.x + (bounds.width - labelSize.x) * 0.5f, bounds.y - labelSize.y - 3.0f},
               tokens.typography.smallSize,
               1.0f,
               mix(tokens.colors.textMuted, stroke, 0.35f));

    if (snapped) {
        DrawRectangleLinesEx(bounds, 1.0f, Fade(tokens.colors.accentSelection, 0.45f));
    }
}
