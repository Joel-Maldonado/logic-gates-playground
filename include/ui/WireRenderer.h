#ifndef WIRE_RENDERER_H
#define WIRE_RENDERER_H

#include "core/Wire.h"
#include "ui/WireRouter.h"
#include <raylib.h>
#include <vector>
#include <memory>

/**
 * Handles rendering of wires and wire previews in the circuit editor.
 * Manages wire appearance, routing, and visual feedback during wire creation.
 */
class WireRenderer {
private:
    WireRouter router_;

public:
    WireRenderer();

    /** Renders all wires in the circuit */
    void renderWires(const std::vector<std::unique_ptr<Wire>>& wires);

    /** Renders a single wire */
    void renderWire(const Wire* wire);

    /** Renders a preview wire during creation */
    void renderWirePreview(Vector2 startPos, Vector2 endPos, bool isDestInput, Color color, float thickness);

    /** Renders a wire path preview with specific control points */
    void renderWirePathPreview(const std::vector<Vector2>& path, Color color, float thickness);

    /** Calculates the preview path for a wire being created */
    std::vector<Vector2> calculatePreviewPath(Vector2 startPos, Vector2 endPos, bool isDestInput = false);

    /** Gets the appropriate color for a wire based on its state */
    Color getWireColor(const Wire* wire) const;

    /** Gets the appropriate thickness for a wire based on its state */
    float getWireThickness(const Wire* wire) const;
};

#endif // WIRE_RENDERER_H
