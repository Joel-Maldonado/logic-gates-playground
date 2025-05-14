#include "core/LogicGate.h"
#include "core/Wire.h"
#include "raylib.h"
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <iostream>
#include <set> // Added for std::set
#include "raymath.h"




LogicGate::LogicGate(std::string gateId, Vector2 pos, float w, float h)
    : id(std::move(gateId)), position(pos), width(w), height(h), isDirty(true), isSelected(false) {
}

LogicGate::~LogicGate() {
    // Pin disconnection logic is initiated by LogicGate::prepareForDeletion().
    // The caller (e.g., simulation manager in main.cpp) is responsible for
    // deleting the Wire objects returned by prepareForDeletion() and then this gate.
    // associatedWires is cleared by prepareForDeletion().
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
        // Output pin state changes propagate dirtiness to dependent gates via their setState.
        // This markDirty call is primarily for changes to input sources or internal state
        // that require the gate itself to re-evaluate.
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
        throw std::out_of_range("Input pin index out of range for gate " + id + ". Index: " + std::to_string(pinIndex) + ", Size: " + std::to_string(inputPins.size()));
    }
    return &inputPins[pinIndex];
}

const GatePin* LogicGate::getInputPin(size_t pinIndex) const {
    if (pinIndex >= inputPins.size()) {
        throw std::out_of_range("Input pin index out of range for gate " + id + ". Index: " + std::to_string(pinIndex) + ", Size: " + std::to_string(inputPins.size()));
    }
    return &inputPins[pinIndex];
}

GatePin* LogicGate::getOutputPin(size_t pinIndex) {
    if (pinIndex >= outputPins.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id + ". Index: " + std::to_string(pinIndex) + ", Size: " + std::to_string(outputPins.size()));
    }
    return &outputPins[pinIndex];
}

const GatePin* LogicGate::getOutputPin(size_t pinIndex) const {
    if (pinIndex >= outputPins.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id + ". Index: " + std::to_string(pinIndex) + ", Size: " + std::to_string(outputPins.size()));
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
        throw std::out_of_range("Input pin index out of range for gate " + id + " when calling setInputState. Index: " + std::to_string(pinIndex) + ", Size: " + std::to_string(inputPins.size()));
    }
    // Directly set the input pin's state for testing purposes.
    // This bypasses the usual connection-based state determination.
    inputPins[pinIndex].setState(state);
    markDirty();
}

bool LogicGate::getOutputState(size_t pinIndex) const {
    if (pinIndex >= outputPins.size()) {
        throw std::out_of_range("Output pin index out of range for gate " + id + " when calling getOutputState. Index: " + std::to_string(pinIndex) + ", Size: " + std::to_string(outputPins.size()));
    }
    return outputPins[pinIndex].getState();
}

void LogicGate::setPosition(Vector2 newPosition) {
    position = newPosition;
    // Moving a gate changes the absolute positions of its pins.
    // This doesn't inherently change logic states, so marking dirty might not be
    // strictly necessary unless visual updates or other systems depend on it.
    // For now, we won't markDirty here, as logical state propagation is primary.
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

// New implementation for LogicGate::prepareForDeletion
std::vector<Wire*> LogicGate::prepareForDeletion() {
    std::set<Wire*> uniqueAffectedWiresSet; // Use a set to automatically handle duplicates

    // Iterate through all input pins of this gate
    // Need to use the non-const version of getAllInputPins() to potentially modify pins
    for (GatePin& inputPin : getAllInputPins()) {
        if (inputPin.getSourceOutputPin() != nullptr) { // If the input pin is connected
            // Find the wire that connects to this input pin
            Wire* connectedWire = nullptr;
            for (Wire* w : associatedWires) { // associatedWires is a member of LogicGate
                if (w->getDestPin() == &inputPin && w->getSourcePin() == inputPin.getSourceOutputPin()) {
                    connectedWire = w;
                    break;
                }
            }

            if (connectedWire) {
                uniqueAffectedWiresSet.insert(connectedWire);
                GatePin* sourceOfWire = connectedWire->getSourcePin(); // This is inputPin.getSourceOutputPin()

                // Tell the source pin (potentially on another gate) to disconnect this wire
                if (sourceOfWire) {
                    sourceOfWire->disconnectWire(connectedWire);
                }
                // Tell this input pin to disconnect from this wire
                inputPin.disconnectWire(connectedWire);
            }
        }
    }

    // Iterate through all output pins of this gate
    for (GatePin& outputPin : getAllOutputPins()) {
        // An output pin can be connected to multiple wires.
        // We need to find all wires originating from this outputPin.
        // Iterate over a temporary copy of wires associated with this output pin,
        // as calling disconnectWire might modify the underlying collections indirectly.
        std::vector<Wire*> wiresFromThisOutputPin;
        for (Wire* w : associatedWires) {
            if (w->getSourcePin() == &outputPin) {
                wiresFromThisOutputPin.push_back(w);
            }
        }

        for (Wire* connectedWire : wiresFromThisOutputPin) {
            uniqueAffectedWiresSet.insert(connectedWire);
            GatePin* destOfWire = connectedWire->getDestPin();

            // Tell the destination pin (on another gate) to disconnect from this wire
            if (destOfWire) {
                destOfWire->disconnectWire(connectedWire);
            }
            // Tell this output pin to disconnect from this wire
            outputPin.disconnectWire(connectedWire);
        }
    }

    // The gate itself no longer needs to track these wires as it's being deleted.
    // The actual Wire objects will be deleted by the caller (main.cpp).
    associatedWires.clear();

    // Convert the set of affected wires to a vector to return
    return std::vector<Wire*>(uniqueAffectedWiresSet.begin(), uniqueAffectedWiresSet.end());
}
// Old static LogicGate::handleGateDeletion has been removed.