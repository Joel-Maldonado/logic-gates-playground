#ifndef UI_COMMANDS_MOVE_GATES_COMMAND_H
#define UI_COMMANDS_MOVE_GATES_COMMAND_H

#include "ui/EditorCommand.h"

#include <raylib.h>
#include <memory>
#include <string>
#include <vector>

class CircuitSimulator;

class MoveGatesCommand : public EditorCommand {
public:
    MoveGatesCommand(std::shared_ptr<CircuitSimulator> simulator,
                     std::vector<std::string> gateIds,
                     std::vector<Vector2> fromPositions,
                     std::vector<Vector2> toPositions);

    void execute() override;
    void undo() override;
    bool mergeWith(const EditorCommand& other) override;

private:
    void apply(const std::vector<Vector2>& positions);

    std::shared_ptr<CircuitSimulator> simulator_;
    std::vector<std::string> gateIds_;
    std::vector<Vector2> fromPositions_;
    std::vector<Vector2> toPositions_;
};

#endif // UI_COMMANDS_MOVE_GATES_COMMAND_H
