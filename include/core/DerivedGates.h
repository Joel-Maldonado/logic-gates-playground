#ifndef DERIVED_GATES_H
#define DERIVED_GATES_H

#include "core/LogicGate.h"

/**
 * AND logic gate implementation.
 * Output is true only when all inputs are true.
 */
class AndGate : public LogicGate {
public:
    AndGate(std::string gateId, Vector2 pos, float w = 60.0f, float h = 40.0f);
    void evaluate() override;
    void draw() override;
};

/**
 * OR logic gate implementation.
 * Output is true when any input is true.
 */
class OrGate : public LogicGate {
public:
    OrGate(std::string gateId, Vector2 pos, float w = 60.0f, float h = 40.0f);
    void evaluate() override;
    void draw() override;
};

/**
 * XOR (exclusive OR) logic gate implementation.
 * Output is true when inputs are different.
 */
class XorGate : public LogicGate {
public:
    XorGate(std::string gateId, Vector2 pos, float w = 60.0f, float h = 40.0f);
    void evaluate() override;
    void draw() override;
};

/**
 * NOT logic gate implementation.
 * Output is the inverse of the input.
 */
class NotGate : public LogicGate {
public:
    NotGate(std::string gateId, Vector2 pos, float w = 60.0f, float h = 40.0f);
    void evaluate() override;
    void draw() override;
};



#endif // DERIVED_GATES_H