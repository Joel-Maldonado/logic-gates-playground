#include "core/OutputSink.h"
#include <utility>

OutputSink::OutputSink(std::string id, Vector2 pos, float visualRadius, std::string visualLabel)
    : LogicGate(std::move(id), GateKind::OUTPUT_SINK, pos, visualRadius * 2, visualRadius * 2),
      active_(false) {
    (void)visualLabel;
    initializeInputPin(0, {0, visualRadius});
    markDirty();
}

OutputSink::~OutputSink() = default;

void OutputSink::evaluate() {
    if (getInputPinCount() > 0) {
        active_ = getInputPin(0)->getState();
    } else {
        active_ = false;
    }
}

bool OutputSink::isActive() const {
    return active_;
}
