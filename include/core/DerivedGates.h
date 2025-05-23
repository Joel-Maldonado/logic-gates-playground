#ifndef DERIVED_GATES_H
#define DERIVED_GATES_H

#include "core/LogicGate.h"

class AndGate : public LogicGate {
public:
    AndGate(std::string gateId, Vector2 pos, float w = 60.0f, float h = 40.0f);
    void evaluate() override;
    void draw() override;
};

class OrGate : public LogicGate {
public:
    OrGate(std::string gateId, Vector2 pos, float w = 60.0f, float h = 40.0f);
    void evaluate() override;
    void draw() override;
};

class XorGate : public LogicGate {
public:
    XorGate(std::string gateId, Vector2 pos, float w = 60.0f, float h = 40.0f);
    void evaluate() override;
    void draw() override;
};

class NotGate : public LogicGate {
public:
    NotGate(std::string gateId, Vector2 pos, float w = 60.0f, float h = 40.0f);
    void evaluate() override;
    void draw() override;
};



#endif // DERIVED_GATES_H