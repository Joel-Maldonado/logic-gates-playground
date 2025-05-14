#ifndef GATE_PIN_H
#define GATE_PIN_H

#include "raylib.h"
#include <vector>

// Forward declarations
class LogicGate;
class Wire;

/**
 * Defines the type of a pin (input or output)
 */
enum class PinType {
    INPUT_PIN,
    OUTPUT_PIN
};

/**
 * Represents a connection point on a logic gate
 * Each pin can be either an input or output and manages its connections
 */
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
    /**
     * Constructs a new GatePin
     * 
     * @param parent The LogicGate this pin belongs to
     * @param pinType The type of pin (input or output)
     * @param id Unique identifier for this pin within its parent gate
     * @param offset Position offset relative to the parent gate's position
     */
    GatePin(LogicGate* parent, PinType pinType, int id, Vector2 offset);

    // Getters
    LogicGate* getParentGate() const;
    PinType getType() const;
    int getId() const;
    Vector2 getRelativeOffset() const;
    bool isHovered() const;
    bool isSelected() const;
    float getClickRadius() const;
    const GatePin* getSourceOutputPin() const;
    const std::vector<GatePin*>& getDependentInputPins() const;
    
    // State management
    bool getState() const;
    void setState(bool newState);

    // UI state mutators
    void setHovered(bool hovered);
    void setSelected(bool selected);

    // Connection management
    void connectTo(GatePin* outputPin);
    void disconnectSource();
    void addDependentPin(GatePin* inputPin);
    void removeDependentPin(GatePin* inputPin);
    void disconnectWire(Wire* wireToDisconnect);

    // Utility methods
    Vector2 getAbsolutePosition() const;
    bool isConnectedInput() const;
    bool hasConnectedOutputDependents() const;
    bool isConnected() const;
    bool isMouseOverPin(Vector2 mousePos) const;
};

#endif // GATE_PIN_H
