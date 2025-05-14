#ifndef WIRE_RENDERER_H
#define WIRE_RENDERER_H

#include "core/Wire.h"
#include "ui/WireRouter.h"
#include "raylib.h"
#include <vector>
#include <memory>

/**
 * Renders wires
 */
class WireRenderer {
private:
    WireRouter router_;

public:
    /**
     * Constructs a new WireRenderer
     */
    WireRenderer();

    /**
     * Renders a collection of wires
     *
     * @param wires Collection of wires to render
     */
    void renderWires(const std::vector<std::unique_ptr<Wire>>& wires);

    /**
     * Renders a single wire
     *
     * @param wire Wire to render
     */
    void renderWire(const Wire* wire);

    /**
     * Renders a wire preview during wire creation
     *
     * @param startPos Starting position of the wire
     * @param endPos End position of the wire
     * @param isDestInput Whether the destination is an input pin
     * @param color Color of the wire
     * @param thickness Thickness of the wire
     */
    void renderWirePreview(Vector2 startPos, Vector2 endPos, bool isDestInput, Color color, float thickness);

    /**
     * Renders a wire preview with a path
     *
     * @param path Vector of points representing the wire path
     * @param color Color of the wire
     * @param thickness Thickness of the wire
     */
    void renderWirePathPreview(const std::vector<Vector2>& path, Color color, float thickness);

    /**
     * Calculates a preview path for a wire being drawn
     *
     * @param startPos Starting position of the wire
     * @param endPos Current mouse position
     * @param isDestInput Whether the destination is an input pin
     * @return Vector of points representing the wire path
     */
    std::vector<Vector2> calculatePreviewPath(Vector2 startPos, Vector2 endPos, bool isDestInput = false);

    /**
     * Gets the color for a wire based on its state and selection
     *
     * @param wire Wire to get color for
     * @return Color for the wire
     */
    Color getWireColor(const Wire* wire) const;

    /**
     * Gets the thickness for a wire based on its selection
     *
     * @param wire Wire to get thickness for
     * @return Thickness for the wire
     */
    float getWireThickness(const Wire* wire) const;
};

#endif // WIRE_RENDERER_H
