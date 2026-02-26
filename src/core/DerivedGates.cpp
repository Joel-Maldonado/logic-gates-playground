#include "core/DerivedGates.h"
#include "app/Config.h"
#include <cmath>
#include <utility>

namespace {
constexpr float INPUT_PIN_TOP_RATIO = 1.0f / 3.0f;
constexpr float INPUT_PIN_BOTTOM_RATIO = 2.0f / 3.0f;
constexpr float OUTPUT_PIN_CENTER_RATIO = 0.5f;

constexpr float OR_XOR_WIDTH_RATIO = 0.8f;
constexpr float OR_XOR_CURVE_DEPTH_RATIO = 0.12f;
constexpr float NOT_WIDTH_RATIO = 0.7f;
constexpr float NOT_TRIANGLE_ASPECT_RATIO = 0.866f;
constexpr float INVERSION_BUBBLE_RADIUS = 5.0f;
}

AndGate::AndGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(std::move(gateId), GateKind::AND_GATE, pos, w, h) {
    initializeInputPin(0, {0, h * INPUT_PIN_TOP_RATIO});
    initializeInputPin(1, {0, h * INPUT_PIN_BOTTOM_RATIO});
    initializeOutputPin(0, {w, h * OUTPUT_PIN_CENTER_RATIO});
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

OrGate::OrGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(std::move(gateId), GateKind::OR_GATE, pos, w, h) {
    const float actualWidth = fminf(w * OR_XOR_WIDTH_RATIO, h * 0.876f);
    const float leftX = (w - actualWidth) / 2.0f;
    const float curveDepth = h * OR_XOR_CURVE_DEPTH_RATIO;
    const float pinRadius = Config::PIN_CLICK_RADIUS;

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

XorGate::XorGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(std::move(gateId), GateKind::XOR_GATE, pos, w, h) {
    const float actualWidth = fminf(w * OR_XOR_WIDTH_RATIO, h * 0.976f);
    const float leftX = (w - actualWidth) / 2.0f;
    const float curveDepth = h * OR_XOR_CURVE_DEPTH_RATIO;
    const float pinRadius = Config::PIN_CLICK_RADIUS;

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

    const bool input0State = getInputPin(0)->getState();
    const bool input1State = getInputPin(1)->getState();
    getOutputPin(0)->setState(input0State != input1State);
}

NotGate::NotGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(std::move(gateId), GateKind::NOT_GATE, pos, w, h) {
    const float triangleHeight = h;
    const float idealWidth = triangleHeight * NOT_TRIANGLE_ASPECT_RATIO;
    const float actualWidth = fminf(w * NOT_WIDTH_RATIO, idealWidth);
    const float leftX = (w - actualWidth) / 2.0f;

    const float pinRadius = Config::PIN_CLICK_RADIUS;

    const float inputX = leftX - pinRadius;
    const float triangleRightEdge = leftX + actualWidth;
    const float outputX = triangleRightEdge + INVERSION_BUBBLE_RADIUS + pinRadius;

    initializeInputPin(0, {inputX, h * OUTPUT_PIN_CENTER_RATIO});
    initializeOutputPin(0, {outputX, h * OUTPUT_PIN_CENTER_RATIO});
}

void NotGate::evaluate() {
    if (getInputPinCount() == 0 || getOutputPinCount() == 0) {
        return;
    }

    getOutputPin(0)->setState(!getInputPin(0)->getState());
}
