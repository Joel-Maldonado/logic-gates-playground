#include "simulation/CircuitSimulator.h"
#include <algorithm>
#include <raylib.h>

CircuitSimulator::CircuitSimulator()
    : nextGateId_(0),
      lastStats_({0, true, false}) {
}

CircuitSimulator::SimulationStats CircuitSimulator::update() {
    SimulationStats stats = {0, true, false};

    if (gates_.empty() && wires_.empty()) {
        lastStats_ = stats;
        return stats;
    }

    bool stable = false;

    for (int pass = 1; pass <= MAX_SIMULATION_PASSES; ++pass) {
        bool anyWireChanged = false;
        bool anyGateChanged = false;

        // Update wires first to propagate source changes to destination gates.
        for (const auto& wire : wires_) {
            anyWireChanged = wire->update() || anyWireChanged;
        }

        for (const auto& gate : gates_) {
            if (gate->needsEvaluation()) {
                anyGateChanged = gate->update() || anyGateChanged;
            }
        }

        bool hasDirtyGates = false;
        for (const auto& gate : gates_) {
            if (gate->needsEvaluation()) {
                hasDirtyGates = true;
                break;
            }
        }

        stats.passes = pass;

        if (!anyWireChanged && !anyGateChanged && !hasDirtyGates) {
            stable = true;
            break;
        }
    }

    if (!stable) {
        stats.stable = false;
        stats.oscillating = true;
        TraceLog(LOG_WARNING, "Simulation reached max passes (%d); circuit may be oscillating", MAX_SIMULATION_PASSES);
    }

    lastStats_ = stats;
    return stats;
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
        TraceLog(LOG_WARNING, "Failed to create wire: %s", e.what());
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

bool CircuitSimulator::bringGateToFront(LogicGate* gate) {
    if (!gate) {
        return false;
    }

    auto it = std::find_if(gates_.begin(), gates_.end(),
        [gate](const std::unique_ptr<LogicGate>& ptr) {
            return ptr.get() == gate;
        });

    if (it == gates_.end() || std::next(it) == gates_.end()) {
        return it != gates_.end();
    }

    std::rotate(it, std::next(it), gates_.end());
    return true;
}

bool CircuitSimulator::bringWireToFront(Wire* wire) {
    if (!wire) {
        return false;
    }

    auto it = std::find_if(wires_.begin(), wires_.end(),
        [wire](const std::unique_ptr<Wire>& ptr) {
            return ptr.get() == wire;
        });

    if (it == wires_.end() || std::next(it) == wires_.end()) {
        return it != wires_.end();
    }

    std::rotate(it, std::next(it), wires_.end());
    return true;
}

LogicGate* CircuitSimulator::findGateById(const std::string& id) const {
    auto it = std::find_if(gates_.begin(), gates_.end(),
        [&id](const std::unique_ptr<LogicGate>& gatePtr) {
            return gatePtr && gatePtr->getId() == id;
        });

    if (it == gates_.end()) {
        return nullptr;
    }

    return it->get();
}

Wire* CircuitSimulator::findWireByPins(const GatePin* sourcePin, const GatePin* destPin) const {
    auto it = std::find_if(wires_.begin(), wires_.end(),
        [sourcePin, destPin](const std::unique_ptr<Wire>& wirePtr) {
            if (!wirePtr) {
                return false;
            }

            return wirePtr->getSourcePin() == sourcePin && wirePtr->getDestPin() == destPin;
        });

    if (it == wires_.end()) {
        return nullptr;
    }

    return it->get();
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

void CircuitSimulator::setNextGateId(int value) {
    nextGateId_ = value;
}

CircuitSimulator::SimulationStats CircuitSimulator::getLastStats() const {
    return lastStats_;
}

void CircuitSimulator::clear() {
    wires_.clear();
    gates_.clear();
    nextGateId_ = 0;
    lastStats_ = {0, true, false};
}
