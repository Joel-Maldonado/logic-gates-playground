#ifndef UI_COMMANDS_ADD_GATE_COMMAND_H
#define UI_COMMANDS_ADD_GATE_COMMAND_H

#include "core/LogicGate.h"
#include "ui/EditorCommand.h"

#include <memory>
#include <string>

class CircuitSimulator;

class AddGateCommand : public EditorCommand {
public:
    AddGateCommand(std::shared_ptr<CircuitSimulator> simulator,
                   GateKind kind,
                   Vector2 position,
                   Vector2 size = {0.0f, 0.0f});

    void execute() override;
    void undo() override;

    const std::string& gateId() const { return gateId_; }

private:
    std::shared_ptr<CircuitSimulator> simulator_;
    GateKind kind_;
    Vector2 position_;
    Vector2 size_;
    std::string gateId_;
};

#endif // UI_COMMANDS_ADD_GATE_COMMAND_H
