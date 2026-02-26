#include "ui/UIManager.h"

#include "ui/InteractionHelpers.h"
#include "ui/InteractionController.h"

#include <raylib.h>

UIManager::UIManager(std::shared_ptr<CircuitSimulator> simulator)
    : simulator_(std::move(simulator)),
      tokens_(CreateDesignTokens()),
      camera_({0}),
      leftPanelBounds_({0}),
      rightPanelBounds_({0}),
      topBarBounds_({0}),
      bottomBarBounds_({0}),
      canvasBounds_({0}),
      paletteManager_(std::make_unique<PaletteManager>(simulator_)),
      interactionController_(nullptr),
      hoveredGate_(nullptr),
      hoveredWire_(nullptr),
      gridSnapEnabled_(true),
      gridVisible_(true),
      debugOverlayEnabled_(false),
      interactionModeLabel_("Idle"),
      statusText_("Ready") {
    camera_.target = {0.0f, 0.0f};
    camera_.offset = {0.0f, 0.0f};
    camera_.rotation = 0.0f;
    camera_.zoom = 1.0f;
}

UIManager::~UIManager() {
    UnloadDesignFonts(tokens_);
}

void UIManager::initialize() {
    LoadDesignFonts(tokens_, "assets");
    updateLayout();
    paletteManager_->initialize();
    interactionController_ = std::make_unique<InteractionController>(simulator_, this);
}

void UIManager::processInput() {
    if (interactionController_) {
        interactionController_->processInput();
    }
}

void UIManager::render() {
    paletteDragPreview_ = buildPaletteDragPreviewState();

    BeginDrawing();
    ClearBackground(tokens_.colors.appBackground);

    DrawRectangleRec(canvasBounds_, tokens_.colors.canvasBackground);

    BeginScissorMode(static_cast<int>(canvasBounds_.x),
                     static_cast<int>(canvasBounds_.y),
                     static_cast<int>(canvasBounds_.width),
                     static_cast<int>(canvasBounds_.height));

    BeginMode2D(camera_);
    sceneRenderer_.renderScene(*simulator_,
                               camera_,
                               canvasBounds_,
                               selection_,
                               hoveredGate_,
                               hoveredWire_,
                               paletteDragPreview_,
                               wirePreview_,
                               marquee_,
                               tokens_,
                               gridVisible_);
    EndMode2D();

    EndScissorMode();

    renderPanels();
    renderBottomBar();

    if (debugOverlayEnabled_) {
        renderDebugOverlay();
    }

    commandPalette_.render(GetScreenWidth(), GetScreenHeight(), tokens_);

    EndDrawing();
}

void UIManager::handleWindowResize(int, int) {
    updateLayout();
    paletteManager_->handleWindowResize();
}

Camera2D& UIManager::getCamera() {
    return camera_;
}

const Camera2D& UIManager::getCamera() const {
    return camera_;
}

Rectangle UIManager::getCanvasBounds() const {
    return canvasBounds_;
}

bool UIManager::isPointInCanvas(Vector2 screenPoint) const {
    return CheckCollisionPointRec(screenPoint, canvasBounds_);
}

PaletteManager& UIManager::getPaletteManager() {
    return *paletteManager_;
}

const PaletteManager& UIManager::getPaletteManager() const {
    return *paletteManager_;
}

CircuitSimulator& UIManager::getSimulator() {
    return *simulator_;
}

const CircuitSimulator& UIManager::getSimulator() const {
    return *simulator_;
}

std::shared_ptr<CircuitSimulator> UIManager::getSimulatorShared() const {
    return simulator_;
}

CommandStack& UIManager::getCommandStack() {
    return commandStack_;
}

EditorSelection& UIManager::getSelection() {
    return selection_;
}

const EditorSelection& UIManager::getSelection() const {
    return selection_;
}

CommandPalette& UIManager::getCommandPalette() {
    return commandPalette_;
}

const DesignTokens& UIManager::getTokens() const {
    return tokens_;
}

void UIManager::setHovered(LogicGate* gate, Wire* wire) {
    hoveredGate_ = gate;
    hoveredWire_ = wire;
}

LogicGate* UIManager::getHoveredGate() const {
    return hoveredGate_;
}

Wire* UIManager::getHoveredWire() const {
    return hoveredWire_;
}

WirePreviewState& UIManager::getWirePreviewState() {
    return wirePreview_;
}

const WirePreviewState& UIManager::getWirePreviewState() const {
    return wirePreview_;
}

void UIManager::clearWirePreview() {
    wirePreview_ = WirePreviewState{};
}

const PaletteDragPreviewState& UIManager::getPaletteDragPreviewState() const {
    return paletteDragPreview_;
}

PaletteDragPreviewState UIManager::buildPaletteDragPreviewState() const {
    PaletteDragPreviewState preview{};
    if (!paletteManager_ || !paletteManager_->isDraggingGateActive()) {
        return preview;
    }

    const GateType draggedType = paletteManager_->getDraggedGateType();
    if (draggedType == GateType::NONE) {
        return preview;
    }

    preview.active = true;
    preview.kind = PaletteManager::toGateKind(draggedType);
    preview.screenPos = paletteManager_->getCurrentDragPosition();
    preview.inCanvas = isPointInCanvas(preview.screenPos);
    preview.worldRaw = GetScreenToWorld2D(preview.screenPos, camera_);
    preview.worldSnapped = preview.worldRaw;
    preview.snapApplied = false;

    if (preview.inCanvas) {
        const bool alt = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
        if (gridSnapEnabled_ && !alt) {
            preview.worldSnapped = InteractionHelpers::snapToGrid(preview.worldRaw, tokens_.metrics.gridSize);
            preview.snapApplied = true;
        }
    }

    return preview;
}

MarqueeState& UIManager::getMarqueeState() {
    return marquee_;
}

const MarqueeState& UIManager::getMarqueeState() const {
    return marquee_;
}

void UIManager::setInteractionModeLabel(const std::string& label) {
    interactionModeLabel_ = label;
}

const std::string& UIManager::getInteractionModeLabel() const {
    return interactionModeLabel_;
}

void UIManager::setStatusText(const std::string& text) {
    statusText_ = text;
}

const std::string& UIManager::getStatusText() const {
    return statusText_;
}

void UIManager::toggleGridVisibility() {
    gridVisible_ = !gridVisible_;
}

bool UIManager::isGridVisible() const {
    return gridVisible_;
}

void UIManager::toggleGridSnap() {
    gridSnapEnabled_ = !gridSnapEnabled_;
}

bool UIManager::isGridSnapEnabled() const {
    return gridSnapEnabled_;
}

void UIManager::toggleDebugOverlay() {
    debugOverlayEnabled_ = !debugOverlayEnabled_;
}

bool UIManager::isDebugOverlayEnabled() const {
    return debugOverlayEnabled_;
}

void UIManager::updateLayout() {
    const float sw = static_cast<float>(GetScreenWidth());
    const float sh = static_cast<float>(GetScreenHeight());

    topBarBounds_ = {0.0f, 0.0f, sw, tokens_.metrics.topBarHeight};
    bottomBarBounds_ = {0.0f, sh - tokens_.metrics.bottomBarHeight, sw, tokens_.metrics.bottomBarHeight};

    leftPanelBounds_ = {
        0.0f,
        topBarBounds_.height,
        tokens_.metrics.leftPanelWidth,
        sh - topBarBounds_.height - bottomBarBounds_.height
    };

    rightPanelBounds_ = {
        sw - tokens_.metrics.rightPanelWidth,
        topBarBounds_.height,
        tokens_.metrics.rightPanelWidth,
        sh - topBarBounds_.height - bottomBarBounds_.height
    };

    canvasBounds_ = {
        leftPanelBounds_.width,
        topBarBounds_.height,
        sw - leftPanelBounds_.width - rightPanelBounds_.width,
        sh - topBarBounds_.height - bottomBarBounds_.height
    };

    camera_.offset = {
        canvasBounds_.x + canvasBounds_.width * 0.5f,
        canvasBounds_.y + canvasBounds_.height * 0.5f
    };

    paletteManager_->setBounds(leftPanelBounds_);
}

void UIManager::renderPanels() {
    paletteManager_->render(tokens_, canvasBounds_);

    toolbar_.render(topBarBounds_,
                    tokens_,
                    commandStack_.canUndo(),
                    commandStack_.canRedo(),
                    gridVisible_,
                    gridSnapEnabled_,
                    camera_.zoom,
                    interactionModeLabel_.c_str());

    inspectorPanel_.render(rightPanelBounds_, tokens_, *simulator_, selection_);
}

void UIManager::renderBottomBar() const {
    DrawRectangleRec(bottomBarBounds_, tokens_.colors.panelBackground);
    DrawLineEx({bottomBarBounds_.x, bottomBarBounds_.y},
               {bottomBarBounds_.x + bottomBarBounds_.width, bottomBarBounds_.y},
               1.0f,
               tokens_.colors.panelBorder);

    DrawTextEx(tokens_.typography.mono,
               statusText_.c_str(),
               {bottomBarBounds_.x + 10.0f, bottomBarBounds_.y + 7.0f},
               tokens_.typography.smallSize,
               1.0f,
               tokens_.colors.textMuted);

    const char* hint = "LMB select/drag | Shift+Drag marquee | G grid | Ctrl+Z undo | Ctrl+K palette";
    Vector2 size = MeasureTextEx(tokens_.typography.mono, hint, tokens_.typography.smallSize, 1.0f);
    DrawTextEx(tokens_.typography.mono,
               hint,
               {bottomBarBounds_.x + bottomBarBounds_.width - size.x - 10.0f, bottomBarBounds_.y + 7.0f},
               tokens_.typography.smallSize,
               1.0f,
               tokens_.colors.textMuted);
}

void UIManager::renderDebugOverlay() const {
    Rectangle box = {canvasBounds_.x + 14.0f, canvasBounds_.y + 14.0f, 350.0f, 110.0f};
    DrawRectangleRounded(box, 0.12f, 8, Fade(tokens_.colors.panelBackground, 0.95f));
    DrawRectangleRoundedLines(box, 0.12f, 8, 1.0f, tokens_.colors.panelBorder);

    const auto stats = simulator_->getLastStats();

    DrawTextEx(tokens_.typography.mono,
               TextFormat("Passes: %d", stats.passes),
               {box.x + 10.0f, box.y + 10.0f},
               tokens_.typography.smallSize,
               1.0f,
               tokens_.colors.textPrimary);

    DrawTextEx(tokens_.typography.mono,
               TextFormat("Stable: %s", stats.stable ? "yes" : "no"),
               {box.x + 10.0f, box.y + 30.0f},
               tokens_.typography.smallSize,
               1.0f,
               tokens_.colors.textPrimary);

    DrawTextEx(tokens_.typography.mono,
               TextFormat("Oscillating: %s", stats.oscillating ? "yes" : "no"),
               {box.x + 10.0f, box.y + 50.0f},
               tokens_.typography.smallSize,
               1.0f,
               tokens_.colors.textPrimary);

    DrawTextEx(tokens_.typography.mono,
               TextFormat("Selection: %zu gate(s), %zu wire(s)", selection_.gates.size(), selection_.wires.size()),
               {box.x + 10.0f, box.y + 70.0f},
               tokens_.typography.smallSize,
               1.0f,
               tokens_.colors.textPrimary);

    DrawTextEx(tokens_.typography.mono,
               TextFormat("Mode: %s", interactionModeLabel_.c_str()),
               {box.x + 10.0f, box.y + 90.0f},
               tokens_.typography.smallSize,
               1.0f,
               tokens_.colors.textPrimary);
}
