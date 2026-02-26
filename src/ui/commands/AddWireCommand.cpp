#include "ui/commands/AddWireCommand.h"

#include "simulation/CircuitSimulator.h"

#include <utility>

namespace {

GatePin* outputPinAt(LogicGate* gate, int index) {
    if (!gate || index < 0) {
        return nullptr;
    }
    if (static_cast<size_t>(index) >= gate->getOutputPinCount()) {
        return nullptr;
    }
    return gate->getOutputPin(static_cast<size_t>(index));
}

GatePin* inputPinAt(LogicGate* gate, int index) {
    if (!gate || index < 0) {
        return nullptr;
    }
    if (static_cast<size_t>(index) >= gate->getInputPinCount()) {
        return nullptr;
    }
    return gate->getInputPin(static_cast<size_t>(index));
}

} // namespace

AddWireCommand::AddWireCommand(std::shared_ptr<CircuitSimulator> simulator,
                               std::string sourceGateId,
                               int sourcePinIndex,
                               std::string destGateId,
                               int destPinIndex)
    : simulator_(std::move(simulator)),
      sourceGateId_(std::move(sourceGateId)),
      destGateId_(std::move(destGateId)),
      sourcePinIndex_(sourcePinIndex),
      destPinIndex_(destPinIndex) {
}

void AddWireCommand::execute() {
    if (!simulator_) {
        return;
    }

    LogicGate* sourceGate = simulator_->findGateById(sourceGateId_);
    LogicGate* destGate = simulator_->findGateById(destGateId_);
    GatePin* src = outputPinAt(sourceGate, sourcePinIndex_);
    GatePin* dst = inputPinAt(destGate, destPinIndex_);

    if (!src || !dst) {
        return;
    }

    if (simulator_->findWireByPins(src, dst)) {
        return;
    }

    simulator_->createWire(src, dst);
}

void AddWireCommand::undo() {
    if (!simulator_) {
        return;
    }

    LogicGate* sourceGate = simulator_->findGateById(sourceGateId_);
    LogicGate* destGate = simulator_->findGateById(destGateId_);
    GatePin* src = outputPinAt(sourceGate, sourcePinIndex_);
    GatePin* dst = inputPinAt(destGate, destPinIndex_);

    if (!src || !dst) {
        return;
    }

    Wire* wire = simulator_->findWireByPins(src, dst);
    if (wire) {
        simulator_->removeWire(wire);
    }
}
