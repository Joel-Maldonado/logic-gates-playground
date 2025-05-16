#ifndef GATE_PIN_H
#define GATE_PIN_H

#include "raylib.h"
#include <vector>

class LogicGate;
class Wire;

enum class PinType {
    INPUT_PIN,
    OUTPUT_PIN
};

class GatePin {
private:
    LogicGate* parentGate_;
    PinType type_;
    int pinId_;
    Vector2 relativeOffset_;
    bool isHovered_;
    bool isSelected_;
    float clickRadius_;
    GatePin* sourceOutputPin_;
    std::vector<GatePin*> dependentInputPins_;
    bool currentState_;

public:
    GatePin(LogicGate* parent, PinType pinType, int id, Vector2 offset);

    LogicGate* getParentGate() const;
    PinType getType() const;
    int getId() const;
    Vector2 getRelativeOffset() const;
    bool isHovered() const;
    bool isSelected() const;
    float getClickRadius() const;
    const GatePin* getSourceOutputPin() const;
    const std::vector<GatePin*>& getDependentInputPins() const;

    bool getState() const;
    void setState(bool newState);

    void setHovered(bool hovered);
    void setSelected(bool selected);

    void connectTo(GatePin* outputPin);
    void disconnectSource();
    void addDependentPin(GatePin* inputPin);
    void removeDependentPin(GatePin* inputPin);
    void disconnectWire(Wire* wireToDisconnect);

    Vector2 getAbsolutePosition() const;
    bool isConnectedInput() const;
    bool hasConnectedOutputDependents() const;
    bool isConnected() const;
    bool isMouseOverPin(Vector2 mousePos) const;
};

#endif // GATE_PIN_H
