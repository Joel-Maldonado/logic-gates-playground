#ifndef UI_EDITOR_COMMAND_H
#define UI_EDITOR_COMMAND_H

class EditorCommand {
public:
    virtual ~EditorCommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual bool mergeWith(const EditorCommand& other) {
        (void)other;
        return false;
    }
};

#endif // UI_EDITOR_COMMAND_H
