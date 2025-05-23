#ifndef GATE_RENDERER_H
#define GATE_RENDERER_H

#include "core/LogicGate.h"
#include "ui/PaletteManager.h"
#include <raylib.h>
#include <vector>
#include <memory>

/**
 * Renders logic gates with improved visual appearance
 */
class GateRenderer {
public:
    GateRenderer();
    void renderGates(const std::vector<std::unique_ptr<LogicGate>>& gates, const Camera2D& camera);
    void renderGate(const LogicGate* gate);
    void renderPinHighlights(const LogicGate* gate, Vector2 mousePos, bool isDrawingWire, const GatePin* wireStartPin);
    void renderWirePreview(const GatePin* startPin,
                          const std::vector<std::unique_ptr<LogicGate>>& gates,
                          Vector2 mousePos);

private:
    GateType determineGateType(const LogicGate* gate) const;
    void renderGateBody(const LogicGate* gate, GateType type) const;
    void renderGatePins(const LogicGate* gate) const;

    // Enhanced rendering methods
    void renderEnhancedGateSymbol(Rectangle bounds, Color fillColor, Color fillColorLight,
                                 Color outlineColor, float outlineThickness, float pulseValue,
                                 bool isSelected, const char* gateType) const;

    // Original gate symbol methods
    void renderAndGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;
    void renderOrGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;
    void renderXorGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;
    void renderNotGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;

    // New triangular gate methods
    void renderTriangularOrGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;
    void renderTriangularXorGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;
    void renderTriangularOrGateShape(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;
    void renderTriangularXorGateShape(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;


    void renderPin(const GatePin* pin, bool showLabel) const;
};

#endif // GATE_RENDERER_H
