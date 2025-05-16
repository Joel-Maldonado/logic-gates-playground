#ifndef CIRCUIT_SIMULATOR_H
#define CIRCUIT_SIMULATOR_H

#include "core/LogicGate.h"
#include "core/Wire.h"
#include <vector>
#include <memory>
#include <string>

class CircuitSimulator {
private:
    std::vector<std::unique_ptr<LogicGate>> gates;
    std::vector<std::unique_ptr<Wire>> wires;
    int nextGateId;

public:
    CircuitSimulator();
    void update();

    LogicGate* addGate(std::unique_ptr<LogicGate> gate);
    Wire* createWire(GatePin* sourcePin, GatePin* destPin);
    bool removeGate(LogicGate* gate);
    bool removeWire(Wire* wire);

    const std::vector<std::unique_ptr<LogicGate>>& getGates() const;
    const std::vector<std::unique_ptr<Wire>>& getWires() const;

    int getNextGateId() const;
    int useNextGateId();
    void clear();
};

#endif // CIRCUIT_SIMULATOR_H
