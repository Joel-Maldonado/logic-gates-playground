#include "ui/GateFactory.h"

#include "app/Config.h"
#include "core/DerivedGates.h"
#include "core/InputSource.h"
#include "core/OutputSink.h"

#include <utility>

namespace GateFactory {

Vector2 defaultSizeFor(GateKind kind) {
    switch (kind) {
        case GateKind::INPUT_SOURCE:
            return {Config::INPUT_OUTPUT_SIZE, Config::INPUT_OUTPUT_SIZE};
        case GateKind::OUTPUT_SINK:
            return {Config::INPUT_OUTPUT_SIZE, Config::INPUT_OUTPUT_SIZE};
        case GateKind::AND_GATE:
        case GateKind::OR_GATE:
        case GateKind::XOR_GATE:
        case GateKind::NOT_GATE:
            return {Config::DEFAULT_GATE_WIDTH, Config::DEFAULT_GATE_HEIGHT};
    }

    return {Config::DEFAULT_GATE_WIDTH, Config::DEFAULT_GATE_HEIGHT};
}

std::unique_ptr<LogicGate> createGate(GateKind kind,
                                      const std::string& gateId,
                                      Vector2 position,
                                      Vector2 size) {
    Vector2 resolved = size;
    if (resolved.x <= 0.0f || resolved.y <= 0.0f) {
        resolved = defaultSizeFor(kind);
    }

    switch (kind) {
        case GateKind::INPUT_SOURCE:
            return std::make_unique<InputSource>(gateId, position, resolved, "IN");

        case GateKind::OUTPUT_SINK:
            return std::make_unique<OutputSink>(gateId, position, resolved.x * 0.5f, "OUT");

        case GateKind::AND_GATE:
            return std::make_unique<AndGate>(gateId, position, resolved.x, resolved.y);

        case GateKind::OR_GATE:
            return std::make_unique<OrGate>(gateId, position, resolved.x, resolved.y);

        case GateKind::XOR_GATE:
            return std::make_unique<XorGate>(gateId, position, resolved.x, resolved.y);

        case GateKind::NOT_GATE:
            return std::make_unique<NotGate>(gateId, position, resolved.x, resolved.y);
    }

    return nullptr;
}

} // namespace GateFactory
