#ifndef CIRCUIT_SIMULATOR_H
#define CIRCUIT_SIMULATOR_H

#include "core/LogicGate.h"
#include "core/Wire.h"
#include <vector>
#include <memory>
#include <string>

/**
 * Main circuit simulation engine.
 * Manages logic gates, wires, and handles circuit evaluation.
 */
class CircuitSimulator {
private:
    std::vector<std::unique_ptr<LogicGate>> gates_;
    std::vector<std::unique_ptr<Wire>> wires_;
    int nextGateId_;

public:
    CircuitSimulator();

    /** Updates all gates and wires in the circuit */
    void update();

    // Component management
    LogicGate* addGate(std::unique_ptr<LogicGate> gate);
    Wire* createWire(GatePin* sourcePin, GatePin* destPin);
    bool removeGate(LogicGate* gate);
    bool removeWire(Wire* wire);

    // Accessors
    const std::vector<std::unique_ptr<LogicGate>>& getGates() const;
    const std::vector<std::unique_ptr<Wire>>& getWires() const;

    // ID management
    int getNextGateId() const;
    int useNextGateId();

    /** Clears all gates and wires from the circuit */
    void clear();
};

#endif // CIRCUIT_SIMULATOR_H
