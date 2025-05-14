#include "core/DerivedGates.h"
#include "raylib.h"

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

// --- AndGate ---
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

// --- OrGate ---
OrGate::OrGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    initializeInputPin(0, {0, h / 3.0f});
    initializeInputPin(1, {0, 2.0f * h / 3.0f});
    initializeOutputPin(0, {w, h / 2.0f});
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

// --- XorGate ---
XorGate::XorGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    initializeInputPin(0, {0, h / 3.0f});
    initializeInputPin(1, {0, 2.0f * h / 3.0f});
    initializeOutputPin(0, {w, h / 2.0f});
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

// --- NotGate ---
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

// --- NandGate ---
NandGate::NandGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    initializeInputPin(0, {0, h / 3.0f});
    initializeInputPin(1, {0, 2.0f * h / 3.0f});
    initializeOutputPin(0, {w, h / 2.0f});
}

void NandGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }
    bool and_result = true;
    for (size_t i = 0; i < getInputPinCount(); ++i) {
        if (!getInputPin(i)->getState()) {
            and_result = false;
            break;
        }
    }
    getOutputPin(0)->setState(!and_result);
}

void NandGate::draw() {
    DrawPlaceholderGate(this, "NAND");
}

// --- NorGate ---
NorGate::NorGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    initializeInputPin(0, {0, h / 3.0f});
    initializeInputPin(1, {0, 2.0f * h / 3.0f});
    initializeOutputPin(0, {w, h / 2.0f});
}

void NorGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }
    bool or_result = false;
    for (size_t i = 0; i < getInputPinCount(); ++i) {
        if (getInputPin(i)->getState()) {
            or_result = true;
            break;
        }
    }
    getOutputPin(0)->setState(!or_result);
}

void NorGate::draw() {
    DrawPlaceholderGate(this, "NOR");
}

// --- XnorGate ---
XnorGate::XnorGate(std::string gateId, Vector2 pos, float w, float h)
    : LogicGate(gateId, pos, w, h) {
    initializeInputPin(0, {0, h / 3.0f});
    initializeInputPin(1, {0, 2.0f * h / 3.0f});
    initializeOutputPin(0, {w, h / 2.0f});
}

void XnorGate::evaluate() {
    if (getInputPinCount() < 2 || getOutputPinCount() == 0) {
        return;
    }
    bool input0State = getInputPin(0)->getState();
    bool input1State = getInputPin(1)->getState();
    getOutputPin(0)->setState(input0State == input1State);
}

void XnorGate::draw() {
    DrawPlaceholderGate(this, "XNOR");
}