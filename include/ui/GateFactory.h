#ifndef UI_GATE_FACTORY_H
#define UI_GATE_FACTORY_H

#include "core/LogicGate.h"

#include <memory>
#include <string>

namespace GateFactory {

std::unique_ptr<LogicGate> createGate(GateKind kind,
                                      const std::string& gateId,
                                      Vector2 position,
                                      Vector2 size = {0.0f, 0.0f});

Vector2 defaultSizeFor(GateKind kind);

} // namespace GateFactory

#endif // UI_GATE_FACTORY_H
