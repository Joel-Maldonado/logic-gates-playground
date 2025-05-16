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
    WireRenderer();
    void renderWires(const std::vector<std::unique_ptr<Wire>>& wires);
    void renderWire(const Wire* wire);
    void renderWirePreview(Vector2 startPos, Vector2 endPos, bool isDestInput, Color color, float thickness);
    void renderWirePathPreview(const std::vector<Vector2>& path, Color color, float thickness);
    std::vector<Vector2> calculatePreviewPath(Vector2 startPos, Vector2 endPos, bool isDestInput = false);
    Color getWireColor(const Wire* wire) const;
    float getWireThickness(const Wire* wire) const;
};

#endif // WIRE_RENDERER_H
