#include "core/DerivedGates.h"
#include "app/Config.h"
#include <raylib.h>

namespace {
    // Gate positioning constants
    constexpr float INPUT_PIN_TOP_RATIO = 1.0f / 3.0f;
    constexpr float INPUT_PIN_BOTTOM_RATIO = 2.0f / 3.0f;
    constexpr float OUTPUT_PIN_CENTER_RATIO = 0.5f;

    // Gate shape constants
    constexpr float OR_XOR_WIDTH_RATIO = 0.8f;
    constexpr float OR_XOR_CURVE_DEPTH_RATIO = 0.12f;
    constexpr float NOT_WIDTH_RATIO = 0.7f;
    constexpr float NOT_TRIANGLE_ASPECT_RATIO = 0.866f; // sqrt(3)/2 for equilateral triangle
    constexpr float INVERSION_BUBBLE_RADIUS = 5.0f;

    /* Draws a placeholder gate for testing purposes */
    void drawPlaceholderGate(LogicGate* gate, const char* label) {
        Rectangle bounds = gate->getBounds();
        DrawRectangleRec(bounds, LIGHTGRAY);
        DrawRectangleLinesEx(bounds, Config::GATE_OUTLINE_THICKNESS, DARKGRAY);
        DrawText(label,
                static_cast<int>(bounds.x + Config::GATE_TEXT_OFFSET),
                static_cast<int>(bounds.y + Config::GATE_TEXT_OFFSET),
                static_cast<int>(Config::GATE_TEXT_SIZE),
                BLACK);

        // Draw input pins
        for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
            const GatePin* pin = gate->getInputPin(i);
            if (pin) {
                Vector2 pinPos = pin->getAbsolutePosition();
                DrawCircleV(pinPos, Config::PIN_RADIUS, pin->getState() ? GREEN : RED);
                DrawCircleLines(pinPos.x, pinPos.y, Config::PIN_RADIUS, DARKGRAY);
            }
        }

        // Draw output pins
        for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
            const GatePin* pin = gate->getOutputPin(i);
            if (pin) {
                Vector2 pinPos = pin->getAbsolutePosition();
                DrawCircleV(pinPos, Config::PIN_RADIUS, pin->getState() ? LIME : MAROON);
                DrawCircleLines(pinPos.x, pinPos.y, Config::PIN_RADIUS, DARKGRAY);
            }
        }
    }
}

AndGate::AndGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    initializeInputPin(0, {0, h * INPUT_PIN_TOP_RATIO});
    initializeInputPin(1, {0, h * INPUT_PIN_BOTTOM_RATIO});
    initializeOutputPin(0, {w, h * OUTPUT_PIN_CENTER_RATIO});
}

void AndGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }

    // AND gate: output is true only if all inputs are true
    bool result = true;
    for (size_t i = 0; i < getInputPinCount(); ++i) {
        if (!getInputPin(i)->getState()) {
            result = false;
            break;
        }
    }
    getOutputPin(0)->setState(result);
}

void AndGate::draw() {
    drawPlaceholderGate(this, "AND");
}

OrGate::OrGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    const float actualWidth = fminf(w * OR_XOR_WIDTH_RATIO, h * 0.876f);
    const float leftX = (w - actualWidth) / 2.0f;
    const float curveDepth = h * OR_XOR_CURVE_DEPTH_RATIO;
    const float pinRadius = Config::PIN_CLICK_RADIUS;

    // Calculate input pin positions on the curved edge
    auto calculateCurvedPinX = [&](float heightRatio) -> float {
        const float normalizedT = 2.0f * heightRatio - 1.0f; // Convert to -1 to 1 range
        const float curveAmount = 1.0f - normalizedT * normalizedT;
        const float gateEdgeX = leftX + curveDepth * curveAmount;
        return gateEdgeX - pinRadius; // Pin center positioned so right edge touches gate
    };

    const float inputX0 = calculateCurvedPinX(INPUT_PIN_TOP_RATIO);
    const float inputX1 = calculateCurvedPinX(INPUT_PIN_BOTTOM_RATIO);

    // Output pin positioned so left edge touches right edge of gate body
    const float gateRightEdge = leftX + actualWidth;
    const float outputX = gateRightEdge + pinRadius;

    initializeInputPin(0, {inputX0, h * INPUT_PIN_TOP_RATIO});
    initializeInputPin(1, {inputX1, h * INPUT_PIN_BOTTOM_RATIO});
    initializeOutputPin(0, {outputX, h * OUTPUT_PIN_CENTER_RATIO});
}

void OrGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }

    // OR gate: output is true if any input is true
    bool result = false;
    for (size_t i = 0; i < getInputPinCount(); ++i) {
        if (getInputPin(i)->getState()) {
            result = true;
            break;
        }
    }
    getOutputPin(0)->setState(result);
}

void OrGate::draw() {
    drawPlaceholderGate(this, "OR");
}

XorGate::XorGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    // XOR gate uses similar curved shape as OR gate
    const float actualWidth = fminf(w * OR_XOR_WIDTH_RATIO, h * 0.976f);
    const float leftX = (w - actualWidth) / 2.0f;
    const float curveDepth = h * OR_XOR_CURVE_DEPTH_RATIO;
    const float pinRadius = Config::PIN_CLICK_RADIUS;

    // Calculate input pin positions on the curved edge (same as OR gate)
    auto calculateCurvedPinX = [&](float heightRatio) -> float {
        const float normalizedT = 2.0f * heightRatio - 1.0f;
        const float curveAmount = 1.0f - normalizedT * normalizedT;
        const float gateEdgeX = leftX + curveDepth * curveAmount;
        return gateEdgeX - pinRadius;
    };

    const float inputX0 = calculateCurvedPinX(INPUT_PIN_TOP_RATIO);
    const float inputX1 = calculateCurvedPinX(INPUT_PIN_BOTTOM_RATIO);

    const float gateRightEdge = leftX + actualWidth;
    const float outputX = gateRightEdge + pinRadius;

    initializeInputPin(0, {inputX0, h * INPUT_PIN_TOP_RATIO});
    initializeInputPin(1, {inputX1, h * INPUT_PIN_BOTTOM_RATIO});
    initializeOutputPin(0, {outputX, h * OUTPUT_PIN_CENTER_RATIO});
}

void XorGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }

    // XOR gate: output is true if inputs are different
    const bool input0State = getInputPin(0)->getState();
    const bool input1State = getInputPin(1)->getState();
    getOutputPin(0)->setState(input0State != input1State);
}

void XorGate::draw() {
    drawPlaceholderGate(this, "XOR");
}

NotGate::NotGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    const float triangleHeight = h;
    const float idealWidth = triangleHeight * NOT_TRIANGLE_ASPECT_RATIO;
    const float actualWidth = fminf(w * NOT_WIDTH_RATIO, idealWidth);
    const float leftX = (w - actualWidth) / 2.0f;

    const float pinRadius = Config::PIN_CLICK_RADIUS;

    // Input pin positioned so right edge touches left edge of triangle
    const float inputX = leftX - pinRadius;

    // Output pin positioned after triangle and inversion bubble
    const float triangleRightEdge = leftX + actualWidth;
    const float outputX = triangleRightEdge + INVERSION_BUBBLE_RADIUS + pinRadius;

    initializeInputPin(0, {inputX, h * OUTPUT_PIN_CENTER_RATIO});
    initializeOutputPin(0, {outputX, h * OUTPUT_PIN_CENTER_RATIO});
}

void NotGate::evaluate() {
    if (getInputPinCount() == 0 || getOutputPinCount() == 0) {
        return;
    }

    // NOT gate: output is the inverse of input
    getOutputPin(0)->setState(!getInputPin(0)->getState());
}

void NotGate::draw() {
    drawPlaceholderGate(this, "NOT");
}

