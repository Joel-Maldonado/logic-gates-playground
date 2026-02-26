#include "ui/commands/DeleteSelectionCommand.h"

#include "core/InputSource.h"
#include "simulation/CircuitSimulator.h"
#include "ui/GateFactory.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

namespace {

int pinIndex(const LogicGate* gate, const GatePin* pin, bool output) {
    if (!gate || !pin) {
        return -1;
    }

    const size_t count = output ? gate->getOutputPinCount() : gate->getInputPinCount();
    for (size_t i = 0; i < count; ++i) {
        const GatePin* candidate = output ? gate->getOutputPin(i) : gate->getInputPin(i);
        if (candidate == pin) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

} // namespace

DeleteSelectionCommand::DeleteSelectionCommand(std::shared_ptr<CircuitSimulator> simulator,
                                               const EditorSelection& selection)
    : simulator_(std::move(simulator)) {
    if (!simulator_) {
        return;
    }

    std::unordered_set<Wire*> uniqueWires;

    for (LogicGate* gate : selection.gates) {
        if (!gate) {
            continue;
        }

        GateSnapshot snapshot{};
        snapshot.kind = gate->getKind();
        snapshot.id = gate->getId();
        snapshot.position = gate->getPosition();
        snapshot.size = {gate->getWidth(), gate->getHeight()};
        snapshot.inputState = false;
        if (gate->getKind() == GateKind::INPUT_SOURCE) {
            snapshot.inputState = static_cast<InputSource*>(gate)->getCurrentState();
        }

        gateSnapshots_.push_back(snapshot);
        gateIdsToDelete_.push_back(snapshot.id);

        for (Wire* wire : gate->getAssociatedWires()) {
            if (wire) {
                uniqueWires.insert(wire);
            }
        }
    }

    for (Wire* wire : selection.wires) {
        if (wire) {
            uniqueWires.insert(wire);
        }
    }

    for (Wire* wire : uniqueWires) {
        GatePin* src = wire->getSourcePin();
        GatePin* dst = wire->getDestPin();
        if (!src || !dst) {
            continue;
        }

        LogicGate* sourceGate = src->getParentGate();
        LogicGate* destGate = dst->getParentGate();
        if (!sourceGate || !destGate) {
            continue;
        }

        WireSnapshot ws{};
        ws.sourceGateId = sourceGate->getId();
        ws.destGateId = destGate->getId();
        ws.sourcePinIndex = pinIndex(sourceGate, src, true);
        ws.destPinIndex = pinIndex(destGate, dst, false);

        if (ws.sourcePinIndex >= 0 && ws.destPinIndex >= 0) {
            wireSnapshots_.push_back(ws);
        }
    }
}

void DeleteSelectionCommand::execute() {
    if (!simulator_) {
        return;
    }

    for (const std::string& gateId : gateIdsToDelete_) {
        LogicGate* gate = simulator_->findGateById(gateId);
        if (gate) {
            simulator_->removeGate(gate);
        }
    }

    for (const WireSnapshot& wireSnapshot : wireSnapshots_) {
        LogicGate* sourceGate = simulator_->findGateById(wireSnapshot.sourceGateId);
        LogicGate* destGate = simulator_->findGateById(wireSnapshot.destGateId);
        if (!sourceGate || !destGate) {
            continue;
        }

        if (wireSnapshot.sourcePinIndex < 0 || wireSnapshot.destPinIndex < 0) {
            continue;
        }

        if (static_cast<size_t>(wireSnapshot.sourcePinIndex) >= sourceGate->getOutputPinCount() ||
            static_cast<size_t>(wireSnapshot.destPinIndex) >= destGate->getInputPinCount()) {
            continue;
        }

        GatePin* src = sourceGate->getOutputPin(static_cast<size_t>(wireSnapshot.sourcePinIndex));
        GatePin* dst = destGate->getInputPin(static_cast<size_t>(wireSnapshot.destPinIndex));
        Wire* wire = simulator_->findWireByPins(src, dst);
        if (wire) {
            simulator_->removeWire(wire);
        }
    }
}

void DeleteSelectionCommand::undo() {
    if (!simulator_) {
        return;
    }

    for (const GateSnapshot& snapshot : gateSnapshots_) {
        if (simulator_->findGateById(snapshot.id)) {
            continue;
        }

        auto gate = GateFactory::createGate(snapshot.kind, snapshot.id, snapshot.position, snapshot.size);
        if (!gate) {
            continue;
        }

        LogicGate* added = simulator_->addGate(std::move(gate));
        if (added && snapshot.kind == GateKind::INPUT_SOURCE) {
            static_cast<InputSource*>(added)->setState(snapshot.inputState);
        }
    }

    for (const WireSnapshot& ws : wireSnapshots_) {
        LogicGate* sourceGate = simulator_->findGateById(ws.sourceGateId);
        LogicGate* destGate = simulator_->findGateById(ws.destGateId);
        if (!sourceGate || !destGate) {
            continue;
        }

        if (ws.sourcePinIndex < 0 || ws.destPinIndex < 0) {
            continue;
        }

        if (static_cast<size_t>(ws.sourcePinIndex) >= sourceGate->getOutputPinCount() ||
            static_cast<size_t>(ws.destPinIndex) >= destGate->getInputPinCount()) {
            continue;
        }

        GatePin* src = sourceGate->getOutputPin(static_cast<size_t>(ws.sourcePinIndex));
        GatePin* dst = destGate->getInputPin(static_cast<size_t>(ws.destPinIndex));
        if (!simulator_->findWireByPins(src, dst) && !dst->isConnectedInput()) {
            simulator_->createWire(src, dst);
        }
    }
}
