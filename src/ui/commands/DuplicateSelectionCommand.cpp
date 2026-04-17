#include "ui/commands/DuplicateSelectionCommand.h"

#include "core/InputSource.h"
#include "simulation/CircuitSimulator.h"
#include "ui/GateFactory.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace {

int outputPinIndex(const LogicGate* gate, const GatePin* pin) {
    if (!gate || !pin) {
        return -1;
    }

    for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
        if (gate->getOutputPin(i) == pin) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

int inputPinIndex(const LogicGate* gate, const GatePin* pin) {
    if (!gate || !pin) {
        return -1;
    }

    for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
        if (gate->getInputPin(i) == pin) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

} // namespace

DuplicateSelectionCommand::DuplicateSelectionCommand(std::shared_ptr<CircuitSimulator> simulator,
                                                     const EditorSelection& selection,
                                                     Vector2 offset)
    : simulator_(std::move(simulator)),
      offset_(offset),
      initialized_(false) {
    for (LogicGate* gate : selection.gates) {
        if (gate) {
            sourceGateIds_.push_back(gate->getId());
        }
    }
}

void DuplicateSelectionCommand::execute() {
    if (!simulator_ || sourceGateIds_.empty()) {
        return;
    }

    if (initialized_) {
        for (const GateSnapshot& snapshot : createdGateSnapshots_) {
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

        for (const WireSnapshot& wireSnapshot : createdWireSnapshots_) {
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
            if (!simulator_->findWireByPins(src, dst) && !dst->isConnectedInput()) {
                simulator_->createWire(src, dst);
            }
        }

        return;
    }

    std::unordered_map<std::string, std::string> sourceToCreated;
    std::unordered_set<std::string> sourceSet(sourceGateIds_.begin(), sourceGateIds_.end());

    std::vector<Wire*> originalWires;
    originalWires.reserve(simulator_->getWires().size());
    for (const auto& wire : simulator_->getWires()) {
        if (wire) {
            originalWires.push_back(wire.get());
        }
    }

    createdGateIds_.clear();
    createdGateSnapshots_.clear();
    createdWireSnapshots_.clear();
    initialized_ = true;

    for (const std::string& sourceId : sourceGateIds_) {
        LogicGate* sourceGate = simulator_->findGateById(sourceId);
        if (!sourceGate) {
            continue;
        }

        const std::string newId = "gate" + std::to_string(simulator_->useNextGateId());
        Vector2 newPos = sourceGate->getPosition();
        newPos.x += offset_.x;
        newPos.y += offset_.y;

        auto clone = GateFactory::createGate(sourceGate->getKind(),
                                             newId,
                                             newPos,
                                             {sourceGate->getWidth(), sourceGate->getHeight()});
        if (!clone) {
            continue;
        }

        LogicGate* added = simulator_->addGate(std::move(clone));
        if (added && added->getKind() == GateKind::INPUT_SOURCE) {
            static_cast<InputSource*>(added)->setState(
                static_cast<InputSource*>(sourceGate)->getCurrentState());
        }

        createdGateIds_.push_back(newId);
        sourceToCreated[sourceId] = newId;

        GateSnapshot snapshot{};
        snapshot.kind = sourceGate->getKind();
        snapshot.id = newId;
        snapshot.position = newPos;
        snapshot.size = {sourceGate->getWidth(), sourceGate->getHeight()};
        snapshot.inputState = false;
        if (sourceGate->getKind() == GateKind::INPUT_SOURCE) {
            snapshot.inputState = static_cast<InputSource*>(sourceGate)->getCurrentState();
        }
        createdGateSnapshots_.push_back(snapshot);
    }

    std::sort(createdGateIds_.begin(), createdGateIds_.end());
    std::sort(createdGateSnapshots_.begin(),
              createdGateSnapshots_.end(),
              [](const GateSnapshot& a, const GateSnapshot& b) {
                  return a.id < b.id;
              });

    for (Wire* wire : originalWires) {
        GatePin* src = wire->getSourcePin();
        GatePin* dst = wire->getDestPin();
        if (!src || !dst) {
            continue;
        }

        LogicGate* srcGate = src->getParentGate();
        LogicGate* dstGate = dst->getParentGate();
        if (!srcGate || !dstGate) {
            continue;
        }

        const std::string srcId = srcGate->getId();
        const std::string dstId = dstGate->getId();
        if (!sourceSet.count(srcId) || !sourceSet.count(dstId)) {
            continue;
        }

        auto srcMappedIt = sourceToCreated.find(srcId);
        auto dstMappedIt = sourceToCreated.find(dstId);
        if (srcMappedIt == sourceToCreated.end() || dstMappedIt == sourceToCreated.end()) {
            continue;
        }

        LogicGate* newSrcGate = simulator_->findGateById(srcMappedIt->second);
        LogicGate* newDstGate = simulator_->findGateById(dstMappedIt->second);
        if (!newSrcGate || !newDstGate) {
            continue;
        }

        const int srcPinIndex = outputPinIndex(srcGate, src);
        const int dstPinIndex = inputPinIndex(dstGate, dst);
        if (srcPinIndex < 0 || dstPinIndex < 0) {
            continue;
        }

        if (static_cast<size_t>(srcPinIndex) >= newSrcGate->getOutputPinCount() ||
            static_cast<size_t>(dstPinIndex) >= newDstGate->getInputPinCount()) {
            continue;
        }

        GatePin* newSrc = newSrcGate->getOutputPin(static_cast<size_t>(srcPinIndex));
        GatePin* newDst = newDstGate->getInputPin(static_cast<size_t>(dstPinIndex));
        if (simulator_->findWireByPins(newSrc, newDst) || newDst->isConnectedInput()) {
            continue;
        }

        Wire* createdWire = simulator_->createWire(newSrc, newDst);
        if (!createdWire) {
            continue;
        }

        WireSnapshot wireSnapshot{};
        wireSnapshot.sourceGateId = newSrcGate->getId();
        wireSnapshot.destGateId = newDstGate->getId();
        wireSnapshot.sourcePinIndex = srcPinIndex;
        wireSnapshot.destPinIndex = dstPinIndex;
        createdWireSnapshots_.push_back(wireSnapshot);
    }
}

void DuplicateSelectionCommand::undo() {
    if (!simulator_) {
        return;
    }

    for (const std::string& id : createdGateIds_) {
        LogicGate* gate = simulator_->findGateById(id);
        if (gate) {
            simulator_->removeGate(gate);
        }
    }
}
