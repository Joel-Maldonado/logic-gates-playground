#include "core/OutputSink.h"
#include <raylib.h>
#include <utility>

namespace {
    constexpr float CONNECTOR_RADIUS = 5.0f;
    constexpr float LABEL_FONT_SIZE = 14.0f;
    constexpr float LABEL_SPACING = 3.0f;
    constexpr Color SELECTION_COLOR = YELLOW;
}

OutputSink::OutputSink(std::string id, Vector2 pos, float visualRadius, std::string visualLabel)
    : LogicGate(std::move(id), pos, visualRadius * 2, visualRadius * 2),
      radius(visualRadius),
      label(std::move(visualLabel)),
      active(false),
      style() {
    initializeInputPin(0, {0, visualRadius});
    markDirty();
}

OutputSink::~OutputSink() = default;

void OutputSink::evaluate() {
    if (getInputPinCount() > 0) {
        bool newState = getInputPin(0)->getState();
        if (active != newState) {
            active = newState;
        }
    } else if (active) {
        active = false;
    }
}

void OutputSink::draw() {
    Rectangle bounds = getBounds();
    Vector2 center = { bounds.x + radius, bounds.y + radius };

    // Draw main body
    DrawCircleV(center, radius, active ? style.fillOn : style.fillOff);

    // Draw outline (thicker if selected)
    if (getIsSelected()) {
        DrawCircleLines(center.x, center.y, radius, SELECTION_COLOR);
        DrawCircleLines(center.x, center.y, radius + 2.0f, SELECTION_COLOR);
    } else {
        DrawCircleLines(center.x, center.y, radius, style.outlineColor);
    }

    // Draw label if present
    if (!label.empty()) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), label.c_str(), LABEL_FONT_SIZE, 1.0f);
        Vector2 textPos = {
            center.x - textSize.x / 2.0f,
            bounds.y - textSize.y - LABEL_SPACING
        };
        DrawTextEx(GetFontDefault(), label.c_str(), textPos, LABEL_FONT_SIZE, 1.0f, style.textColor);
    }

    // Draw input pin
    if (getInputPinCount() > 0) {
        const GatePin* pin = getInputPin(0);
        if (pin) {
            Vector2 pinPos = pin->getAbsolutePosition();
            DrawCircleV(pinPos, CONNECTOR_RADIUS, active ? style.fillOn : style.fillOff);
            DrawCircleLines(pinPos.x, pinPos.y, CONNECTOR_RADIUS, style.outlineColor);
        }
    }
}

bool OutputSink::isActive() const {
    return active;
}
