#include "ui/SceneRenderer.h"

#include "ui/GateGeometry.h"

#include <algorithm>
#include <cmath>

namespace {

Rectangle normalizeRect(Rectangle rect) {
    if (rect.width < 0.0f) {
        rect.x += rect.width;
        rect.width = -rect.width;
    }
    if (rect.height < 0.0f) {
        rect.y += rect.height;
        rect.height = -rect.height;
    }
    return rect;
}

} // namespace

void SceneRenderer::renderScene(const CircuitSimulator& simulator,
                                const Camera2D& camera,
                                const Rectangle& canvasWorldRect,
                                const EditorSelection& selection,
                                const LogicGate* hoveredGate,
                                const Wire* hoveredWire,
                                const PaletteDragPreviewState& palettePreview,
                                const WirePreviewState& wirePreview,
                                const MarqueeState& marquee,
                                const DesignTokens& tokens,
                                bool gridEnabled) const {
    if (gridEnabled) {
        renderGrid(camera, canvasWorldRect, tokens);
    }

    wirePainter_.renderWires(simulator.getWires(), selection, hoveredWire, tokens);

    if (wirePreview.active) {
        const std::vector<Vector2> path = wireRouter_.calculatePreviewPath(
            wirePreview.start,
            wirePreview.end,
            wirePreview.validTarget);
        wirePainter_.renderWirePreview(path, wirePreview.validTarget, tokens);
    }

    if (palettePreview.active && palettePreview.inCanvas) {
        const Vector2 pos = palettePreview.snapApplied ? palettePreview.worldSnapped : palettePreview.worldRaw;
        gatePainter_.renderGhostGate(palettePreview.kind, pos, palettePreview.snapApplied, tokens);
    }

    for (const auto& gate : simulator.getGates()) {
        const bool selected = selection.containsGate(gate.get());
        const bool hovered = (hoveredGate == gate.get());
        gatePainter_.renderGate(gate.get(), selected, hovered, tokens);
    }

    if (marquee.active) {
        Rectangle rect = normalizeRect(marquee.rect);
        DrawRectangleRec(rect, Fade(tokens.colors.accentPrimary, 0.15f));
        DrawRectangleLinesEx(rect, 1.5f, tokens.colors.accentPrimary);
    }
}

void SceneRenderer::renderGrid(const Camera2D& camera,
                               const Rectangle& canvasWorldRect,
                               const DesignTokens& tokens) const {
    Vector2 topLeft = GetScreenToWorld2D({canvasWorldRect.x, canvasWorldRect.y}, camera);
    Vector2 bottomRight = GetScreenToWorld2D(
        {canvasWorldRect.x + canvasWorldRect.width, canvasWorldRect.y + canvasWorldRect.height},
        camera);

    if (bottomRight.x < topLeft.x) {
        std::swap(topLeft.x, bottomRight.x);
    }
    if (bottomRight.y < topLeft.y) {
        std::swap(topLeft.y, bottomRight.y);
    }

    const float baseGrid = tokens.metrics.gridSize;
    const float majorGrid = baseGrid * 4.0f;

    const int startMajorX = static_cast<int>(floorf(topLeft.x / majorGrid) * majorGrid);
    const int endMajorX = static_cast<int>(ceilf(bottomRight.x / majorGrid) * majorGrid);
    const int startMajorY = static_cast<int>(floorf(topLeft.y / majorGrid) * majorGrid);
    const int endMajorY = static_cast<int>(ceilf(bottomRight.y / majorGrid) * majorGrid);

    for (int x = startMajorX; x <= endMajorX; x += static_cast<int>(majorGrid)) {
        DrawLineEx({static_cast<float>(x), topLeft.y},
                   {static_cast<float>(x), bottomRight.y},
                   1.0f,
                   tokens.colors.gridMajor);
    }
    for (int y = startMajorY; y <= endMajorY; y += static_cast<int>(majorGrid)) {
        DrawLineEx({topLeft.x, static_cast<float>(y)},
                   {bottomRight.x, static_cast<float>(y)},
                   1.0f,
                   tokens.colors.gridMajor);
    }

    const float z = camera.zoom;
    const float density = (z < 0.6f) ? 3.0f : ((z < 0.9f) ? 2.0f : 1.0f);
    const float step = baseGrid * density;

    const int startX = static_cast<int>(floorf(topLeft.x / step) * step);
    const int endX = static_cast<int>(ceilf(bottomRight.x / step) * step);
    const int startY = static_cast<int>(floorf(topLeft.y / step) * step);
    const int endY = static_cast<int>(ceilf(bottomRight.y / step) * step);

    const float dotRadius = std::clamp(1.1f / z, 0.4f, 1.2f);
    for (int x = startX; x <= endX; x += static_cast<int>(step)) {
        for (int y = startY; y <= endY; y += static_cast<int>(step)) {
            DrawCircleV({static_cast<float>(x), static_cast<float>(y)}, dotRadius, tokens.colors.gridMinor);
        }
    }
}
