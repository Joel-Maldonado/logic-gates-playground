#ifndef UI_WIRE_PAINTER_H
#define UI_WIRE_PAINTER_H

#include "core/Wire.h"
#include "ui/DesignTokens.h"
#include "ui/EditorSelection.h"

#include <memory>
#include <vector>

class WirePainter {
public:
    void renderWires(const std::vector<std::unique_ptr<Wire>>& wires,
                     const EditorSelection& selection,
                     const Wire* hoveredWire,
                     const DesignTokens& tokens) const;

    void renderWirePreview(const std::vector<Vector2>& previewPath,
                           bool validTarget,
                           const DesignTokens& tokens) const;
};

#endif // UI_WIRE_PAINTER_H
