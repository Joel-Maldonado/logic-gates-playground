#ifndef GATE_RENDERER_H
#define GATE_RENDERER_H

#include "core/LogicGate.h"
#include "ui/PaletteManager.h"
#include "raylib.h"
#include <vector>
#include <memory>

/**
 * Renders logic gates with improved visual appearance
 */
class GateRenderer {
public:
    /**
     * Constructs a new GateRenderer
     */
    GateRenderer();

    /**
     * Renders a collection of gates
     *
     * @param gates Collection of gates to render
     * @param camera Camera for world-to-screen conversion
     */
    void renderGates(const std::vector<std::unique_ptr<LogicGate>>& gates, const Camera2D& camera);

    /**
     * Renders a single gate
     *
     * @param gate Gate to render
     */
    void renderGate(const LogicGate* gate);

    /**
     * Renders pin highlights for a gate
     *
     * @param gate Gate to render pin highlights for
     * @param mousePos Current mouse position
     * @param isDrawingWire Whether a wire is being drawn
     * @param wireStartPin Starting pin of the wire being drawn
     */
    void renderPinHighlights(const LogicGate* gate, Vector2 mousePos, bool isDrawingWire, const GatePin* wireStartPin);

    /**
     * Renders a wire preview
     *
     * @param startPin Starting pin of the wire
     * @param endPos End position of the wire
     * @param gates Collection of gates to check for potential connections
     * @param mousePos Current mouse position
     */
    void renderWirePreview(const GatePin* startPin, Vector2 endPos,
                          const std::vector<std::unique_ptr<LogicGate>>& gates,
                          Vector2 mousePos);

private:
    /**
     * Determines the gate type from a LogicGate pointer
     *
     * @param gate Gate to determine type for
     * @return The gate type
     */
    GateType determineGateType(const LogicGate* gate) const;

    /**
     * Renders the body of a gate based on its type
     *
     * @param gate Gate to render
     * @param type Type of the gate
     */
    void renderGateBody(const LogicGate* gate, GateType type) const;

    /**
     * Renders the pins of a gate with state indicators
     *
     * @param gate Gate to render pins for
     */
    void renderGatePins(const LogicGate* gate) const;

    /**
     * Renders an AND gate symbol
     *
     * @param bounds Bounds of the gate
     * @param fillColor Fill color
     * @param outlineColor Outline color
     * @param outlineThickness Outline thickness
     */
    void renderAndGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;

    /**
     * Renders an OR gate symbol
     *
     * @param bounds Bounds of the gate
     * @param fillColor Fill color
     * @param outlineColor Outline color
     * @param outlineThickness Outline thickness
     */
    void renderOrGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;

    /**
     * Renders an XOR gate symbol
     *
     * @param bounds Bounds of the gate
     * @param fillColor Fill color
     * @param outlineColor Outline color
     * @param outlineThickness Outline thickness
     */
    void renderXorGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;

    /**
     * Renders a NOT gate symbol
     *
     * @param bounds Bounds of the gate
     * @param fillColor Fill color
     * @param outlineColor Outline color
     * @param outlineThickness Outline thickness
     */
    void renderNotGateSymbol(Rectangle bounds, Color fillColor, Color outlineColor, float outlineThickness) const;

    /**
     * Renders a pin with state indicator
     *
     * @param pin Pin to render
     * @param showLabel Whether to show the pin state label
     */
    void renderPin(const GatePin* pin, bool showLabel) const;
};

#endif // GATE_RENDERER_H
