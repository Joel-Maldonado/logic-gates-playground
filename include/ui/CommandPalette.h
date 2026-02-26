#ifndef UI_COMMAND_PALETTE_H
#define UI_COMMAND_PALETTE_H

#include "ui/DesignTokens.h"

#include <optional>
#include <string>
#include <vector>

class CommandPalette {
public:
    struct Entry {
        std::string id;
        std::string label;
    };

    CommandPalette();

    void toggle();
    void open();
    void close();
    bool isOpen() const;

    std::optional<std::string> handleInput();
    void render(int screenWidth, int screenHeight, const DesignTokens& tokens) const;

private:
    bool open_;
    int selectedIndex_;
    std::vector<Entry> entries_;
};

#endif // UI_COMMAND_PALETTE_H
