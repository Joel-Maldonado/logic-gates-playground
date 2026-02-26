#ifndef UI_SCENE_RENDERER_H
#define UI_SCENE_RENDERER_H

#include "simulation/CircuitSimulator.h"
#include "ui/DesignTokens.h"
#include "ui/EditorSelection.h"
#include "ui/GatePainter.h"
#include "ui/WirePainter.h"
#include "ui/WireRouter.h"

#include <raylib.h>

struct WirePreviewState {
    bool active = false;
    Vector2 start{0.0f, 0.0f};
    Vector2 end{0.0f, 0.0f};
    bool validTarget = false;
};

struct PaletteDragPreviewState {
    bool active = false;
    GateKind kind = GateKind::AND_GATE;
    Vector2 screenPos{0.0f, 0.0f};
    Vector2 worldRaw{0.0f, 0.0f};
    Vector2 worldSnapped{0.0f, 0.0f};
    bool inCanvas = false;
    bool snapApplied = false;
};

struct MarqueeState {
    bool active = false;
    Rectangle rect{0.0f, 0.0f, 0.0f, 0.0f};
};

class SceneRenderer {
public:
    void renderScene(const CircuitSimulator& simulator,
                     const Camera2D& camera,
                     const Rectangle& canvasWorldRect,
                     const EditorSelection& selection,
                     const LogicGate* hoveredGate,
                     const Wire* hoveredWire,
                     const PaletteDragPreviewState& palettePreview,
                     const WirePreviewState& wirePreview,
                     const MarqueeState& marquee,
                     const DesignTokens& tokens,
                     bool gridEnabled) const;

private:
    void renderGrid(const Camera2D& camera,
                    const Rectangle& canvasWorldRect,
                    const DesignTokens& tokens) const;

    GatePainter gatePainter_;
    WirePainter wirePainter_;
    mutable WireRouter wireRouter_;
};

#endif // UI_SCENE_RENDERER_H
