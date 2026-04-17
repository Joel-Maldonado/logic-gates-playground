#ifndef UI_INTERACTION_CONTROLLER_H
#define UI_INTERACTION_CONTROLLER_H

#include "simulation/CircuitSimulator.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class GatePin;
class LogicGate;
class UIManager;
class Wire;

class InteractionController {
public:
    InteractionController(std::shared_ptr<CircuitSimulator> simulator, UIManager* uiManager);

    void processInput();

private:
    enum class Mode {
        Idle,
        PaletteDrag,
        WireDraw,
        GatePressPending,
        GateDrag,
        WirePointDrag,
        Pan,
        Marquee
    };

    struct PressCapture {
        LogicGate* gate = nullptr;
        Vector2 worldPos{0.0f, 0.0f};
        bool valid = false;
    };

    void setMode(Mode mode);
    const char* modeName(Mode mode) const;

    bool isPrimaryModifierDown() const;
    bool isShiftDown() const;
    bool isAltDown() const;

    GatePin* findPinUnderMouse(Vector2 worldMousePos) const;
    LogicGate* findGateUnderMouse(Vector2 worldMousePos) const;
    Wire* findWireUnderMouse(Vector2 worldMousePos) const;

    void beginGatePress(LogicGate* gate, Vector2 worldMousePos);
    void updateGateDrag(Vector2 worldMousePos);
    void finishGateDrag();

    void beginMarquee(Vector2 worldMousePos);
    void updateMarquee(Vector2 worldMousePos);
    void finishMarquee();

    void clearPressCapture();

    void selectSingleGate(LogicGate* gate);
    void selectSingleWire(Wire* wire);
    void clearSelection();

    void handleShortcuts();
    void handleCommandPaletteAction(const std::string& actionId);
    void frameSelection();

    std::shared_ptr<CircuitSimulator> simulator_;
    UIManager* uiManager_;

    Mode mode_;

    PressCapture pressCapture_;

    bool isPanning_;
    Vector2 lastPanMousePos_;

    bool dragPending_;
    bool isDraggingGates_;
    Vector2 dragStartMouseWorld_;
    std::vector<std::string> draggedGateIds_;
    std::vector<Vector2> dragFromPositions_;

    bool isDraggingWirePoint_;

    bool marqueeAdditive_;

    GatePin* wireStartPin_;
};

#endif // UI_INTERACTION_CONTROLLER_H
