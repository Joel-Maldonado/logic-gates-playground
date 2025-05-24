#include "core/OutputSink.h"
#include "app/Config.h"
#include <raylib.h>
#include <utility>

namespace {
    constexpr Color SELECTION_COLOR = YELLOW;
}

OutputSink::OutputSink(std::string id, Vector2 pos, float visualRadius, std::string visualLabel)
    : LogicGate(std::move(id), pos, visualRadius * 2, visualRadius * 2),
      radius_(visualRadius),
      label_(std::move(visualLabel)),
      active_(false),
      style_() {
    initializeInputPin(0, {0, visualRadius});
    markDirty();
}

OutputSink::~OutputSink() = default;

void OutputSink::evaluate() {
    if (getInputPinCount() > 0) {
        bool newState = getInputPin(0)->getState();
        if (active_ != newState) {
            active_ = newState;
        }
    } else if (active_) {
        active_ = false;
    }
}

void OutputSink::draw() {
    Rectangle bounds = getBounds();
    Vector2 center = { bounds.x + radius_, bounds.y + radius_ };

    // Draw main body
    DrawCircleV(center, radius_, active_ ? style_.fillOn : style_.fillOff);

    // Draw outline (thicker if selected)
    if (getIsSelected()) {
        DrawCircleLines(center.x, center.y, radius_, SELECTION_COLOR);
        DrawCircleLines(center.x, center.y, radius_ + Config::GATE_OUTLINE_THICKNESS, SELECTION_COLOR);
    } else {
        DrawCircleLines(center.x, center.y, radius_, style_.outlineColor);
    }

    // Draw label if present
    if (!label_.empty()) {
        const float labelFontSize = Config::LABEL_FONT_SIZE;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), label_.c_str(), labelFontSize, 1.0f);
        Vector2 textPos = {
            center.x - textSize.x / 2.0f,
            bounds.y - textSize.y - Config::LABEL_SPACING
        };
        DrawTextEx(GetFontDefault(), label_.c_str(), textPos, labelFontSize, 1.0f, style_.textColor);
    }

    // Draw input pin
    if (getInputPinCount() > 0) {
        const GatePin* pin = getInputPin(0);
        if (pin) {
            Vector2 pinPos = pin->getAbsolutePosition();
            DrawCircleV(pinPos, Config::CONNECTOR_PIN_RADIUS, active_ ? style_.fillOn : style_.fillOff);
            DrawCircleLines(pinPos.x, pinPos.y, Config::CONNECTOR_PIN_RADIUS, style_.outlineColor);
        }
    }
}

bool OutputSink::isActive() const {
    return active_;
}
