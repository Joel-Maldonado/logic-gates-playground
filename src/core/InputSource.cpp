#include "core/InputSource.h"
#include "app/Config.h"
#include <raylib.h>
#include <utility>

namespace {
    // Visual constants for input sources
    const Color PIN_STATE_ON_COLOR = SKYBLUE;
    const Color PIN_STATE_OFF_COLOR = GRAY;
    const Color COMPONENT_LABEL_COLOR = LIGHTGRAY;

    /** Draws the body and outline of an input source */
    void drawSourceBodyAndOutline(const Rectangle& bounds, bool isSelected, bool currentState,
                                  Color bodyOnColor, Color bodyOffColor, Color outlineColor) {
        DrawRectangleRec(bounds, currentState ? bodyOnColor : bodyOffColor);
        if (isSelected) {
            DrawRectangleLinesEx(bounds, Config::GATE_OUTLINE_THICKNESS_SELECTED, YELLOW);
        } else {
            DrawRectangleLinesEx(bounds, Config::GATE_OUTLINE_THICKNESS, outlineColor);
        }
    }

    /** Draws the state indicator text (ON/OFF) in the center of the component */
    void drawSourceStateIndicator(const Rectangle& bounds, bool currentState, Color textColor) {
        const char* stateText = currentState ? "ON" : "OFF";
        int tempFontSize = static_cast<int>(bounds.height / 2.0f);
        if (tempFontSize < 12) {
            tempFontSize = 12;
        }
        const float fontSize = static_cast<float>(tempFontSize);

        Vector2 textSize = MeasureTextEx(GetFontDefault(), stateText, fontSize, 1.0f);
        Vector2 textPosition = {
            bounds.x + (bounds.width - textSize.x) / 2.0f,
            bounds.y + (bounds.height - textSize.y) / 2.0f
        };
        DrawTextEx(GetFontDefault(), stateText, textPosition, fontSize, 1.0f, textColor);
    }

    /** Draws the component label above the component */
    void drawSourceComponentLabel(const Rectangle& bounds, const std::string& label) {
        if (!label.empty()) {
            const float labelFontSize = Config::LABEL_FONT_SIZE;
            Vector2 labelTextSize = MeasureTextEx(GetFontDefault(), label.c_str(), labelFontSize, 1.0f);
            Vector2 labelPosition = {
                bounds.x + (bounds.width - labelTextSize.x) / 2.0f,
                bounds.y - labelTextSize.y - Config::LABEL_SPACING
            };
            DrawTextEx(GetFontDefault(), label.c_str(), labelPosition, labelFontSize, 1.0f, COMPONENT_LABEL_COLOR);
        }
    }

    /** Draws the output pin of the input source */
    void drawSourceOutputPin(const GatePin* pin, Color outlineColorToUse) {
        if (pin) {
            Vector2 pinPos = pin->getAbsolutePosition();
            DrawCircleV(pinPos, Config::CONNECTOR_PIN_RADIUS, pin->getState() ? PIN_STATE_ON_COLOR : PIN_STATE_OFF_COLOR);
            DrawCircleLines(pinPos.x, pinPos.y, Config::CONNECTOR_PIN_RADIUS, outlineColorToUse);
        }
    }
}

InputSource::InputSource(std::string id, Vector2 pos, Vector2 visualSize, std::string visualLabel)
    : LogicGate(std::move(id), pos, visualSize.x, visualSize.y),
      label_(std::move(visualLabel)),
      internalState_(false) {
    initializeOutputPin(0, {visualSize.x, visualSize.y / 2.0f});

    colors_.bodyOff = DARKGRAY;
    colors_.bodyOn = LIME;
    colors_.textColor = WHITE;
    colors_.outlineColor = BLACK;

    markDirty();
}

InputSource::~InputSource() {
}

void InputSource::evaluate() {
    if (getOutputPinCount() > 0) {
        getOutputPin(0)->setState(internalState_);
    }
}

void InputSource::draw() {
    Rectangle currentBounds = getBounds();

    drawSourceBodyAndOutline(currentBounds, getIsSelected(), internalState_, colors_.bodyOn, colors_.bodyOff, colors_.outlineColor);
    drawSourceStateIndicator(currentBounds, internalState_, colors_.textColor);
    drawSourceComponentLabel(currentBounds, label_);

    if (getOutputPinCount() > 0) {
        drawSourceOutputPin(getOutputPin(0), colors_.outlineColor);
    }
}

void InputSource::handleInput(Vector2 mousePos) {
    if (isMouseOver(mousePos)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            toggleState();
        }
    }
}

void InputSource::toggleState() {
    internalState_ = !internalState_;
    markDirty();
}

bool InputSource::getCurrentState() const {
    return internalState_;
}

void InputSource::setState(bool newState) {
    if (internalState_ != newState) {
        internalState_ = newState;
        markDirty();
    }
}