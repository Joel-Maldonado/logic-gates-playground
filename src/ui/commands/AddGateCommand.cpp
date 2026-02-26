#include "ui/commands/AddGateCommand.h"

#include "simulation/CircuitSimulator.h"
#include "ui/GateFactory.h"

#include <utility>

AddGateCommand::AddGateCommand(std::shared_ptr<CircuitSimulator> simulator,
                               GateKind kind,
                               Vector2 position,
                               Vector2 size)
    : simulator_(std::move(simulator)),
      kind_(kind),
      position_(position),
      size_(size) {
}

void AddGateCommand::execute() {
    if (!simulator_) {
        return;
    }

    if (gateId_.empty()) {
        gateId_ = "gate" + std::to_string(simulator_->useNextGateId());
    }

    if (simulator_->findGateById(gateId_)) {
        return;
    }

    auto gate = GateFactory::createGate(kind_, gateId_, position_, size_);
    if (!gate) {
        return;
    }

    simulator_->addGate(std::move(gate));
}

void AddGateCommand::undo() {
    if (!simulator_ || gateId_.empty()) {
        return;
    }

    LogicGate* gate = simulator_->findGateById(gateId_);
    if (gate) {
        simulator_->removeGate(gate);
    }
}
