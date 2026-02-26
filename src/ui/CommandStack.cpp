#include "ui/CommandStack.h"

void CommandStack::execute(std::unique_ptr<EditorCommand> cmd) {
    if (!cmd) {
        return;
    }

    cmd->execute();

    if (!undoStack_.empty() && undoStack_.back()->mergeWith(*cmd)) {
        redoStack_.clear();
        return;
    }

    undoStack_.push_back(std::move(cmd));
    redoStack_.clear();
}

bool CommandStack::canUndo() const {
    return !undoStack_.empty();
}

bool CommandStack::canRedo() const {
    return !redoStack_.empty();
}

void CommandStack::undo() {
    if (undoStack_.empty()) {
        return;
    }

    std::unique_ptr<EditorCommand> cmd = std::move(undoStack_.back());
    undoStack_.pop_back();
    cmd->undo();
    redoStack_.push_back(std::move(cmd));
}

void CommandStack::redo() {
    if (redoStack_.empty()) {
        return;
    }

    std::unique_ptr<EditorCommand> cmd = std::move(redoStack_.back());
    redoStack_.pop_back();
    cmd->execute();
    undoStack_.push_back(std::move(cmd));
}

void CommandStack::clear() {
    undoStack_.clear();
    redoStack_.clear();
}
