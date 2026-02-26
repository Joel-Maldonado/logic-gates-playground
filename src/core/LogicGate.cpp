#include "core/LogicGate.h"
#include "core/Wire.h"
#include <raylib.h>
#include <raymath.h>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <iostream>
#include <set>

LogicGate::LogicGate(std::string gateId, GateKind kind, Vector2 pos, float w, float h)
    : id_(std::move(gateId)), gateKind_(kind), position_(pos), width_(w), height_(h), isDirty_(true), isSelected_(false) {
}

LogicGate::~LogicGate() {
}

bool LogicGate::update() {
    if (!isDirty_) {
        return false;
    }

    std::vector<bool> previousOutputs;
    previousOutputs.reserve(outputPins_.size());
    for (const auto& pin : outputPins_) {
        previousOutputs.push_back(pin.getState());
    }

    evaluate();
    isDirty_ = false;

    for (size_t i = 0; i < outputPins_.size(); ++i) {
        if (outputPins_[i].getState() != previousOutputs[i]) {
            return true;
        }
    }

    return false;
}

void LogicGate::markDirty() {
    if (!isDirty_) {
        isDirty_ = true;
    }
}

bool LogicGate::needsEvaluation() const {
    return isDirty_;
}

void LogicGate::initializeInputPin(int pinId, Vector2 relativeOffset) {
    inputPins_.emplace_back(this, PinType::INPUT_PIN, pinId, relativeOffset);
}

void LogicGate::initializeOutputPin(int pinId, Vector2 relativeOffset) {
    outputPins_.emplace_back(this, PinType::OUTPUT_PIN, pinId, relativeOffset);
}

GatePin* LogicGate::getInputPin(size_t pinIndex) {
    if (pinIndex >= inputPins_.size()) {
        throw std::out_of_range("Input pin index out of range for gate " + id_);
    }
    return &inputPins_[pinIndex];
}

const GatePin* LogicGate::getInputPin(size_t pinIndex) const {
    if (pinIndex >= inputPins_.size()) {
        throw std::out_of_range("Input pin index out of range for gate " + id_);
    }
    return &inputPins_[pinIndex];
}

GatePin* LogicGate::getOutputPin(size_t pinIndex) {
    if (pinIndex >= outputPins_.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id_);
    }
    return &outputPins_[pinIndex];
}

const GatePin* LogicGate::getOutputPin(size_t pinIndex) const {
    if (pinIndex >= outputPins_.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id_);
    }
    return &outputPins_[pinIndex];
}

size_t LogicGate::getInputPinCount() const {
    return inputPins_.size();
}

size_t LogicGate::getOutputPinCount() const {
    return outputPins_.size();
}

void LogicGate::setInputState(size_t pinIndex, bool state) {
    if (pinIndex >= inputPins_.size()) {
        throw std::out_of_range("Input pin index out of range for gate " + id_);
    }
    inputPins_[pinIndex].setState(state);
    markDirty();
}

bool LogicGate::getOutputState(size_t pinIndex) const {
    if (pinIndex >= outputPins_.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id_);
    }
    return outputPins_[pinIndex].getState();
}

void LogicGate::setPosition(Vector2 newPosition) {
    position_ = newPosition;
}

Vector2 LogicGate::getPosition() const {
    return position_;
}

Rectangle LogicGate::getBounds() const {
    return { position_.x, position_.y, width_, height_ };
}

bool LogicGate::isMouseOver(Vector2 mousePos) const {
    return CheckCollisionPointRec(mousePos, getBounds());
}

GatePin* LogicGate::getPinAt(Vector2 mousePos, float tolerance) {
    for (GatePin& pin : inputPins_) {
        if (Vector2Distance(mousePos, pin.getAbsolutePosition()) <= pin.getClickRadius() + tolerance) {
            return &pin;
        }
    }
    for (GatePin& pin : outputPins_) {
        if (Vector2Distance(mousePos, pin.getAbsolutePosition()) <= pin.getClickRadius() + tolerance) {
            return &pin;
        }
    }
    return nullptr;
}

void LogicGate::setSelected(bool selected) {
    isSelected_ = selected;
}

bool LogicGate::getIsSelected() const {
    return isSelected_;
}

std::string LogicGate::getId() const {
    return id_;
}

GateKind LogicGate::getKind() const {
    return gateKind_;
}

float LogicGate::getWidth() const {
    return width_;
}

float LogicGate::getHeight() const {
    return height_;
}

void LogicGate::addWire(Wire* wire) {
    if (wire && std::find(associatedWires_.begin(), associatedWires_.end(), wire) == associatedWires_.end()) {
        associatedWires_.push_back(wire);
    }
}

void LogicGate::removeWire(Wire* wire) {
    associatedWires_.erase(std::remove(associatedWires_.begin(), associatedWires_.end(), wire), associatedWires_.end());
}

const std::vector<Wire*>& LogicGate::getAssociatedWires() const {
    return associatedWires_;
}

std::vector<Wire*> LogicGate::prepareForDeletion() {
    std::set<Wire*> uniqueAffectedWiresSet;

    for (GatePin& inputPin : getAllInputPins()) {
        if (inputPin.getSourceOutputPin() != nullptr) {
            Wire* connectedWire = nullptr;
            for (Wire* w : associatedWires_) {
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
        for (Wire* w : associatedWires_) {
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

    associatedWires_.clear();
    return std::vector<Wire*>(uniqueAffectedWiresSet.begin(), uniqueAffectedWiresSet.end());
}
