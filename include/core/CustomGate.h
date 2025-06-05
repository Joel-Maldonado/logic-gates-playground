#ifndef CUSTOMGATE_H
#define CUSTOMGATE_H

#include "core/LogicGate.h"
#include "core/CustomGateData.h"
#include "simulation/CircuitSimulator.h" // Assuming this path is correct
#include <memory>
#include <vector>
#include <string>

// Forward declaration if GatePin is defined elsewhere and only pointers/references are used in this header
// class GatePin;

class CustomGate : public LogicGate {
public:
    CustomGate(std::string gateId, Vector2 pos, const CustomGateData& definition);

    void evaluate() override;
    void draw() override;
    // Optional overrides if needed:
    // int getInputPinCount() const override;
    // int getOutputPinCount() const override;

private:
    CustomGateData definition_;
    std::unique_ptr<CircuitSimulator> internalCircuit_;

    // Using GatePin* for now, assuming GatePin is a known type.
    // If GatePin is defined in a way that causes include issues,
    // we might need to forward declare it or use a more abstract type.
    std::vector<GatePin*> externalToInternalInputPinMap_;
    std::vector<GatePin*> internalToExternalOutputPinMap_;

    void setupInternalCircuit();

    bool isValid_; // Flag to indicate if setup was successful
};

#endif // CUSTOMGATE_H
