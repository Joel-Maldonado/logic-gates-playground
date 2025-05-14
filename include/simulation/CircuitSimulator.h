#ifndef CIRCUIT_SIMULATOR_H
#define CIRCUIT_SIMULATOR_H

#include "core/LogicGate.h"
#include "core/Wire.h"
#include <vector>
#include <memory>
#include <string>

/**
 * Manages the simulation of a logic circuit
 * Handles the collection of gates and wires, and their evaluation
 */
class CircuitSimulator {
private:
    std::vector<std::unique_ptr<LogicGate>> gates;
    std::vector<std::unique_ptr<Wire>> wires;
    int nextGateId;

public:
    /**
     * Constructs a new CircuitSimulator
     */
    CircuitSimulator();

    /**
     * Updates the simulation state
     * Evaluates all gates and propagates signals through wires
     */
    void update();

    /**
     * Adds a gate to the simulation
     * 
     * @param gate Unique pointer to the gate to add
     * @return Raw pointer to the added gate
     */
    LogicGate* addGate(std::unique_ptr<LogicGate> gate);

    /**
     * Creates a wire between two pins
     * 
     * @param sourcePin Source pin (output pin)
     * @param destPin Destination pin (input pin)
     * @return Raw pointer to the created wire, or nullptr if creation failed
     */
    Wire* createWire(GatePin* sourcePin, GatePin* destPin);

    /**
     * Removes a gate from the simulation
     * Also removes any connected wires
     * 
     * @param gate Pointer to the gate to remove
     * @return True if the gate was found and removed
     */
    bool removeGate(LogicGate* gate);

    /**
     * Removes a wire from the simulation
     * 
     * @param wire Pointer to the wire to remove
     * @return True if the wire was found and removed
     */
    bool removeWire(Wire* wire);

    /**
     * Gets all gates in the simulation
     * 
     * @return Const reference to the vector of gates
     */
    const std::vector<std::unique_ptr<LogicGate>>& getGates() const;

    /**
     * Gets all wires in the simulation
     * 
     * @return Const reference to the vector of wires
     */
    const std::vector<std::unique_ptr<Wire>>& getWires() const;

    /**
     * Gets the next available gate ID
     * 
     * @return Next gate ID
     */
    int getNextGateId() const;

    /**
     * Increments the next gate ID
     * 
     * @return The ID that was just used
     */
    int useNextGateId();

    /**
     * Clears all gates and wires from the simulation
     */
    void clear();
};

#endif // CIRCUIT_SIMULATOR_H
