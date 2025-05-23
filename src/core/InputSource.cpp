#include "core/InputSource.h"
#include <raylib.h>
#include <utility>

namespace { // Anonymous namespace for helpers and constants
    const Color PIN_STATE_ON_COLOR = SKYBLUE;
    const Color PIN_STATE_OFF_COLOR = GRAY;
    const Color COMPONENT_LABEL_COLOR = LIGHTGRAY;
    const float CONNECTOR_PIN_RADIUS = 5.0f;

    void drawSourceBodyAndOutline(const Rectangle& bounds, bool isSelected, bool currentState,
                                  Color bodyOnColor, Color bodyOffColor, Color outlineColor) {
        DrawRectangleRec(bounds, currentState ? bodyOnColor : bodyOffColor);
        if (isSelected) {
            DrawRectangleLinesEx(bounds, 3.0f, YELLOW);
        } else {
            DrawRectangleLinesEx(bounds, 2.0f, outlineColor);
        }
    }

    void drawSourceStateIndicator(const Rectangle& bounds, bool currentState, Color textColor) {
        const char* stateText = currentState ? "ON" : "OFF";
        int tempFontSize = static_cast<int>(bounds.height / 2.0f);
        if (tempFontSize < 12) {
            tempFontSize = 12;
        }
        float fontSize = static_cast<float>(tempFontSize);

        Vector2 textSize = MeasureTextEx(GetFontDefault(), stateText, fontSize, 1.0f);
        Vector2 textPosition = {
            bounds.x + (bounds.width - textSize.x) / 2.0f,
            bounds.y + (bounds.height - textSize.y) / 2.0f
        };
        DrawTextEx(GetFontDefault(), stateText, textPosition, fontSize, 1.0f, textColor);
    }

    void drawSourceComponentLabel(const Rectangle& bounds, const std::string& label) {
        if (!label.empty()) {
            const float labelFontSize = 14.0f;
            Vector2 labelTextSize = MeasureTextEx(GetFontDefault(), label.c_str(), labelFontSize, 1.0f);
            Vector2 labelPosition = {
                bounds.x + (bounds.width - labelTextSize.x) / 2.0f,
                bounds.y - labelTextSize.y - 3.0f
            };
            DrawTextEx(GetFontDefault(), label.c_str(), labelPosition, labelFontSize, 1.0f, COMPONENT_LABEL_COLOR);
        }
    }

    void drawSourceOutputPin(const GatePin* pin, Color outlineColorToUse) {
        if (pin) {
            Vector2 pinPos = pin->getAbsolutePosition();
            DrawCircleV(pinPos, CONNECTOR_PIN_RADIUS, pin->getState() ? PIN_STATE_ON_COLOR : PIN_STATE_OFF_COLOR);
            DrawCircleLines(pinPos.x, pinPos.y, CONNECTOR_PIN_RADIUS, outlineColorToUse);
        }
    }
}

InputSource::InputSource(std::string id, Vector2 pos, Vector2 visualSize, std::string visualLabel)
    : LogicGate(std::move(id), pos, visualSize.x, visualSize.y),
      label(std::move(visualLabel)),
      internalState(false) {
    initializeOutputPin(0, {visualSize.x, visualSize.y / 2.0f});

    colors.bodyOff = DARKGRAY;
    colors.bodyOn = LIME;
    colors.textColor = WHITE;
    colors.outlineColor = BLACK;

    markDirty();
}

InputSource::~InputSource() {
}

void InputSource::evaluate() {
    if (getOutputPinCount() > 0) {
        getOutputPin(0)->setState(internalState);
    }
}

void InputSource::draw() {
    Rectangle currentBounds = getBounds();

    drawSourceBodyAndOutline(currentBounds, getIsSelected(), internalState, colors.bodyOn, colors.bodyOff, colors.outlineColor);
    drawSourceStateIndicator(currentBounds, internalState, colors.textColor);
    drawSourceComponentLabel(currentBounds, label);

    if (getOutputPinCount() > 0) {
        drawSourceOutputPin(getOutputPin(0), colors.outlineColor);
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
    internalState = !internalState;
    markDirty();
}

bool InputSource::getCurrentState() const {
    return internalState;
}

void InputSource::setState(bool newState) {
    if (internalState != newState) {
        internalState = newState;
        markDirty();
    }
}