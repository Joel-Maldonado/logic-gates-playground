#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "simulation/CircuitSimulator.h"
#include "ui/CommandPalette.h"
#include "ui/CommandStack.h"
#include "ui/DesignTokens.h"
#include "ui/EditorSelection.h"
#include "ui/InspectorPanel.h"
#include "ui/PaletteManager.h"
#include "ui/SceneRenderer.h"
#include "ui/Toolbar.h"

#include <memory>
#include <string>

class InteractionController;

class UIManager {
public:
    explicit UIManager(std::shared_ptr<CircuitSimulator> simulator);
    ~UIManager();

    void initialize();
    void processInput();
    void render();
    void handleWindowResize(int newWidth, int newHeight);

    Camera2D& getCamera();
    const Camera2D& getCamera() const;
    Rectangle getCanvasBounds() const; // screen-space
    bool isPointInCanvas(Vector2 screenPoint) const;

    PaletteManager& getPaletteManager();
    const PaletteManager& getPaletteManager() const;

    CircuitSimulator& getSimulator();
    const CircuitSimulator& getSimulator() const;
    std::shared_ptr<CircuitSimulator> getSimulatorShared() const;

    CommandStack& getCommandStack();
    EditorSelection& getSelection();
    const EditorSelection& getSelection() const;

    CommandPalette& getCommandPalette();

    const DesignTokens& getTokens() const;

    void setHovered(LogicGate* gate, Wire* wire);
    LogicGate* getHoveredGate() const;
    Wire* getHoveredWire() const;

    WirePreviewState& getWirePreviewState();
    const WirePreviewState& getWirePreviewState() const;
    void clearWirePreview();
    const PaletteDragPreviewState& getPaletteDragPreviewState() const;
    PaletteDragPreviewState buildPaletteDragPreviewState() const;

    MarqueeState& getMarqueeState();
    const MarqueeState& getMarqueeState() const;

    void setInteractionModeLabel(const std::string& label);
    const std::string& getInteractionModeLabel() const;

    void setStatusText(const std::string& text);
    const std::string& getStatusText() const;

    void toggleGridVisibility();
    bool isGridVisible() const;

    void toggleGridSnap();
    bool isGridSnapEnabled() const;

    void toggleDebugOverlay();
    bool isDebugOverlayEnabled() const;

private:
    void updateLayout();
    void renderPanels();
    void renderBottomBar() const;
    void renderDebugOverlay() const;

    std::shared_ptr<CircuitSimulator> simulator_;

    DesignTokens tokens_;
    Camera2D camera_;

    Rectangle leftPanelBounds_;
    Rectangle rightPanelBounds_;
    Rectangle topBarBounds_;
    Rectangle bottomBarBounds_;
    Rectangle canvasBounds_;

    std::unique_ptr<PaletteManager> paletteManager_;
    std::unique_ptr<InteractionController> interactionController_;

    SceneRenderer sceneRenderer_;
    Toolbar toolbar_;
    InspectorPanel inspectorPanel_;
    CommandPalette commandPalette_;

    CommandStack commandStack_;
    EditorSelection selection_;

    LogicGate* hoveredGate_;
    Wire* hoveredWire_;
    WirePreviewState wirePreview_;
    PaletteDragPreviewState paletteDragPreview_;
    MarqueeState marquee_;

    bool gridSnapEnabled_;
    bool gridVisible_;
    bool debugOverlayEnabled_;
    std::string interactionModeLabel_;
    std::string statusText_;
};

#endif // UI_MANAGER_H
