#include "core/DerivedGates.h"
#include "app/Config.h"
#include <raylib.h>

void DrawPlaceholderGate(LogicGate* gate, const char* label) {
    Rectangle bounds = gate->getBounds();
    DrawRectangleRec(bounds, LIGHTGRAY);
    DrawRectangleLinesEx(bounds, 2, DARKGRAY);
    DrawText(label, (int)(bounds.x + 5), (int)(bounds.y + 5), 20, BLACK);

    for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
        const GatePin* pin = gate->getInputPin(i);
        if (pin) {
            Vector2 pinPos = pin->getAbsolutePosition();
            DrawCircleV(pinPos, 5, pin->getState() ? GREEN : RED);
            DrawCircleLines(pinPos.x, pinPos.y, 5, DARKGRAY);
        }
    }

    for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
        const GatePin* pin = gate->getOutputPin(i);
        if (pin) {
            Vector2 pinPos = pin->getAbsolutePosition();
            DrawCircleV(pinPos, 5, pin->getState() ? LIME : MAROON);
            DrawCircleLines(pinPos.x, pinPos.y, 5, DARKGRAY);
        }
    }
}

AndGate::AndGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    initializeInputPin(0, {0, h / 3.0f});
    initializeInputPin(1, {0, 2.0f * h / 3.0f});
    initializeOutputPin(0, {w, h / 2.0f});
}

void AndGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }
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
    DrawPlaceholderGate(this, "AND");
}

OrGate::OrGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    // Calculate pin positions to match the triangular OR gate visual shape
    // Use same calculations as in GateRenderer::renderTriangularOrGateShape
    float actualWidth = fminf(w * 0.8f, h * 0.876f);
    float leftX = (w - actualWidth) / 2.0f;
    float curveDepth = h * 0.12f;

    // Input pins: right edge of circle should touch left edge of gate body
    // So pin center should be at (gate edge - pin radius)
    float pinRadius = Config::PIN_CLICK_RADIUS;

    // For input pin 0 (top): at 1/3 height on the curved edge
    float t0 = 1.0f / 3.0f;
    float normalizedT0 = 2.0f * t0 - 1.0f; // -1 to 1
    float curveAmount0 = 1.0f - normalizedT0 * normalizedT0;
    float gateEdgeX0 = leftX + curveDepth * curveAmount0;
    float inputX0 = gateEdgeX0 - pinRadius; // Right edge of pin touches gate edge

    // For input pin 1 (bottom): at 2/3 height on the curved edge
    float t1 = 2.0f / 3.0f;
    float normalizedT1 = 2.0f * t1 - 1.0f; // -1 to 1
    float curveAmount1 = 1.0f - normalizedT1 * normalizedT1;
    float gateEdgeX1 = leftX + curveDepth * curveAmount1;
    float inputX1 = gateEdgeX1 - pinRadius; // Right edge of pin touches gate edge

    // Output pin: left edge of circle should touch right edge of gate body
    // So pin center should be at (gate edge + pin radius)
    float gateRightEdge = leftX + actualWidth;
    float outputX = gateRightEdge + pinRadius; // Left edge of pin touches gate edge

    initializeInputPin(0, {inputX0, h / 3.0f});
    initializeInputPin(1, {inputX1, 2.0f * h / 3.0f});
    initializeOutputPin(0, {outputX, h / 2.0f});
}

void OrGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }
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
    DrawPlaceholderGate(this, "OR");
}

XorGate::XorGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    // Calculate pin positions to match the triangular XOR gate visual shape
    // Use same calculations as in GateRenderer::renderTriangularXorGateShape
    float actualWidth = fminf(w * 0.8f, h * 0.976f);
    float leftX = (w - actualWidth) / 2.0f;
    float curveDepth = h * 0.12f;

    // Input pins: right edge of circle should touch left edge of gate body
    // So pin center should be at (gate edge - pin radius)
    float pinRadius = Config::PIN_CLICK_RADIUS;

    // For input pin 0 (top): at 1/3 height on the curved edge
    float t0 = 1.0f / 3.0f;
    float normalizedT0 = 2.0f * t0 - 1.0f; // -1 to 1
    float curveAmount0 = 1.0f - normalizedT0 * normalizedT0;
    float gateEdgeX0 = leftX + curveDepth * curveAmount0;
    float inputX0 = gateEdgeX0 - pinRadius; // Right edge of pin touches gate edge

    // For input pin 1 (bottom): at 2/3 height on the curved edge
    float t1 = 2.0f / 3.0f;
    float normalizedT1 = 2.0f * t1 - 1.0f; // -1 to 1
    float curveAmount1 = 1.0f - normalizedT1 * normalizedT1;
    float gateEdgeX1 = leftX + curveDepth * curveAmount1;
    float inputX1 = gateEdgeX1 - pinRadius; // Right edge of pin touches gate edge

    // Output pin: left edge of circle should touch right edge of gate body
    // So pin center should be at (gate edge + pin radius)
    float gateRightEdge = leftX + actualWidth;
    float outputX = gateRightEdge + pinRadius; // Left edge of pin touches gate edge

    initializeInputPin(0, {inputX0, h / 3.0f});
    initializeInputPin(1, {inputX1, 2.0f * h / 3.0f});
    initializeOutputPin(0, {outputX, h / 2.0f});
}

void XorGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }
    bool input0State = getInputPin(0)->getState();
    bool input1State = getInputPin(1)->getState();
    getOutputPin(0)->setState(input0State != input1State);
}

void XorGate::draw() {
    DrawPlaceholderGate(this, "XOR");
}

NotGate::NotGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    initializeInputPin(0, {0, h / 2.0f});
    initializeOutputPin(0, {w, h / 2.0f});
}

void NotGate::evaluate() {
    if (getInputPinCount() == 0 || getOutputPinCount() == 0) {
        return;
    }
    getOutputPin(0)->setState(!getInputPin(0)->getState());
}

void NotGate::draw() {
    DrawPlaceholderGate(this, "NOT");
}

