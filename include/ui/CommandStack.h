#ifndef UI_COMMAND_STACK_H
#define UI_COMMAND_STACK_H

#include "ui/EditorCommand.h"

#include <memory>
#include <vector>

class CommandStack {
public:
    void execute(std::unique_ptr<EditorCommand> cmd);
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    void clear();

private:
    std::vector<std::unique_ptr<EditorCommand>> undoStack_;
    std::vector<std::unique_ptr<EditorCommand>> redoStack_;
};

#endif // UI_COMMAND_STACK_H
