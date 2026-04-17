#ifndef UI_GATE_PAINTER_H
#define UI_GATE_PAINTER_H

#include "core/LogicGate.h"
#include "ui/DesignTokens.h"

class GatePainter {
public:
    void renderGate(const LogicGate* gate, bool selected, bool hovered, const DesignTokens& tokens) const;
    void renderGhostGate(GateKind kind, Vector2 worldPos, bool snapped, const DesignTokens& tokens) const;
};

#endif // UI_GATE_PAINTER_H
