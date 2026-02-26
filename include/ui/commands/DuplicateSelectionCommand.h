#ifndef UI_COMMANDS_DUPLICATE_SELECTION_COMMAND_H
#define UI_COMMANDS_DUPLICATE_SELECTION_COMMAND_H

#include "ui/EditorCommand.h"
#include "ui/EditorSelection.h"

#include <memory>
#include <string>
#include <vector>

class CircuitSimulator;

class DuplicateSelectionCommand : public EditorCommand {
public:
    DuplicateSelectionCommand(std::shared_ptr<CircuitSimulator> simulator,
                              const EditorSelection& selection,
                              Vector2 offset);

    void execute() override;
    void undo() override;

    const std::vector<std::string>& createdGateIds() const { return createdGateIds_; }

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
    std::vector<std::string> sourceGateIds_;
    std::vector<std::string> createdGateIds_;
    std::vector<GateSnapshot> createdGateSnapshots_;
    std::vector<WireSnapshot> createdWireSnapshots_;
    Vector2 offset_;
    bool initialized_;
};

#endif // UI_COMMANDS_DUPLICATE_SELECTION_COMMAND_H
