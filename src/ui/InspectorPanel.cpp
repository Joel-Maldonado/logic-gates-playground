#include "ui/InspectorPanel.h"

#include <raylib.h>
#include <string>

namespace {

const char* gateKindName(GateKind kind) {
    switch (kind) {
        case GateKind::INPUT_SOURCE: return "Input Source";
        case GateKind::OUTPUT_SINK: return "Output Sink";
        case GateKind::AND_GATE: return "AND";
        case GateKind::OR_GATE: return "OR";
        case GateKind::XOR_GATE: return "XOR";
        case GateKind::NOT_GATE: return "NOT";
    }

    return "Unknown";
}

} // namespace

void InspectorPanel::render(const Rectangle& bounds,
                            const DesignTokens& tokens,
                            const CircuitSimulator& simulator,
                            const EditorSelection& selection) const {
    (void)simulator;

    DrawRectangleRec(bounds, tokens.colors.panelBackground);
    DrawLineEx({bounds.x, bounds.y}, {bounds.x, bounds.y + bounds.height}, 1.0f, tokens.colors.panelBorder);

    float cursorY = bounds.y + 10.0f;
    const float x = bounds.x + 12.0f;

    DrawTextEx(tokens.typography.ui,
               "Inspector",
               {x, cursorY},
               tokens.typography.bodySize,
               1.0f,
               tokens.colors.textPrimary);
    cursorY += 28.0f;

    char countLine[128];
    snprintf(countLine,
             sizeof(countLine),
             "Selected: %zu gate(s), %zu wire(s)",
             selection.gates.size(),
             selection.wires.size());
    DrawTextEx(tokens.typography.mono,
               countLine,
               {x, cursorY},
               tokens.typography.smallSize,
               1.0f,
               tokens.colors.textMuted);
    cursorY += 24.0f;

    if (selection.gates.size() == 1 && selection.wires.empty()) {
        const LogicGate* gate = selection.gates.front();
        DrawTextEx(tokens.typography.mono,
                   TextFormat("Type: %s", gateKindName(gate->getKind())),
                   {x, cursorY},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textPrimary);
        cursorY += 20.0f;

        DrawTextEx(tokens.typography.mono,
                   TextFormat("ID: %s", gate->getId().c_str()),
                   {x, cursorY},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textPrimary);
        cursorY += 20.0f;

        Vector2 pos = gate->getPosition();
        DrawTextEx(tokens.typography.mono,
                   TextFormat("Pos: (%.0f, %.0f)", pos.x, pos.y),
                   {x, cursorY},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textPrimary);
        cursorY += 20.0f;

        DrawTextEx(tokens.typography.mono,
                   TextFormat("Size: %.0f x %.0f", gate->getWidth(), gate->getHeight()),
                   {x, cursorY},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textPrimary);
        cursorY += 20.0f;

        for (size_t i = 0; i < gate->getInputPinCount(); ++i) {
            DrawTextEx(tokens.typography.mono,
                       TextFormat("IN%zu: %d", i, gate->getInputPin(i)->getState() ? 1 : 0),
                       {x, cursorY},
                       tokens.typography.smallSize,
                       1.0f,
                       tokens.colors.textMuted);
            cursorY += 18.0f;
        }
        for (size_t i = 0; i < gate->getOutputPinCount(); ++i) {
            DrawTextEx(tokens.typography.mono,
                       TextFormat("OUT%zu: %d", i, gate->getOutputPin(i)->getState() ? 1 : 0),
                       {x, cursorY},
                       tokens.typography.smallSize,
                       1.0f,
                       tokens.colors.textMuted);
            cursorY += 18.0f;
        }
    } else if (selection.wires.size() == 1 && selection.gates.empty()) {
        const Wire* wire = selection.wires.front();
        const GatePin* src = wire->getSourcePin();
        const GatePin* dst = wire->getDestPin();

        const std::string srcId = (src && src->getParentGate()) ? src->getParentGate()->getId() : "?";
        const std::string dstId = (dst && dst->getParentGate()) ? dst->getParentGate()->getId() : "?";

        DrawTextEx(tokens.typography.mono,
                   TextFormat("Wire: %s -> %s", srcId.c_str(), dstId.c_str()),
                   {x, cursorY},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textPrimary);
        cursorY += 20.0f;

        DrawTextEx(tokens.typography.mono,
                   TextFormat("Signal: %d", wire->getState() ? 1 : 0),
                   {x, cursorY},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textPrimary);
    } else if (selection.empty()) {
        DrawTextEx(tokens.typography.mono,
                   "No selection",
                   {x, cursorY},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textMuted);
    } else {
        DrawTextEx(tokens.typography.mono,
                   "Multi-selection",
                   {x, cursorY},
                   tokens.typography.smallSize,
                   1.0f,
                   tokens.colors.textMuted);
    }
}
