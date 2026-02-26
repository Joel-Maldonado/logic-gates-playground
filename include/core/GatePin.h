#ifndef GATE_PIN_H
#define GATE_PIN_H

#include <raylib.h>
#include <vector>

class LogicGate;
class Wire;

/** Enumeration for pin types */
enum class PinType {
    INPUT_PIN,
    OUTPUT_PIN
};

/**
 * Represents a connection point on a logic gate.
 * Handles input/output connections and state management.
 */
class GatePin {
private:
    LogicGate* parentGate_;
    PinType type_;
    int pinId_;
    Vector2 relativeOffset_;
    float clickRadius_;
    GatePin* sourceOutputPin_;
    std::vector<GatePin*> dependentInputPins_;
    bool currentState_;

public:
    /**
     * Constructs a gate pin.
     * @param parent The parent logic gate
     * @param pinType Type of pin (input or output)
     * @param id Unique identifier within the gate
     * @param offset Position relative to gate origin
     */
    GatePin(LogicGate* parent, PinType pinType, int id, Vector2 offset);

    // Property getters
    LogicGate* getParentGate() const;
    PinType getType() const;
    int getId() const;
    Vector2 getRelativeOffset() const;
    float getClickRadius() const;
    const GatePin* getSourceOutputPin() const;
    const std::vector<GatePin*>& getDependentInputPins() const;

    // State management
    bool getState() const;
    bool setState(bool newState);

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
