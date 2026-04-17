#include "ui/CommandPalette.h"

#include <raylib.h>

CommandPalette::CommandPalette()
    : open_(false),
      selectedIndex_(0),
      entries_({
          {"undo", "Undo"},
          {"redo", "Redo"},
          {"select_all", "Select All"},
          {"duplicate", "Duplicate Selection"},
          {"delete", "Delete Selection"},
          {"frame", "Frame Selection"},
          {"toggle_grid", "Toggle Grid"},
          {"toggle_snap", "Toggle Grid Snap"}
      }) {
}

void CommandPalette::toggle() {
    open_ = !open_;
}

void CommandPalette::open() {
    open_ = true;
}

void CommandPalette::close() {
    open_ = false;
}

bool CommandPalette::isOpen() const {
    return open_;
}

std::optional<std::string> CommandPalette::handleInput() {
    if (!open_) {
        return std::nullopt;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        close();
        return std::nullopt;
    }

    if (IsKeyPressed(KEY_DOWN)) {
        selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(entries_.size());
    }
    if (IsKeyPressed(KEY_UP)) {
        selectedIndex_--;
        if (selectedIndex_ < 0) {
            selectedIndex_ = static_cast<int>(entries_.size()) - 1;
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        const std::string id = entries_[static_cast<size_t>(selectedIndex_)].id;
        close();
        return id;
    }

    for (size_t i = 0; i < entries_.size() && i < 9; ++i) {
        if (IsKeyPressed(KEY_ONE + static_cast<int>(i))) {
            const std::string id = entries_[i].id;
            close();
            return id;
        }
    }

    return std::nullopt;
}

void CommandPalette::render(int screenWidth, int screenHeight, const DesignTokens& tokens) const {
    if (!open_) {
        return;
    }

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.45f));

    const float width = static_cast<float>(screenWidth) * 0.55f;
    const float height = static_cast<float>(entries_.size() * 36 + 56);
    const Rectangle box = {
        (screenWidth - width) * 0.5f,
        90.0f,
        width,
        height
    };

    DrawRectangleRounded(box, 0.08f, 10, tokens.colors.panelBackground);
    DrawRectangleRoundedLines(box, 0.08f, 10, 1.5f, tokens.colors.panelBorder);

    DrawTextEx(tokens.typography.ui,
               "Command Palette (1-9 or Enter)",
               {box.x + 14.0f, box.y + 12.0f},
               tokens.typography.bodySize,
               1.0f,
               tokens.colors.textPrimary);

    float y = box.y + 40.0f;
    for (size_t i = 0; i < entries_.size(); ++i) {
        const bool selected = static_cast<int>(i) == selectedIndex_;
        Rectangle row = {box.x + 10.0f, y, box.width - 20.0f, 30.0f};
        if (selected) {
            DrawRectangleRounded(row, 0.15f, 8, Fade(tokens.colors.accentPrimary, 0.2f));
        }

        const std::string line = TextFormat("%zu. %s", i + 1, entries_[i].label.c_str());
        DrawTextEx(tokens.typography.mono,
                   line.c_str(),
                   {row.x + 8.0f, row.y + 7.0f},
                   tokens.typography.smallSize,
                   1.0f,
                   selected ? tokens.colors.textPrimary : tokens.colors.textMuted);

        y += 34.0f;
    }
}
