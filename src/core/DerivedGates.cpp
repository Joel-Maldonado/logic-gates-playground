#include "core/DerivedGates.h"
#include "core/GateSymbolGeometry.h"

#include <cmath>
#include <utility>

AndGate::AndGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(std::move(gateId), GateKind::AND_GATE, pos, w, h) {
    const auto pins = GateSymbolGeometry::pinOffsets(GateKind::AND_GATE, {w, h});
    initializeInputPin(0, pins[0]);
    initializeInputPin(1, pins[1]);
    initializeOutputPin(0, pins[2]);
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
    const auto pins = GateSymbolGeometry::pinOffsets(GateKind::OR_GATE, {w, h});
    initializeInputPin(0, pins[0]);
    initializeInputPin(1, pins[1]);
    initializeOutputPin(0, pins[2]);
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
    const auto pins = GateSymbolGeometry::pinOffsets(GateKind::XOR_GATE, {w, h});
    initializeInputPin(0, pins[0]);
    initializeInputPin(1, pins[1]);
    initializeOutputPin(0, pins[2]);
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
    const auto pins = GateSymbolGeometry::pinOffsets(GateKind::NOT_GATE, {w, h});
    initializeInputPin(0, pins[0]);
    initializeOutputPin(0, pins[1]);
}

void NotGate::evaluate() {
    if (getInputPinCount() == 0 || getOutputPinCount() == 0) {
        return;
    }

    getOutputPin(0)->setState(!getInputPin(0)->getState());
}
