#include "core/OutputSink.h"
#include "app/Config.h"
#include <raylib.h>
#include "ui/VisualEffects.h"
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

    // Determine colors with higher contrast against dark theme
    Color fillOff = VisualEffects::lerpColor(Config::Colors::GATE_FILL, WHITE, 0.15f);
    Color outlineOff = VisualEffects::lerpColor(Config::Colors::GATE_OUTLINE, WHITE, 0.45f);
    Color fillOn = Config::Colors::OUTPUT_ON;
    Color outlineOn = VisualEffects::lerpColor(Config::Colors::GATE_OUTLINE, WHITE, 0.25f);

    Color fill = active_ ? fillOn : fillOff;
    Color outline = active_ ? outlineOn : outlineOff;

    // Draw main body
    DrawCircleV(center, radius_, fill);

    // Draw a ring outline for stronger separation
    float ringInner = radius_ - 2.0f;
    float ringOuter = radius_ + 0.0f;
    if (getIsSelected()) {
        DrawRing(center, ringInner - 1.0f, ringOuter + 1.0f, 0.0f, 360.0f, 36, SELECTION_COLOR);
        DrawRing(center, ringInner - 3.0f, ringInner - 1.0f, 0.0f, 360.0f, 36, Fade(SELECTION_COLOR, 0.6f));
    } else {
        DrawRing(center, ringInner, ringOuter, 0.0f, 360.0f, 36, outline);
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
            Color pinFill = active_ ? fillOn : fillOff;
            Color pinOutline = active_ ? outlineOn : outlineOff;
            DrawCircleV(pinPos, Config::CONNECTOR_PIN_RADIUS - 1.0f, pinFill);
            DrawRing(pinPos, Config::CONNECTOR_PIN_RADIUS - 1.5f, Config::CONNECTOR_PIN_RADIUS, 0.0f, 360.0f, 24, pinOutline);
        }
    }
}

bool OutputSink::isActive() const {
    return active_;
}
