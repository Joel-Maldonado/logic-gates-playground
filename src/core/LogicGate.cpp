#include "core/LogicGate.h"
#include "core/Wire.h"
#include "raylib.h"
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <iostream>
#include <set>
#include "raymath.h"

LogicGate::LogicGate(std::string gateId, Vector2 pos, float w, float h)
    : id(std::move(gateId)), position(pos), width(w), height(h), isDirty(true), isSelected(false) {
}

LogicGate::~LogicGate() {
}

void LogicGate::update() {
    if (isDirty) {
        evaluate();
        isDirty = false;
    }
}

void LogicGate::markDirty() {
    if (!isDirty) {
        isDirty = true;
    }
}

bool LogicGate::needsEvaluation() const {
    return isDirty;
}

void LogicGate::initializeInputPin(int pinId, Vector2 relativeOffset) {
    inputPins.emplace_back(this, PinType::INPUT_PIN, pinId, relativeOffset);
}

void LogicGate::initializeOutputPin(int pinId, Vector2 relativeOffset) {
    outputPins.emplace_back(this, PinType::OUTPUT_PIN, pinId, relativeOffset);
}

GatePin* LogicGate::getInputPin(size_t pinIndex) {
    if (pinIndex >= inputPins.size()) {
        throw std::out_of_range("Input pin index out of range for gate " + id);
    }
    return &inputPins[pinIndex];
}

const GatePin* LogicGate::getInputPin(size_t pinIndex) const {
    if (pinIndex >= inputPins.size()) {
        throw std::out_of_range("Input pin index out of range for gate " + id);
    }
    return &inputPins[pinIndex];
}

GatePin* LogicGate::getOutputPin(size_t pinIndex) {
    if (pinIndex >= outputPins.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id);
    }
    return &outputPins[pinIndex];
}

const GatePin* LogicGate::getOutputPin(size_t pinIndex) const {
    if (pinIndex >= outputPins.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id);
    }
    return &outputPins[pinIndex];
}

size_t LogicGate::getInputPinCount() const {
    return inputPins.size();
}

size_t LogicGate::getOutputPinCount() const {
    return outputPins.size();
}

void LogicGate::setInputState(size_t pinIndex, bool state) {
    if (pinIndex >= inputPins.size()) {
        throw std::out_of_range("Input pin index out of range for gate " + id);
    }
    inputPins[pinIndex].setState(state);
    markDirty();
}

bool LogicGate::getOutputState(size_t pinIndex) const {
    if (pinIndex >= outputPins.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id);
    }
    return outputPins[pinIndex].getState();
}

void LogicGate::setPosition(Vector2 newPosition) {
    position = newPosition;
}

Vector2 LogicGate::getPosition() const {
    return position;
}

Rectangle LogicGate::getBounds() const {
    return { position.x, position.y, width, height };
}

bool LogicGate::isMouseOver(Vector2 mousePos) const {
    return CheckCollisionPointRec(mousePos, getBounds());
}

GatePin* LogicGate::getPinAt(Vector2 mousePos, float tolerance) {
    for (GatePin& pin : inputPins) {
        if (Vector2Distance(mousePos, pin.getAbsolutePosition()) <= pin.getClickRadius() + tolerance) {
            return &pin;
        }
    }
    for (GatePin& pin : outputPins) {
        if (Vector2Distance(mousePos, pin.getAbsolutePosition()) <= pin.getClickRadius() + tolerance) {
            return &pin;
        }
    }
    return nullptr;
}

void LogicGate::setSelected(bool selected) {
    isSelected = selected;
}

bool LogicGate::getIsSelected() const {
    return isSelected;
}

std::string LogicGate::getId() const {
    return id;
}

float LogicGate::getWidth() const {
    return width;
}

float LogicGate::getHeight() const {
    return height;
}

void LogicGate::addWire(Wire* wire) {
    if (wire && std::find(associatedWires.begin(), associatedWires.end(), wire) == associatedWires.end()) {
        associatedWires.push_back(wire);
    }
}

void LogicGate::removeWire(Wire* wire) {
    associatedWires.erase(std::remove(associatedWires.begin(), associatedWires.end(), wire), associatedWires.end());
}

const std::vector<Wire*>& LogicGate::getAssociatedWires() const {
    return associatedWires;
}

std::vector<Wire*> LogicGate::prepareForDeletion() {
    std::set<Wire*> uniqueAffectedWiresSet;

    for (GatePin& inputPin : getAllInputPins()) {
        if (inputPin.getSourceOutputPin() != nullptr) {
            Wire* connectedWire = nullptr;
            for (Wire* w : associatedWires) {
                if (w->getDestPin() == &inputPin && w->getSourcePin() == inputPin.getSourceOutputPin()) {
                    connectedWire = w;
                    break;
                }
            }

            if (connectedWire) {
                uniqueAffectedWiresSet.insert(connectedWire);
                GatePin* sourceOfWire = connectedWire->getSourcePin();

                if (sourceOfWire) {
                    sourceOfWire->disconnectWire(connectedWire);
                }
                inputPin.disconnectWire(connectedWire);
            }
        }
    }

    for (GatePin& outputPin : getAllOutputPins()) {
        std::vector<Wire*> wiresFromThisOutputPin;
        for (Wire* w : associatedWires) {
            if (w->getSourcePin() == &outputPin) {
                wiresFromThisOutputPin.push_back(w);
            }
        }

        for (Wire* connectedWire : wiresFromThisOutputPin) {
            uniqueAffectedWiresSet.insert(connectedWire);
            GatePin* destOfWire = connectedWire->getDestPin();

            if (destOfWire) {
                destOfWire->disconnectWire(connectedWire);
            }
            outputPin.disconnectWire(connectedWire);
        }
    }

    associatedWires.clear();
    return std::vector<Wire*>(uniqueAffectedWiresSet.begin(), uniqueAffectedWiresSet.end());
}