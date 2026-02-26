#include "ui/commands/MoveGatesCommand.h"

#include "simulation/CircuitSimulator.h"

#include <algorithm>

MoveGatesCommand::MoveGatesCommand(std::shared_ptr<CircuitSimulator> simulator,
                                   std::vector<std::string> gateIds,
                                   std::vector<Vector2> fromPositions,
                                   std::vector<Vector2> toPositions)
    : simulator_(std::move(simulator)),
      gateIds_(std::move(gateIds)),
      fromPositions_(std::move(fromPositions)),
      toPositions_(std::move(toPositions)) {
}

void MoveGatesCommand::execute() {
    apply(toPositions_);
}

void MoveGatesCommand::undo() {
    apply(fromPositions_);
}

bool MoveGatesCommand::mergeWith(const EditorCommand& other) {
    const auto* otherMove = dynamic_cast<const MoveGatesCommand*>(&other);
    if (!otherMove) {
        return false;
    }

    if (gateIds_ != otherMove->gateIds_) {
        return false;
    }

    toPositions_ = otherMove->toPositions_;
    return true;
}

void MoveGatesCommand::apply(const std::vector<Vector2>& positions) {
    if (!simulator_) {
        return;
    }

    const size_t count = std::min(gateIds_.size(), positions.size());
    for (size_t i = 0; i < count; ++i) {
        LogicGate* gate = simulator_->findGateById(gateIds_[i]);
        if (!gate) {
            continue;
        }

        gate->setPosition(positions[i]);
        for (Wire* wire : gate->getAssociatedWires()) {
            if (wire) {
                wire->recalculatePath();
            }
        }
    }
}
