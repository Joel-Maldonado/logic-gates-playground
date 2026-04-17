#ifndef LOGIC_GATE_H
#define LOGIC_GATE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <raylib.h>
#include "core/GatePin.h"

class Wire;

enum class GateKind {
    INPUT_SOURCE,
    OUTPUT_SINK,
    AND_GATE,
    OR_GATE,
    XOR_GATE,
    NOT_GATE
};

/**
 * Abstract base class for all logic gates in the circuit simulator.
 * Provides common functionality for gate positioning, pin management,
 * and state evaluation.
 */
class LogicGate {
protected:
    std::string id_;
    GateKind gateKind_;
    Vector2 position_;
    float width_;
    float height_;
    bool isDirty_;

    std::vector<GatePin> inputPins_;
    std::vector<GatePin> outputPins_;
    std::vector<Wire*> associatedWires_;

public:
    /**
     * Constructs a logic gate with the specified parameters.
     * @param gateId Unique identifier for this gate
     * @param pos Position of the gate in world coordinates
     * @param w Width of the gate
     * @param h Height of the gate
     */
    LogicGate(std::string gateId, GateKind kind, Vector2 pos, float w, float h);
    virtual ~LogicGate();

    /**
     * Evaluates the gate's logic based on input pin states.
     * Must be implemented by derived classes.
     */
    virtual void evaluate() = 0;

    /** Updates the gate state if marked as dirty; returns true when output pins changed */
    bool update();

    /** Marks the gate for re-evaluation on next update */
    void markDirty();

    /** Returns true if the gate needs evaluation */
    bool needsEvaluation() const;

protected:
    /** Initializes an input pin at the specified relative position */
    void initializeInputPin(int pinId, Vector2 relativeOffset);

    /** Initializes an output pin at the specified relative position */
    void initializeOutputPin(int pinId, Vector2 relativeOffset);

public:
    // Pin access methods
    GatePin* getInputPin(size_t pinIndex);
    GatePin* getOutputPin(size_t pinIndex);
    const GatePin* getInputPin(size_t pinIndex) const;
    const GatePin* getOutputPin(size_t pinIndex) const;

    /** Returns the number of input pins */
    size_t getInputPinCount() const;

    /** Returns the number of output pins */
    size_t getOutputPinCount() const;

    /** Sets the state of an input pin */
    void setInputState(size_t pinIndex, bool state);

    /** Gets the state of an output pin */
    bool getOutputState(size_t pinIndex = 0) const;

    // Position and bounds methods
    virtual void setPosition(Vector2 newPosition);
    virtual Vector2 getPosition() const;
    virtual Rectangle getBounds() const;

    // Mouse interaction methods
    virtual bool isMouseOver(Vector2 mousePos) const;
    virtual GatePin* getPinAt(Vector2 mousePos, float tolerance = 5.0f);

    // Property getters
    std::string getId() const;
    GateKind getKind() const;
    float getWidth() const;
    float getHeight() const;

    // Wire management
    void addWire(Wire* wire);
    void removeWire(Wire* wire);
    const std::vector<Wire*>& getAssociatedWires() const;

    /** Returns all input pins (const version) */
    const std::vector<GatePin>& getAllInputPins() const { return inputPins_; }

    /** Returns all output pins (const version) */
    const std::vector<GatePin>& getAllOutputPins() const { return outputPins_; }

    /** Prepares gate for deletion by disconnecting all wires */
    std::vector<Wire*> prepareForDeletion();

protected:
    /** Returns all input pins (mutable version) */
    std::vector<GatePin>& getAllInputPins() { return inputPins_; }

    /** Returns all output pins (mutable version) */
    std::vector<GatePin>& getAllOutputPins() { return outputPins_; }
};

#endif // LOGIC_GATE_H
