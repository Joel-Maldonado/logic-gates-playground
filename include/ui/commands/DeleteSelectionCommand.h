#ifndef UI_COMMANDS_DELETE_SELECTION_COMMAND_H
#define UI_COMMANDS_DELETE_SELECTION_COMMAND_H

#include "core/LogicGate.h"
#include "ui/EditorCommand.h"
#include "ui/EditorSelection.h"

#include <memory>
#include <string>
#include <vector>

class CircuitSimulator;

class DeleteSelectionCommand : public EditorCommand {
public:
    DeleteSelectionCommand(std::shared_ptr<CircuitSimulator> simulator,
                           const EditorSelection& selection);

    void execute() override;
    void undo() override;

private:
    struct GateSnapshot {
        GateKind kind;
        std::string id;
        Vector2 position;
        Vector2 size;
        bool inputState;
    };

    struct WireSnapshot {
        std::string sourceGateId;
        int sourcePinIndex;
        std::string destGateId;
        int destPinIndex;
    };

    std::shared_ptr<CircuitSimulator> simulator_;
    std::vector<GateSnapshot> gateSnapshots_;
    std::vector<WireSnapshot> wireSnapshots_;
    std::vector<std::string> gateIdsToDelete_;
};

#endif // UI_COMMANDS_DELETE_SELECTION_COMMAND_H
