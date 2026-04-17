#ifndef UI_COMMANDS_ADD_WIRE_COMMAND_H
#define UI_COMMANDS_ADD_WIRE_COMMAND_H

#include "ui/EditorCommand.h"

#include <memory>
#include <string>

class CircuitSimulator;

class AddWireCommand : public EditorCommand {
public:
    AddWireCommand(std::shared_ptr<CircuitSimulator> simulator,
                   std::string sourceGateId,
                   int sourcePinIndex,
                   std::string destGateId,
                   int destPinIndex);

    void execute() override;
    void undo() override;

private:
    std::shared_ptr<CircuitSimulator> simulator_;
    std::string sourceGateId_;
    std::string destGateId_;
    int sourcePinIndex_;
    int destPinIndex_;
};

#endif // UI_COMMANDS_ADD_WIRE_COMMAND_H
