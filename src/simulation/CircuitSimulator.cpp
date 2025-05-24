#include "simulation/CircuitSimulator.h"
#include <algorithm>
#include <iostream>

CircuitSimulator::CircuitSimulator() : nextGateId_(0) {
}

void CircuitSimulator::update() {
    // Update wire states first to propagate signals
    for (const auto& wire : wires_) {
        wire->update();
    }

    // Update gates twice to handle multi-level logic propagation
    for (const auto& gate : gates_) {
        gate->update();
    }

    for (const auto& gate : gates_) {
        gate->update();
    }
}

LogicGate* CircuitSimulator::addGate(std::unique_ptr<LogicGate> gate) {
    if (!gate) {
        return nullptr;
    }

    LogicGate* rawPtr = gate.get();
    gates_.push_back(std::move(gate));
    return rawPtr;
}

Wire* CircuitSimulator::createWire(GatePin* sourcePin, GatePin* destPin) {
    if (!sourcePin || !destPin) {
        return nullptr;
    }

    try {
        auto wire = std::make_unique<Wire>(sourcePin, destPin);
        Wire* rawPtr = wire.get();
        wires_.push_back(std::move(wire));
        return rawPtr;
    } catch (const std::exception& e) {
        return nullptr;
    }
}

bool CircuitSimulator::removeGate(LogicGate* gate) {
    if (!gate) {
        return false;
    }

    // Clean up all wires connected to this gate before deletion
    std::vector<Wire*> wiresToRemove = gate->prepareForDeletion();

    for (Wire* wire : wiresToRemove) {
        removeWire(wire);
    }

    // Find and remove the gate from the collection
    auto it = std::find_if(gates_.begin(), gates_.end(),
        [gate](const std::unique_ptr<LogicGate>& ptr) {
            return ptr.get() == gate;
        });

    if (it != gates_.end()) {
        gates_.erase(it);
        return true;
    }

    return false;
}

bool CircuitSimulator::removeWire(Wire* wire) {
    if (!wire) {
        return false;
    }

    if (wire->getSourcePin() && wire->getDestPin()) {
        wire->getDestPin()->disconnectSource();
    }

    if (wire->getSourcePin() && wire->getSourcePin()->getParentGate()) {
        wire->getSourcePin()->getParentGate()->removeWire(wire);
    }
    if (wire->getDestPin() && wire->getDestPin()->getParentGate()) {
        wire->getDestPin()->getParentGate()->removeWire(wire);
    }

    auto it = std::find_if(wires_.begin(), wires_.end(),
        [wire](const std::unique_ptr<Wire>& ptr) {
            return ptr.get() == wire;
        });

    if (it != wires_.end()) {
        wires_.erase(it);
        return true;
    }

    return false;
}

const std::vector<std::unique_ptr<LogicGate>>& CircuitSimulator::getGates() const {
    return gates_;
}

const std::vector<std::unique_ptr<Wire>>& CircuitSimulator::getWires() const {
    return wires_;
}

int CircuitSimulator::getNextGateId() const {
    return nextGateId_;
}

int CircuitSimulator::useNextGateId() {
    return nextGateId_++;
}

void CircuitSimulator::clear() {
    wires_.clear();
    gates_.clear();
    nextGateId_ = 0;
}
