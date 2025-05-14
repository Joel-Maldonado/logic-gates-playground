#ifndef LOGIC_GATE_H
#define LOGIC_GATE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "raylib.h"
#include "core/GatePin.h"

class Wire;

class LogicGate {
protected:
    std::string id;
    Vector2 position;
    float width;
    float height;
    bool isDirty;
    bool isSelected;

    std::vector<GatePin> inputPins;
    std::vector<GatePin> outputPins;
    std::vector<Wire*> associatedWires;

public:
    LogicGate(std::string gateId, Vector2 pos, float w, float h);
    virtual ~LogicGate();

    virtual void evaluate() = 0;
    virtual void draw() = 0;

    void update();
    void markDirty();
    bool needsEvaluation() const;

protected: // Changed from public
    void initializeInputPin(int pinId, Vector2 relativeOffset);
    void initializeOutputPin(int pinId, Vector2 relativeOffset);
public:   // Back to public for subsequent methods

    GatePin* getInputPin(size_t pinIndex);
    GatePin* getOutputPin(size_t pinIndex);
    const GatePin* getInputPin(size_t pinIndex) const;
    const GatePin* getOutputPin(size_t pinIndex) const;


    size_t getInputPinCount() const;
    size_t getOutputPinCount() const;

    void setInputState(size_t pinIndex, bool state);
    bool getOutputState(size_t pinIndex = 0) const;

    virtual void setPosition(Vector2 newPosition);
    virtual Vector2 getPosition() const;
    virtual Rectangle getBounds() const;

    virtual bool isMouseOver(Vector2 mousePos) const;
    virtual GatePin* getPinAt(Vector2 mousePos, float tolerance = 5.0f);
    void setSelected(bool selected);
    bool getIsSelected() const;

    std::string getId() const;
    float getWidth() const;
    float getHeight() const;

    void addWire(Wire* wire);
    void removeWire(Wire* wire);
    const std::vector<Wire*>& getAssociatedWires() const;

    const std::vector<GatePin>& getAllInputPins() const { return inputPins; }
    const std::vector<GatePin>& getAllOutputPins() const { return outputPins; }

    // Replaced static handleGateDeletion with a member function
    std::vector<Wire*> prepareForDeletion();

protected: // For non-const versions
    std::vector<GatePin>& getAllInputPins() { return inputPins; }
    std::vector<GatePin>& getAllOutputPins() { return outputPins; }

};

#endif // LOGIC_GATE_H