#include "simulation/CircuitSimulator.h"
#include <algorithm>
#include <iostream>

CircuitSimulator::CircuitSimulator() : nextGateId(0) {
}

void CircuitSimulator::update() {
    for (const auto& wire : wires) {
        wire->update();
    }

    for (const auto& gate : gates) {
        gate->update();
    }

    for (const auto& gate : gates) {
        gate->update();
    }
}

LogicGate* CircuitSimulator::addGate(std::unique_ptr<LogicGate> gate) {
    if (!gate) {
        return nullptr;
    }

    LogicGate* rawPtr = gate.get();
    gates.push_back(std::move(gate));
    return rawPtr;
}

Wire* CircuitSimulator::createWire(GatePin* sourcePin, GatePin* destPin) {
    if (!sourcePin || !destPin) {
        return nullptr;
    }

    try {
        auto wire = std::make_unique<Wire>(sourcePin, destPin);
        Wire* rawPtr = wire.get();
        wires.push_back(std::move(wire));
        return rawPtr;
    } catch (const std::exception& e) {
        return nullptr;
    }
}

bool CircuitSimulator::removeGate(LogicGate* gate) {
    if (!gate) {
        return false;
    }

    std::vector<Wire*> wiresToRemove = gate->prepareForDeletion();

    for (Wire* wire : wiresToRemove) {
        removeWire(wire);
    }

    auto it = std::find_if(gates.begin(), gates.end(),
        [gate](const std::unique_ptr<LogicGate>& ptr) {
            return ptr.get() == gate;
        });

    if (it != gates.end()) {
        gates.erase(it);
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

    auto it = std::find_if(wires.begin(), wires.end(),
        [wire](const std::unique_ptr<Wire>& ptr) {
            return ptr.get() == wire;
        });

    if (it != wires.end()) {
        wires.erase(it);
        return true;
    }

    return false;
}

const std::vector<std::unique_ptr<LogicGate>>& CircuitSimulator::getGates() const {
    return gates;
}

const std::vector<std::unique_ptr<Wire>>& CircuitSimulator::getWires() const {
    return wires;
}

int CircuitSimulator::getNextGateId() const {
    return nextGateId;
}

int CircuitSimulator::useNextGateId() {
    return nextGateId++;
}

void CircuitSimulator::clear() {
    wires.clear();
    gates.clear();
    nextGateId = 0;
}
